#include <lib/classes.h>
#include <lib/VK_Servo.h>
#include <lib/VK_Stepper.h> // Подключаем класс шагового мотора

WorkFS fileSystem;
WorkFS::ConfigData sysConfig;
WiFiConnect netManager;

VK_Servo myServo;
VK_Stepper myStepper; // ✅ Создаем физический объект шагового мотора

void setup() {
#if DEBUG_MODE
    // 2-секундный буфер для инициализации встроенного USB CDC чипа ESP32-C3
    delay(2000); 

    Serial.begin(115200);
    for(int i = 0; i < 3; i++) { 
        delay(1000); 
        Serial.print("."); 
    }
    Serial.println("\n\n=== СИНХРОННЫЙ ОПЕРАЦИОННЫЙ ТЕСТ МЕХАНИКИ ===");
#endif

    // Инициализируем файловую систему
    if (!fileSystem.begin(FSType::LITTLE_FS, true)) {
#if DEBUG_MODE
        Serial.println("[-] Ошибка LittleFS!");
#endif
        while(1) delay(1000);
    }

    // Проверяем наличие файла конфигурации config.txt на диске
    if (!fileSystem.exists()) {
        fileSystem.createConfigIfNotExists();
        fileSystem.setDefaults(sysConfig);
        fileSystem.saveConfig(sysConfig);
#if DEBUG_MODE
        Serial.println("[FS] Создан новый config.txt с дефолтными значениями.");
#endif
    }

    // Считываем сохраненные параметры механики и сети из файла
    fileSystem.loadConfig(sysConfig);

#if DEBUG_MODE
    Serial.println("[FS] Настройки успешно загружены из файла.");
#endif

    // 1. Сначала запускаем Wi-Fi связь (без просадок питания от периферии)
    netManager.setupWiFi(fileSystem, sysConfig, WIFI_CONNECT_TIMEOUT);

    // 2. Инициализируем сервопривод и задаем ему считанную скорость
    myServo.begin(sysConfig.servoPin, 0, 50, 14, sysConfig.servoMinPulse, sysConfig.servoMaxPulse);
    myServo.setSpeedDelay(sysConfig.servoStepDelay); 

    // 3. ✅ Инициализируем шаговый мотор, передавая ему параметры из живого конфига!
    // Step Pin, Dir Pin, Скорость (шаг/с), Шагов на оборот, Длина импульса (delay мкс)
    myStepper.begin(sysConfig.stepPin, sysConfig.dirPin, sysConfig.motorSpeed, sysConfig.stepsPerRev, sysConfig.stepDelay);

    // 4. ✅ Связываем оба устройства с сетевым менеджером веб-сервера
    netManager.attachServo(myServo);
    netManager.attachStepper(myStepper);
}

void loop() {
    // Обслуживаем асинхронный веб-сервер и безопасные OTA-обновления по воздуху
    netManager.handle();
    
    // ✅ Параллельное фоновое обслуживание исполнительных устройств
    myServo.tick();   // Двигает серву на 1 градус, если пришло время millis()
    myStepper.tick(); // Генерирует импульс шага для A4988, если пришло время micros()
    
    // Микропауза, чтобы дать планировщику FreeRTOS переключать задачи ядра
    // Вместо него для стабильности FreeRTOS используем стандартный yield()
    yield(); 
}
