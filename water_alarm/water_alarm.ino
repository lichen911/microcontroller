#define FLOW_SENSOR_PIN 4  // GPIO pin connected to the sensor's data pin
#define BUZZER_PIN 2

volatile int flowCount = 0;  // To count the number of pulses
bool alarmActive = false;
int alarmThreshold = 20;
int activeSeconds = 0;
unsigned long lastTime = 0;  // Time tracking for flow rate calculation

unsigned long lastChimeTime = 0;
int chimeDuration = 100;  // ms
int initChimeIntervalDelay = 3000;
int chimeIntervalDelay = initChimeIntervalDelay;
int chimeFreq1 = 880;
int chimeFreq2 = 1047;

void IRAM_ATTR flowInterrupt() {
  flowCount++;  // Increment pulse count for each flow
}

void setup() {
  // Start Serial communication for debugging
  Serial.begin(9600);

  // Initialize flow sensor pin as input
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Attach interrupt to flow sensor pin, trigger on rising edge
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowInterrupt, RISING);
}

void doChime(int pin) {
  if (millis() - lastChimeTime >= chimeIntervalDelay) {
    lastChimeTime = millis();
    tone(pin, chimeFreq1, chimeDuration);
    tone(pin, chimeFreq2, chimeDuration);
  }
}

void loop() {
  // Every second, calculate and print the flow rate
  if (millis() - lastTime >= 1000) {
    Serial.print("Flow count: ");
    Serial.println(flowCount);

    if (flowCount >= alarmThreshold) {
      alarmActive = true;
      activeSeconds++;
    } else {
      alarmActive = false;
      activeSeconds = 0;
    }

    // Reset pulse count for next second
    flowCount = 0;
    lastTime = millis();

    // Decrease chime interval
    if (activeSeconds > 0 && activeSeconds % 10 == 0) {
      chimeIntervalDelay = chimeIntervalDelay / 2;
      Serial.println(chimeIntervalDelay, activeSeconds % 10 == 0);
    }
    if (activeSeconds == 0 && chimeIntervalDelay != initChimeIntervalDelay) {
      chimeIntervalDelay = initChimeIntervalDelay;
    }
  }

  // Do chime if it's time
  if (alarmActive) {
    doChime(BUZZER_PIN);
  }
}
