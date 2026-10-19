#pragma once
#include <Arduino.h>

class VK_Stepper {
private:
    int _stepPin;
    int _dirPin;
    float _speed;          // Рабочая скорость (шагов в секунду)
    int _stepsPerRev;      // Шагов на один оборот (для индикации/расчетов)
    int _stepWidthUs;      // Длина самого импульса STEP (обычно 2-10 мкс)
    
    bool _isMoving;
    bool _direction;
    unsigned long _lastStepTime;
    unsigned long _stepIntervalUs; // Рассчитанное время между импульсами в мкс

public:
    VK_Stepper();

    // Инициализация пинов и параметров вращения из считанного конфига
    void begin(int stepPin, int dirPin, float speed, int stepsPerRev, int stepWidthUs);

    // Управление вращением (true - по часовой (cw), false - против часовой (ccw))
    void startRotation(bool direction);
    
    // Изменение скорости на лету
    void setSpeed(float speed);
    
    // Остановка мотора
    void stop();
    
    // Неблокирующий метод для вызова в основном loop()
    void tick();

    // Получение текущего состояния движения
    bool isMoving() const;
};
