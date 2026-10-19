#include "classes.h"
#include <ESPmDNS.h>

WorkFS::WorkFS(const String& configFile)
  : configPath(configFile), tempPath(configFile + ".tmp"), currentFS(FSType::LITTLE_FS), _initialized(false) {}

bool WorkFS::begin(FSType type, bool formatOnFail) {
    currentFS = type;
    if (currentFS == FSType::LITTLE_FS) {
        _initialized = LittleFS.begin(formatOnFail, "/littlefs", 5, "littlefs");
    } else {
        _initialized = SPIFFS.begin(formatOnFail);
    }
    return _initialized;
}

bool WorkFS::formatFS() {
    bool res = false;
    if (currentFS == FSType::LITTLE_FS) res = LittleFS.format();
    else res = SPIFFS.format();
#if DEBUG_MODE
    if (res) Serial.println("[FS] Файловая система успешно отформатирована.");
    else Serial.println("[FS] Ошибка форматирования!");
#endif
    return res;
}

bool WorkFS::exists() {
    if (!_initialized) return false;
    return (currentFS == FSType::LITTLE_FS) ? LittleFS.exists(configPath) : SPIFFS.exists(configPath);
}

bool WorkFS::createConfigIfNotExists() {
    if (exists()) return true;
#if DEBUG_MODE
    Serial.println("[FS] Файл конфигурации не найден. Создаём пустой базовый файл...");
#endif
    return writeRawText(configPath, "");
}

// ✅ МЕТОД СОХРАНЕНИЯ: Теперь записывает все 12 параметров механики и сети в config.txt
bool WorkFS::saveConfig(const ConfigData &data) {
    String content = "";
    content += "ssid=" + data.ssid + "\n";
    content += "password=" + data.password + "\n";
    content += "espname=" + data.espName + "\n";
    
    content += "step_pin=" + String(data.stepPin) + "\n";
    content += "dir_pin=" + String(data.dirPin) + "\n";
    content += "step_speed=" + String(data.motorSpeed, 1) + "\n";
    content += "step_accel=" + String(data.motorAccel, 1) + "\n";
    content += "step_rev=" + String(data.stepsPerRev) + "\n";
    content += "step_delay=" + String(data.stepDelay) + "\n";
    
    content += "servo_pin=" + String(data.servoPin) + "\n";
    content += "servo_min=" + String(data.servoMinPulse) + "\n";
    content += "servo_max=" + String(data.servoMaxPulse) + "\n";
    content += "servo_delay=" + String(data.servoStepDelay) + "\n";

    if (!writeRawText(tempPath, content)) return false;

    if (currentFS == FSType::LITTLE_FS) {
        if (LittleFS.exists(configPath)) LittleFS.remove(configPath);
        return LittleFS.rename(tempPath, configPath);
    } else {
        if (SPIFFS.exists(configPath)) SPIFFS.remove(configPath);
        return writeRawText(configPath, content);
    }
}

