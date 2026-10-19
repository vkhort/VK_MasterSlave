#pragma once
#include <Arduino.h>

class VK_Servo {
private:
    int _pin;
    int _channel;        // Храним выделенный аппаратный канал ШИМ
    int _minPulse;
    int _maxPulse;
    
    int _currentAngle;   // Где серва находится сейчас
    int _targetAngle;    // Куда серва должна прийти
    unsigned long _lastStepTime;
    unsigned long _stepDelayMs; // Задержка между шагами (скорость)
    bool _isInitialized;

    void refreshPhysicalPWM(); // Внутренний метод отправки ШИМ в пин

public:
    VK_Servo();

    // begin теперь корректно принимает и использует канал
    bool begin(int pin, int channel = 0, int freq = 50, int resolution = 14, int minPulse = 500, int maxPulse = 2400);

    // Установка целевого угла
    void write(int angle);
    
    // Настраивает скорость (ms на 1 градус)
    void setSpeedDelay(unsigned long delayMs);

    // Неблокирующий метод для вызова в основном loop()
    void tick();

    int getAngle() const;
    void detach();
};
