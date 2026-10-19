#include <lib/classes.h>

WorkFS fileSystem;
WorkFS::ConfigData hubConfig;
WiFiConnect netManager;

void setup() {
#if DEBUG_MODE
    delay(3000); 
    Serial.begin(115200);
    Serial.println("\n\n=== СТАРТ ЦЕНТРАЛЬНОГО ПУЛЬТА (MASTER HUB) ===");
#endif

    if (!fileSystem.begin(FSType::LITTLE_FS, true)) {
#if DEBUG_MODE
        Serial.println("[-] Ошибка LittleFS на Центре!");
#endif
        while(1) delay(1000);
    }

    // ====================================================================
    // 💡 ОДНОКРАТНЫЙ СБРОС СТАРОГО КОНФИГА ИЗ КЛИЕНТСКОГО КОДА
    // Раскомментируй строку ниже, прошей плату ОДИН раз (она сотрет старый IP),
    // а потом закомментируй её обратно, чтобы конфиг не стирался при рестартах.
    // ====================================================================
//    fileSystem.deleteFS(); 

    if (!fileSystem.exists()) {
        fileSystem.createConfigIfNotExists();
        fileSystem.setDefaults(hubConfig);
        
        fileSystem.saveConfig(hubConfig);
#if DEBUG_MODE
        Serial.println("[FS] Создан новый чистый config.txt для Хаба.");
#endif
    }

    fileSystem.loadConfig(hubConfig);
    netManager.setupHub(fileSystem, hubConfig);
}

void loop() {
    netManager.handle();
    // delay(10);
}