// ✅ ПОСТРОЧНЫЙ ПАРСЕР: Считывает все новые параметры механики из config.txt
bool WorkFS::loadConfig(ConfigData &data) {
    if (!exists()) {
        setDefaults(data);
        saveConfig(data);
        return false;
    }

    // 1. Сначала считываем всё, что реально есть в файле на данный момент
    String content = readRawText(configPath);
    int index = 0;

    // Флаги, чтобы узнать, какие параметры физически присутствуют в файле
    bool hasSsid = false, hasPass = false, hasName = false;
    bool hasStepPin = false, hasDirPin = false, hasSpeed = false, hasAccel = false, hasRev = false, hasDelay = false;
    bool hasSrvPin = false, hasSrvMin = false, hasSrvMax = false;

    while (index < content.length()) {
        int nextLine = content.indexOf('\n', index);
        if (nextLine == -1) nextLine = content.length();
        String line = content.substring(index, nextLine);
        line.trim();
        index = nextLine + 1;

        if (line.length() == 0 || line.startsWith("#")) continue;
        int eqIndex = line.indexOf('=');
        if (eqIndex == -1) continue;

        String key = line.substring(0, eqIndex);
        String value = line.substring(eqIndex + 1);
        key.trim(); value.trim();

        if (key == "ssid") { data.ssid = value; hasSsid = true; }
        else if (key == "password") { data.password = value; hasPass = true; }
        else if (key == "espname") { data.espName = value; hasName = true; }
        else if (key == "step_pin") { data.stepPin = value.toInt(); hasStepPin = true; }
        else if (key == "dir_pin") { data.dirPin = value.toInt(); hasDirPin = true; }
        else if (key == "step_speed") { data.motorSpeed = value.toFloat(); hasSpeed = true; }
        else if (key == "step_accel") { data.motorAccel = value.toFloat(); hasAccel = true; }
        else if (key == "step_rev") { data.stepsPerRev = value.toInt(); hasRev = true; }
        else if (key == "step_delay") { data.stepDelay = value.toInt(); hasDelay = true; }
        else if (key == "servo_pin") { data.servoPin = value.toInt(); hasSrvPin = true; }
        else if (key == "servo_min") { data.servoMinPulse = value.toInt(); hasSrvMin = true; }
        else if (key == "servo_max") { data.servoMaxPulse = value.toInt(); hasSrvMax = true; }
        else if (key == "servo_delay") { data.servoStepDelay = value.toInt(); }
    }

    // 2. ✅ Безопасная дозапись: если какого-то поля не было, мы НЕ стираем файл.
    // Мы точечно дописываем только его, используя метод updateValue!
    if (!hasSsid)    { updateValue("ssid", CONNECT_SSID); data.ssid = CONNECT_SSID; }
    if (!hasPass)    { updateValue("password", CONNECT_PASSWORD); data.password = CONNECT_PASSWORD; }
    if (!hasName)    { updateValue("espname", AP_SSID); data.espName = AP_SSID; }
    
    if (!hasStepPin) { updateValue("step_pin", String(MOTOR_STEP_PIN)); data.stepPin = MOTOR_STEP_PIN; }
    if (!hasDirPin)  { updateValue("dir_pin", String(MOTOR_DIR_PIN)); data.dirPin = MOTOR_DIR_PIN; }
    if (!hasSpeed)   { updateValue("step_speed", String(DEFAULT_MOTOR_SPEED, 1)); data.motorSpeed = DEFAULT_MOTOR_SPEED; }
    if (!hasAccel)   { updateValue("step_accel", String(DEFAULT_MOTOR_ACCEL, 1)); data.motorAccel = DEFAULT_MOTOR_ACCEL; }
    if (!hasRev)     { updateValue("step_rev", String(DEFAULT_STEPS_REV)); data.stepsPerRev = DEFAULT_STEPS_REV; }
    if (!hasDelay)   { updateValue("step_delay", String(DEFAULT_STEP_DELAY)); data.stepDelay = DEFAULT_STEP_DELAY; }
    
    if (!hasSrvPin)  { updateValue("servo_pin", String(SERVO_PIN)); data.servoPin = SERVO_PIN; }
    if (!hasSrvMin)  { updateValue("servo_min", String(SERVO_MIN_PULSE)); data.servoMinPulse = SERVO_MIN_PULSE; }
    if (!hasSrvMax)  { updateValue("servo_max", String(SERVO_MAX_PULSE)); data.servoMaxPulse = SERVO_MAX_PULSE; }

    return true;
}

bool WorkFS::updateValue(const String& key, const String& value) {
    if (!exists()) createConfigIfNotExists();

    String content = readRawText(configPath);
    String newContent = "";
    int index = 0;
    bool keyFound = false;

    while (index < content.length()) {
        int nextLine = content.indexOf('\n', index);
        if (nextLine == -1) nextLine = content.length();
        
        String line = content.substring(index, nextLine);
        index = nextLine + 1;

        String trimmedLine = line;
        trimmedLine.trim();

        if (trimmedLine.length() > 0 && !trimmedLine.startsWith("#")) {
            int eqIndex = trimmedLine.indexOf('=');
            if (eqIndex != -1) {
                String currentKey = trimmedLine.substring(0, eqIndex);
                currentKey.trim();

                if (currentKey == key) {
                    newContent += key + "=" + value + "\n";
                    keyFound = true;
                    continue;
                }
            }
        }
        if (line.length() > 0) {
            newContent += line;
            if (!line.endsWith("\n")) newContent += "\n";
        }
    }

    if (!keyFound) newContent += key + "=" + value + "\n";
    return writeRawText(configPath, newContent);
}

