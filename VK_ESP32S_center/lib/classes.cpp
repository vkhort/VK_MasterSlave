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
    if (res) Serial.println("[FS] Файловая система Центра успешно отформатирована.");
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
    Serial.println("[FS] Конфиг не найден на Центре. Создаём пустой базовый файл...");
#endif
    return writeRawText(configPath, "");
}

// ✅ МЕТОД СОХРАНЕНИЯ ЦЕНТРА: Сохраняет сетевые параметры Хаба и адреса плат
bool WorkFS::saveConfig(const ConfigData &data) {
    String content = "";
    content += "ap_ssid=" + data.apSsid + "\n";
    content += "ap_pass=" + data.apPass + "\n";

    if (!writeRawText(tempPath, content)) return false;

    if (currentFS == FSType::LITTLE_FS) {
        if (LittleFS.exists(configPath)) LittleFS.remove(configPath);
        return LittleFS.rename(tempPath, configPath);
    } else {
        if (SPIFFS.exists(configPath)) SPIFFS.remove(configPath);
        return writeRawText(configPath, content);
    }
}

// ✅ ПОСТРОЧНЫЙ ПАРСЕР ЦЕНТРА: Считывает mDNS паспорта исполнителей
bool WorkFS::loadConfig(ConfigData &data) {
    if (!exists()) {
        setDefaults(data);
        saveConfig(data);
        return false;
    }

    String content = readRawText(configPath);
    int index = 0;

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

        if (key == "ap_ssid") { data.apSsid = value; }
        else if (key == "ap_pass") { data.apPass = value; }
    }
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

void WorkFS::setDefaults(ConfigData &data) {
    data.apSsid = DEFAULT_AP_SSID;
    data.apPass = DEFAULT_AP_PASS;
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
// ==================== РЕАЛИЗАЦИЯ WIFI + ВЕБ-СЕРВЕР ЦЕНТРА ============
// ====================================================================

WiFiConnect::WiFiConnect() 
  : currentMode(Mode::DISCONNECTED), 
    currentStatus(Status::IDLE), 
    fsRef(nullptr),
    server(80) {}

// ✅ МЕТОД ЗАПУСКА ЦЕНТРА: Никаких внешних сетей, сразу поднимает AP режим
bool WiFiConnect::setupHub(WorkFS& fs, WorkFS::ConfigData& config) {
    fsRef = &fs;
    hubSSID = config.apSsid;
    hubPassword = config.apPass;
    
    // ✅ ИСПРАВЛЕНО: Адреса slaves больше не читаются с флешки ФС Хаба.
    // Они жестко и надежно инициализируются макросами из системного файла config_task.h!
    targetSlave1 = DEFAULT_SLAVE1;
    targetSlave2 = DEFAULT_SLAVE2;
    targetSlave3 = DEFAULT_SLAVE3;
    targetSlave4 = DEFAULT_SLAVE4;

    startHubAP();
    return true;
}

// Замените этот метод в lib/classes.cpp на стороне Хаба (ESP-32S)
void WiFiConnect::startHubAP() {
    WiFi.mode(WIFI_AP);
    delay(100);
    WiFi.disconnect(true);
    delay(100);

    // Настраиваем пин светодиода на выход и сразу гасим его
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); 

    // ====================================================================
    // 📡 ДИНАМИЧЕСКИЙ ЛОВЕЦ ПОДКЛЮЧЕНИЙ С ВЫВОДОМ IP-АДРЕСА
    // ====================================================================
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        
        // Событие: Устройству в сети AP успешно назначен или подтвержден IP адрес!
        if (event == ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED) {
            // Преобразуем полученный IP-адрес из структуры события в читаемую строку
            String clientIP = IPAddress(info.wifi_ap_staipassigned.ip.addr).toString();
            
            #if DEBUG_MODE
            Serial.printf("[HUB] Подключилось устройство с IP: %s\n", clientIP.c_str());
            Serial.printf("[HUB] Всего активных клиентов на связи: %d\n", WiFi.softAPgetStationNum());
            #endif

            // Логика проверки лимита для зажигания светодиода
            if (WiFi.softAPgetStationNum() >= TOTAL_SLAVES_COUNT) {
                digitalWrite(LED_PIN, HIGH); // 🔥 Включаем светодиод успеха
                #if DEBUG_MODE
                Serial.println("[HUB] Отлично! Все исполнители на связи. Светодиод ЗАЖЖЕН.");
                #endif
            }
        }
        
        // Событие: Клиент отключился от нашей точки доступа
        else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
            #if DEBUG_MODE
            Serial.printf("[HUB] Устройство отключилось. Осталось клиентов: %d\n", WiFi.softAPgetStationNum());
            #endif
            
            if (WiFi.softAPgetStationNum() < TOTAL_SLAVES_COUNT) {
                digitalWrite(LED_PIN, LOW); // Гасим светодиод, если кто-то отключился
            }
        }
    });
    // ====================================================================
    
    // Поднимаем мастер-сеть Хаба
    WiFi.softAP(hubSSID.c_str(), hubPassword.c_str());

    currentMode = Mode::AP_MODE;
    currentStatus = Status::AP_ACTIVE;

