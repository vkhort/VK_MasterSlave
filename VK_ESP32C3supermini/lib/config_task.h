#ifndef CONFIG_TASK_H
#define CONFIG_TASK_H

#include <Arduino.h>

// ==================== КОНФИГУРАЦИЯ ПЛАТЫ (0 - МАКЕТКА / 1 - ПЛАТА) ====================
#define MOTOR_STEP_PIN     3   // Переключить для платы
#define MOTOR_DIR_PIN      1

#define SERVO_PIN            4   // Пин управления сервоприводом

// ==================== ПАРАМЕТРЫ ШАГОВОГО ДВИГАТЕЛЯ ПО УМОЛЧАНИЮ ====================
#define DEFAULT_MOTOR_SPEED  500.0f  // Рабочая скорость (шагов/сек)
#define DEFAULT_MOTOR_ACCEL  200.0f  // Ускорение (шагов/сек²)
#define DEFAULT_STEPS_REV    200     // Полный оборот мотора (full-step NEMA 8)
#define DEFAULT_STEP_DELAY   2000    // Задержка между шагами по умолчанию (мкс)

// ==================== ПАРАМЕТРЫ СЕРВОПРИВОДА (LEDC) ====================
#define DEFAULT_SERVO_ANGLE  0       // Стартовый угол сервы
#define SERVO_PWM_CHANNEL    0       // Аппаратный канал LEDC (0..15)
#define SERVO_PWM_FREQ       50      // Частота ШИМ для сервоприводов (50 Гц)
#define SERVO_PWM_RES        14      // Разрешение ШИМ (14 бит)
#define SERVO_MIN_PULSE      700     // Импульс для 0° (мкс)
#define SERVO_MAX_PULSE      2200    // Импульс для 180° (мкс)
#define SERVO_STEP_DELAY     10      // Задержка шага поворота сервы в мс (меньше = быстрее)

// ==================== ПРОТОКОЛ СЕТЕВЫХ КОМАНД ====================
#define CMD_STEPPER_MOVE     "STEP_MOVE"  // Относительные шаги (STEP_MOVE:200)
#define CMD_STEPPER_GOTO     "STEP_GOTO"  // В абсолютную позицию
#define CMD_STEPPER_STOP     "STEP_STOP"  // Экстренная остановка шаговика
#define CMD_STEPPER_SPEED    "STEP_SPEED" // Изменение скорости
#define CMD_SERVO_SET        "SRV_SET"    // Установить угол сервы (SRV_SET:90)
#define CMD_GET_STATUS       "STATUS"     // Запрос состояния исполнителей

#endif // CONFIG_TASK_H