// ✅ МЕТОД ДЕФОЛТОВ: Теперь сбрасывает абсолютно все новые параметры под константы из config_task.h
void WorkFS::setDefaults(ConfigData &data) {
    data.ssid = CONNECT_SSID;
    data.password = CONNECT_PASSWORD;
    data.espName = AP_SSID;
    
    data.stepPin = MOTOR_STEP_PIN;
    data.dirPin = MOTOR_DIR_PIN;
    data.motorSpeed = DEFAULT_MOTOR_SPEED;
    data.motorAccel = DEFAULT_MOTOR_ACCEL;
    data.stepsPerRev = DEFAULT_STEPS_REV;
    data.stepDelay = DEFAULT_STEP_DELAY;

    data.servoPin = SERVO_PIN;
    data.servoMinPulse = SERVO_MIN_PULSE;
    data.servoMaxPulse = SERVO_MAX_PULSE;
    data.servoStepDelay = SERVO_STEP_DELAY;
}

bool WorkFS::deleteFS() {
    if (!_initialized) return false;
#if DEBUG_MODE
    Serial.println("[FS] Запрос на удаление файла конфигурации config.txt...");
#endif
    return (currentFS == FSType::LITTLE_FS) ? LittleFS.remove(configPath) : SPIFFS.remove(configPath);
}

size_t WorkFS::getFileSize() {
    if (!exists()) return 0;
    File file = (currentFS == FSType::LITTLE_FS) ? LittleFS.open(configPath, "r") : SPIFFS.open(configPath, "r");
    if (!file) return 0;
    size_t size = file.size();
    file.close();
    return size;
}

void WorkFS::printDirectory(const String& path, uint8_t numSpaces) {
    if (!_initialized) return;
    File root = (currentFS == FSType::LITTLE_FS) ? LittleFS.open(path) : SPIFFS.open(path);
    if (!root || !root.isDirectory()) return;

    File file = root.openNextFile();
    while (file) {
        for (uint8_t i = 0; i < numSpaces; i++) {
#if DEBUG_MODE
            Serial.print(" ");
#endif
        }
#if DEBUG_MODE
        Serial.printf("-> %s (%d bytes)\n", file.name(), file.size());
#endif
        file = root.openNextFile();
    }
}

bool WorkFS::writeRawText(const String& path, const String& content) {
    File file = (currentFS == FSType::LITTLE_FS) ? LittleFS.open(path, "w") : SPIFFS.open(path, "w");
    if (!file) return false;
    file.print(content);
    file.close();
    return true;
}

String WorkFS::readRawText(const String& path) {
    File file = (currentFS == FSType::LITTLE_FS) ? LittleFS.open(path, "r") : SPIFFS.open(path, "r");
    if (!file) return "";
    String content = file.readString();
    file.close();
    return content;
}

// ====================================================================
// ==================== РЕАЛИЗАЦИЯ WIFI + ВЕБ-СЕРВЕР ===================
// ====================================================================

WiFiConnect::WiFiConnect() 
  : currentMode(Mode::DISCONNECTED), 
    currentStatus(Status::IDLE), 
    connectTimeout(WIFI_CONNECT_TIMEOUT), 
    fsRef(nullptr),
    servoRef(nullptr),
    stepperRef(nullptr), // ✅ Инициализируем указатель шаговика нулем
    server(80) {}

void WiFiConnect::attachServo(VK_Servo& servoInstance) {
    servoRef = &servoInstance;
}

void WiFiConnect::attachStepper(VK_Stepper& stepperInstance) {
    stepperRef = &stepperInstance;
}

bool WiFiConnect::setupWiFi(WorkFS& fs, WorkFS::ConfigData& config, unsigned long timeoutMs) {
    fsRef = &fs;
    targetSSID = config.ssid;
    targetPassword = config.password;
    deviceName = config.espName; // Текст IP-адреса из config.txt ("192.168.4.2")
    connectTimeout = timeoutMs;
    
#if DEBUG_MODE
    Serial.printf("[WiFi] Попытка подключения к сети: %s\n", targetSSID.c_str());
#endif
    
    WiFi.mode(WIFI_STA); 
    delay(50);
    WiFi.disconnect(true); 
    delay(50);

    // ====================================================================
    // ⚙️ СТАТИЧЕСКАЯ УСТАНОВКА СЕТЕВЫХ КЛАССОВ ИЗ CONFIG.H
    // ====================================================================
    IPAddress local_IP;
    if (local_IP.fromString(deviceName)) {
        // Задаем IP, Шлюз, Маску и DNS (равный шлюзу) без лишних переменных
        if (WiFi.config(local_IP, MASTER_GATEWAY, MASTER_SUBNET, MASTER_GATEWAY)) {
#if DEBUG_MODE
            Serial.printf("[WiFi] Установлен статический IP: %s\n", local_IP.toString().c_str());
#endif
        } else {
#if DEBUG_MODE
            Serial.println("[-] Ошибка применения настроек статического IP!");
#endif
        }
    } else {
#if DEBUG_MODE
            Serial.printf("[-] Ошибка парсинга IP из конфига: '%s'\n", deviceName.c_str());
#endif
    }
    // ====================================================================
    
    WiFi.begin(targetSSID.c_str(), targetPassword.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < connectTimeout) {
        delay(500);
#if DEBUG_MODE
        Serial.print(".");
#endif
    }
#if DEBUG_MODE
    Serial.println();
#endif

    if (WiFi.status() == WL_CONNECTED) {
        currentMode = Mode::STA_MODE;
        currentStatus = Status::CONNECTED;
#if DEBUG_MODE
        Serial.printf("[WiFi] Подключено! Рабочий IP: %s\n", WiFi.localIP().toString().c_str());
#endif
        initOTA();
        setupWebServer();
        return true;
    }
    
    startAPMode();
    return false;
}