#if DEBUG_MODE
    Serial.println("==========================================");
    Serial.printf("[HUB AP] Центральная сеть активна!\n");
    Serial.printf("  SSID (Имя сети Hub): %s\n", hubSSID.c_str());
    Serial.printf("  IP-адрес Центра    : %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("==========================================");
#endif
    
    setupWebServer();
}

void WiFiConnect::setupWebServer() {
    server.on("/", HTTP_GET, std::bind(&WiFiConnect::handleRootPage, this));
    server.on("/save", HTTP_POST, std::bind(&WiFiConnect::handleSaveConfig, this));
    
    // ✅ ГЛАВНЫЙ МАРШРУТ ФОРВАРДИНГА: перехватывает клики пульта Центра
    server.on("/api/forward", HTTP_GET, std::bind(&WiFiConnect::handleApiForward, this));
    
    server.begin();
#if DEBUG_MODE
    Serial.println("[WEB] HTTP Сервер Центра успешно запущен.");
#endif
}

void WiFiConnect::handleRootPage() {
    WorkFS::ConfigData currentData;
    if (fsRef) {
        fsRef->loadConfig(currentData);
    }

    String page = String(FPSTR(HTML_STYLE));
    String bodyHtml = String(FPSTR(HTML_BODY));
    String script = String(FPSTR(HTML_SCRIPT));

    // Подставляем только 2 живых параметра Хаба
    bodyHtml.replace("%AP_SSID_VAL%", currentData.apSsid);
    bodyHtml.replace("%AP_PASS_VAL%", currentData.apPass);

    page += bodyHtml;
    page += script;

    server.send(200, "text/html", page);
}

void WiFiConnect::handleSaveConfig() {
    if (server.hasArg("ap_ssid") && server.hasArg("ap_pass")) {
        
        if (fsRef) {
            // Сохраняем только SSID и PASS
            fsRef->updateValue("ap_ssid", server.arg("ap_ssid"));
            fsRef->updateValue("ap_pass", server.arg("ap_pass"));
        }

        server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'></head><body><h3>Hub settings saved! Rebooting...</h3></body></html>");
        
#if DEBUG_MODE
        Serial.println("[WEB] Конфигурация Хаба изменена. Перезагрузка...");
#endif
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

// ✅ МЕТОД ТРАНСЛЯЦИИ КОМАНД ПО ВОЗДУХУ (HTTP FORWARDING)
void WiFiConnect::handleApiForward() {
    if (server.hasArg("dev") && server.hasArg("action")) {
        String device = server.arg("dev");
        String action = server.arg("action");
        String targetUrl = "";

        // Строим правильный путь в зависимости от типа устройства
        if (device == "stepper") {
            // Например: http://192.168.1 (или через mDNS паспорт targetSlave1)
            targetUrl = "http://" + targetSlave1 + "/api/stepper?cmd=" + action;
        } else if (device == "servo") {
            // Например: http://192.168.1
            targetUrl = "http://" + targetSlave1 + "/api/servo?angle=" + action;
        }

#if DEBUG_MODE
        Serial.printf("[FORWARD] Перенаправление запроса: %s\n", targetUrl.c_str());
#endif

        // Вызываем неблокирующую отправку пакета по Wi-Fi
        sendGetRequest(targetUrl);
        
        server.send(200, "text/plain", "FORWARDED");
    } else {
        server.send(400, "text/plain", "Missing args");
    }
}

// ✅ УТИЛИТА ОТПРАВКИ HTTP GET ПАКЕТА НА ИСПОЛНИТЕЛЬНЫЙ МОДУЛЬ
void WiFiConnect::sendGetRequest(const String& url) {
    HTTPClient http;
    
    // Инициализируем соединение по Wi-Fi
    http.begin(url);
    
    // Выполняем быстрый фоновый GET-запрос
    int httpCode = http.GET();
    
#if DEBUG_MODE
    if (httpCode > 0) {
        Serial.printf("[HTTP] Код ответа от исполнителя: %d\n", httpCode);
    } else {
        Serial.printf("[HTTP] Ошибка отправки пакета по Wi-Fi: %s\n", http.errorToString(httpCode).c_str());
    }
#endif

    http.end(); // Обязательно закрываем сессию сокета
}

void WiFiConnect::handle() {
    server.handleClient(); // Обслуживаем веб-клиентов Центра
}

String WiFiConnect::getAPIPAddress() { return WiFi.softAPIP().toString(); }
WiFiConnect::Mode WiFiConnect::getMode() { return currentMode; }
WiFiConnect::Status WiFiConnect::getStatus() { return currentStatus; }
