#ifndef CONFIG_TASK_H
#define CONFIG_TASK_H

#include <Arduino.h>

// ==================== НАСТРОЙКИ СВЯЗИ ПО УМОЛЧАНИЮ ====================
#define DEFAULT_MASTER    "192.168.4.1"       // IP самого Центрального пульта
#define DEFAULT_SLAVE1    "192.168.4.2"       // Наша Supermini №1 с жестким IP!
#define DEFAULT_SLAVE2    "192.168.4.3"       // Задел под плату №2
#define DEFAULT_SLAVE3    "192.168.4.4"       
#define DEFAULT_SLAVE4    "192.168.4.5"       


// ==================== ПРОТОКОЛ СЕТЕВЫХ КОМАНД ====================
#define CMD_STEPPER_MOVE     "STEP_MOVE"  // Относительные шаги (STEP_MOVE:200)
#define CMD_STEPPER_GOTO     "STEP_GOTO"  // В абсолютную позицию
#define CMD_STEPPER_STOP     "STEP_STOP"  // Экстренная остановка шаговика
#define CMD_STEPPER_SPEED    "STEP_SPEED" // Изменение скорости
#define CMD_SERVO_SET        "SRV_SET"    // Установить угол сервы (SRV_SET:90)
#define CMD_GET_STATUS       "STATUS"     // Запрос состояния исполнителей

// Добавьте в конец файла lib/config_task.h на стороне ESP-32S
// ==================== ИНДИКАЦИЯ И МОНИТОРИНГ ====================
#define TOTAL_SLAVES_COUNT   1       // Количество ожидаемых исполнителей (пока 1, потом исправим на 4)
#define LED_PIN              2       // На каком пине висит встроенный светодиод платы ESP-32S

#endif // CONFIG_TASK_H
