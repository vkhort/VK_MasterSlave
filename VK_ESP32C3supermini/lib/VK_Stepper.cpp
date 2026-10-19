#include "VK_Stepper.h"
#include "config.h" // Для проверки макроса DEBUG_MODE

VK_Stepper::VK_Stepper() 
    : _stepPin(-1), _dirPin(-1), _speed(500.0f), _stepsPerRev(200), 
      _stepWidthUs(2000), _isMoving(false), _direction(true), 
      _lastStepTime(0), _stepIntervalUs(2000) {}

void VK_Stepper::begin(int stepPin, int dirPin, float speed, int stepsPerRev, int stepWidthUs) {
    _stepPin = stepPin;
    _dirPin = dirPin;
    _speed = speed;
    _stepsPerRev = stepsPerRev;
    _stepWidthUs = stepWidthUs;

    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
    digitalWrite(_stepPin, LOW);
    digitalWrite(_dirPin, LOW);

    // Автоматически рассчитываем временной интервал на основе переданной скорости
    setSpeed(_speed);

#if DEBUG_MODE
    Serial.printf("[Stepper] Инициализирован на STEP: GPIO%d, DIR: GPIO%d, Скорость: %.1f шаг/с\n", _stepPin, _dirPin, _speed);
#endif
}

void VK_Stepper::setSpeed(float speed) {
    if (speed <= 0.0f) {
        stop();
        return;
    }
    _speed = speed;
    // Интервал в микросекундах между импульсами STEP = 1 000 000 мкс / скорость
    _stepIntervalUs = (unsigned long)(1000000.0f / _speed);
}

void VK_Stepper::startRotation(bool direction) {
    _direction = direction;
    _isMoving = true;
    digitalWrite(_dirPin, _direction ? HIGH : LOW);
    
    // Даем микросхеме A4988 минимальное время на фиксацию логического уровня направления
    delayMicroseconds(5); 

#if DEBUG_MODE
    Serial.printf("[Stepper] Запуск вращения: %s\n", _direction ? "По часовой (CW)" : "Против часовой (CCW)");
#endif
}

void VK_Stepper::stop() {
    _isMoving = false;
#if DEBUG_MODE
    Serial.println("[Stepper] Мотор остановлен.");
#endif
}

// ✅ ГЛАВНЫЙ ТАКТОВЫЙ МЕТОД: Крутит мотор в фоновом режиме, не блокируя процессор
// Переписываем метод в файле lib/VK_Stepper.cpp
// Замените этот метод в lib/VK_Stepper.cpp
void VK_Stepper::tick() {
    if (!_isMoving) return;

    // Проверяем, пришло ли время делать шаг по микросекундам
    if (micros() - _lastStepTime >= _stepIntervalUs) {
        _lastStepTime = micros();

        // Формируем моментальный и чистый аппаратный импульс шага для драйвера A4988
        digitalWrite(_stepPin, HIGH);
        delayMicroseconds(3); // Драйверу A4988 достаточно импульса длинее 1 мкс!
        digitalWrite(_stepPin, LOW);
    }
}


bool VK_Stepper::isMoving() const {
    return _isMoving;
}
