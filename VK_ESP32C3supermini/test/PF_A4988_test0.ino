// ESP32-C3 Supermini + A4988 (шаговый) + MG996R (серва)
// DIR -> GPIO3, STEP -> GPIO1, EN -> GND
// Серва -> GPIO4 (через LEDC, без библиотеки, старый API)

#define DIR_PIN   1
#define STEP_PIN  3
#define SERVO_PIN 4

// --- Шаговый мотор ---
const int STEPS_PER_REV = 200;   // full-step для NEMA 8
const int STEP_DELAY_US = 2000;  // 2000 мкс = медленно и надёжно

// --- Сервопривод (LEDC) ---
const int PWM_CHANNEL    = 0;     // канал LEDC (0..15)
const int PWM_FREQ       = 50;    // 50 Гц
const int PWM_RESOLUTION = 14;    // 14 бит
const int MIN_PULSE_US   = 500;   // импульс для 0°
const int MAX_PULSE_US   = 2400;  // импульс для 180°

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Final test: stepper + servo (legacy LEDC)");

  // --- Шаговый ---
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);

  // --- Серва (старый API) ---
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(SERVO_PIN, PWM_CHANNEL);
  writeServo(0);

  delay(1000);
}

// --- Управление сервой ---
void writeServo(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  int pulseUs = MIN_PULSE_US + (angle * (MAX_PULSE_US - MIN_PULSE_US) / 180);
  uint32_t maxDuty = (1 << PWM_RESOLUTION) - 1;
  uint32_t duty = (pulseUs * maxDuty) / 20000;  // 20000 мкс = 20 мс

  ledcWrite(PWM_CHANNEL, duty);
}

// --- Управление шаговым мотором ---
void stepMotor(int steps, bool direction) {
  digitalWrite(DIR_PIN, direction ? HIGH : LOW);
  delayMicroseconds(5);  // дать драйверу время на смену направления

  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }
}

void loop() {
  // 1. Шаговый в одну сторону
  Serial.println("Stepper: forward");
  stepMotor(STEPS_PER_REV, true);
  delay(1000);

  // 2. Серва 0 -> 180
  Serial.println("Servo: 0 -> 180");
  for (int a = 0; a <= 180; a++) {
    writeServo(a);
    delay(15);
  }
  delay(500);

  // 3. Шаговый в другую сторону
  Serial.println("Stepper: backward");
  stepMotor(STEPS_PER_REV, false);
  delay(1000);

  // 4. Серва 180 -> 0
  Serial.println("Servo: 180 -> 0");
  for (int a = 180; a >= 0; a--) {
    writeServo(a);
    delay(15);
  }

  // 5. Пауза перед повтором
  Serial.println("Pause 2s");
  delay(2000);
}
