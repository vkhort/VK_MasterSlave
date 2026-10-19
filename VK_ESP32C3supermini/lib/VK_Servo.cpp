#include "VK_Servo.h"
#include "config.h" 

VK_Servo::VK_Servo() 
    : _pin(-1), _channel(0), _minPulse(500), _maxPulse(2400), 
      _currentAngle(0), _targetAngle(0), _lastStepTime(0), 
      _stepDelayMs(15), 
      _isInitialized(false) {}

bool VK_Servo::begin(int pin, int channel, int freq, int resolution, int minPulse, int maxPulse) {
    _pin = pin;
    _channel = channel;
    _minPulse = minPulse;
    _maxPulse = maxPulse;

    // В ядре ESP32 v3.x ledcAttach связывает пин, базовую частоту и разрешение
    // Больше не нужно вызывать ledcSetup() раздельно!
    if (!ledcAttach(_pin, freq, resolution)) {
#if DEBUG_MODE
        Serial.printf("[-] Ошибка инициализации LEDC на GPIO%d\n", _pin);
#endif
        return false;
    }
    
    _isInitialized = true;
    _currentAngle = 0;
    _targetAngle = 0;
    refreshPhysicalPWM();

#if DEBUG_MODE
    Serial.printf("[Servo] Плавный ШИМ инициализирован на GPIO%d через LEDC API v3\n", _pin);
#endif
    return true;
}

void VK_Servo::write(int angle) {
    if (!_isInitialized) return;

    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    _targetAngle = angle; 

#if DEBUG_MODE
    Serial.printf("[Servo] Новая цель: %d град.\n", _targetAngle);
#endif
}

void VK_Servo::setSpeedDelay(unsigned long delayMs) {
    _stepDelayMs = delayMs;
}

void VK_Servo::tick() {
    if (!_isInitialized || _currentAngle == _targetAngle) return;

    if (millis() - _lastStepTime >= _stepDelayMs) {
        _lastStepTime = millis();

        if (_currentAngle < _targetAngle) {
            _currentAngle++;
        } else {
            _currentAngle--;
        }

        refreshPhysicalPWM(); 
    }
}

void VK_Servo::refreshPhysicalPWM() {
    if (!_isInitialized) return;
    
    // Рассчитываем длительность импульса в микросекундах
    int pulseUs = _minPulse + (_currentAngle * (_maxPulse - _minPulse) / 180);
    
    // Для разрешения 14 бит максимальное значение регистра ШИМ: (1 << 14) - 1 = 16383
    // Период ШИМ при 50Гц составляет 20000 микросекунд.
    uint32_t maxDuty = (1 << 14) - 1;
    uint32_t duty = (pulseUs * maxDuty) / 20000;

    // В ядре v3.x ledcWrite принимает ПИН, а не номер канала!
    ledcWrite(_pin, duty);
}

int VK_Servo::getAngle() const {
    return _currentAngle;
}

void VK_Servo::detach() {
    if (_isInitialized) {
        ledcDetach(_pin);
        _isInitialized = false;
    }
}