void WiFiConnect::startAPMode() {
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP);
    delay(100);
    
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    currentMode = Mode::AP_MODE;
    currentStatus = Status::AP_ACTIVE;

#if DEBUG_MODE
    Serial.println("==========================================");
    Serial.printf("[WiFi AP] Резервная точка настройки активна!\n");
    Serial.printf("  SSID (Имя сети): %s\n", AP_SSID);
    Serial.printf("  IP-адрес платы : %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("==========================================");
#endif
    
    initOTA();
    setupWebServer();
}

void WiFiConnect::setupWebServer() {
    server.on("/", std::bind(&WiFiConnect::handleRootPage, this));
    server.on("/save", HTTP_POST, std::bind(&WiFiConnect::handleSaveConfig, this));
    
    // ✅ ИСПРАВЛЕНО: Жестко привязываем роуты к HTTP_GET, чтобы сервер хватал fetch-запросы пультов
    server.on("/api/stepper", HTTP_GET, std::bind(&WiFiConnect::handleApiStepper, this));
    server.on("/api/servo", HTTP_GET, std::bind(&WiFiConnect::handleApiServo, this));
    
    server.begin();
#if DEBUG_MODE
    Serial.println("[WEB] HTTP Веб-сервер успешно запущен.");
#endif
}

void WiFiConnect::handleRootPage() {
    WorkFS::ConfigData currentData;
    if (fsRef) {
        fsRef->loadConfig(currentData);
    }

    // Собираем всю страницу в RAM, чтобы сервер сам посчитал правильный Content-Length
    String fullHtml = String(FPSTR(HTML_STYLE));
    
    String bodyHtml = String(FPSTR(HTML_BODY));
    bodyHtml.replace("%ESPNAME_VAL%", currentData.espName);
    bodyHtml.replace("%SSID_VAL%", currentData.ssid);
    bodyHtml.replace("%PASS_VAL%", currentData.password);
    bodyHtml.replace("%STEP_PIN_VAL%", String(currentData.stepPin));
    bodyHtml.replace("%DIR_PIN_VAL%", String(currentData.dirPin));
    bodyHtml.replace("%STEP_SPEED_VAL%", String(currentData.motorSpeed, 1));
    bodyHtml.replace("%STEP_ACCEL_VAL%", String(currentData.motorAccel, 1));
    bodyHtml.replace("%STEP_REV_VAL%", String(currentData.stepsPerRev));
    bodyHtml.replace("%STEP_DELAY_VAL%", String(currentData.stepDelay));
    bodyHtml.replace("%SERVO_PIN_VAL%", String(currentData.servoPin));
    bodyHtml.replace("%SERVO_MIN_VAL%", String(currentData.servoMinPulse));
    bodyHtml.replace("%SERVO_MAX_VAL%", String(currentData.servoMaxPulse));
    bodyHtml.replace("%SERVO_DELAY_VAL%", String(currentData.servoStepDelay));
    
    fullHtml += bodyHtml;
    fullHtml += String(FPSTR(HTML_SCRIPT));

    // Отправляем всё одним махом. Сервер автоматически закроет сессию корректно
    server.send(200, "text/html", fullHtml);
}


void WiFiConnect::handleSaveConfig() {
    if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("espname")) {
        
        if (fsRef) {
            fsRef->updateValue("ssid", server.arg("ssid"));
            fsRef->updateValue("password", server.arg("password"));
            fsRef->updateValue("espname", server.arg("espname"));
            
            fsRef->updateValue("step_pin", server.arg("step_pin"));
            fsRef->updateValue("dir_pin", server.arg("dir_pin"));
            fsRef->updateValue("step_speed", server.arg("step_speed"));
            fsRef->updateValue("step_accel", server.arg("step_accel"));
            fsRef->updateValue("step_rev", server.arg("step_rev"));
            fsRef->updateValue("step_delay", server.arg("step_delay"));
            
            fsRef->updateValue("servo_pin", server.arg("servo_pin"));
            fsRef->updateValue("servo_min", server.arg("servo_min"));
            fsRef->updateValue("servo_max", server.arg("servo_max"));
            fsRef->updateValue("servo_delay", server.arg("servo_delay"));
        }

        server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'></head><body><h3>Настройки сохранены! Плата перезагружается...</h3></body></html>");
        
#if DEBUG_MODE
        Serial.println("[WEB] Настройки обновлены через браузер. Корректное отключение радиомодуля...");
#endif
        // ✅ ИСПРАВЛЕНО: Даем роутеру время закрыть сессию, чтобы при старте не было ошибки подключения
        delay(1000);
        WiFi.disconnect(true);
        delay(1000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

// ✅ Обработчик команд пульта шаговика (REST API)
void WiFiConnect::handleApiStepper() {
    if (server.hasArg("cmd")) {
        String command = server.arg("cmd");
        
#if DEBUG_MODE
        Serial.printf("[API] Команда Stepper: %s\n", command.c_str());
#endif

        // ✅ ЕСЛИ ШАГОВИК ПРИВЯЗАН К СЕТИ - ОТПРАВЛЯЕМ КОМАНДЫ В ЖЕЛЕЗО!
        if (stepperRef != nullptr) {
            if (command == "cw") {
                stepperRef->startRotation(true);  // Вращение по часовой (↻)
            } else if (command == "ccw") {
                stepperRef->startRotation(false); // Вращение против часовой (↺)
            } else if (command == "stop") {
                stepperRef->stop();               // Экстренная остановка (STOP)
            }
        }

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing cmd arg");
    }
}

// ✅ Обработчик команд пульта сервопривода (REST API)
void WiFiConnect::handleApiServo() {
    if (server.hasArg("angle")) {
        int angle = server.arg("angle").toInt();
        
#if DEBUG_MODE
        Serial.printf("[API] Команда Servo: установить угол %d градусов\n", angle);
#endif

        // ✅ ЕСЛИ СЕРВА ПОДКЛЮЧЕНА - ОТПРАВЛЯЕМ КОМАНДУ В ЖЕЛЕЗО!
        if (servoRef != nullptr) {
            servoRef->write(angle);
        }

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missing angle arg");
    }
}

void WiFiConnect::initOTA() {
    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname(deviceName.c_str());
    
    // ✅ ДОБАВЛЯЕМ: Регистрируем имя vk_stepper1.local в сети Хаба!
//    String mdnsName = deviceName;
//    mdnsName.toLowerCase(); // Переводим "VK_STEPPER1" в нижний регистр "vk_stepper1"
    
//    if (MDNS.begin(mdnsName.c_str())) {
//#if DEBUG_MODE
//        Serial.printf("[MDNS] Registered name: http://%s.local\n", mdnsName.c_str());
//#endif
//    } else {
//#if DEBUG_MODE
//        Serial.println("[MDNS] Error starting mDNS responder!");
//#endif
//    }

    ArduinoOTA.onStart([]() { 
#if DEBUG_MODE
        Serial.println("\n[OTA] Начало обновления..."); 
#endif
    });
    ArduinoOTA.onEnd([]() { 
#if DEBUG_MODE
        Serial.println("\n[OTA] Успешно! Перезагрузка..."); 
#endif
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#if DEBUG_MODE
        Serial.printf("[OTA] Загрузка: %u%%\r", (progress / (total / 100)));
#endif
    });
    ArduinoOTA.onError([](ota_error_t error) { 
#if DEBUG_MODE
        Serial.printf("[OTA] Ошибка [%u]\n", error); 
#endif
    });
    
    ArduinoOTA.begin();
}

void WiFiConnect::handle() {
    ArduinoOTA.handle();
    server.handleClient();
}

String WiFiConnect::getIPAddress() { return WiFi.localIP().toString(); }
String WiFiConnect::getAPIPAddress() { return WiFi.softAPIP().toString(); }
bool WiFiConnect::isConnected() { return (WiFi.status() == WL_CONNECTED) && (currentMode == Mode::STA_MODE); }
WiFiConnect::Mode WiFiConnect::getMode() { return currentMode; }
WiFiConnect::Status WiFiConnect::getStatus() { return currentStatus; }
