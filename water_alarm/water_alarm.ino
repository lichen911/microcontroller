#define FLOW_SENSOR_PIN 4
#define BUZZER_PIN 2

volatile int flowValue = 0;
bool alarmActive = false;
int alarmThreshold = 20;
int activeSeconds = 0;
unsigned long lastTime = 0;

unsigned long lastChimeTime = 0;
int chimeDuration = 100; // ms
int initChimeIntervalDelay = 3000;
int chimeIntervalDelay = initChimeIntervalDelay;
int chimeFreq1 = 880;
int chimeFreq2 = 1047;

float flowRateCutOff = -10.0;
int flowRateCutOffCountThreshold = 2;

void IRAM_ATTR flowInterrupt()
{
  flowValue++; // Increment pulse count for each flow
}

void setup()
{
  // Start Serial communication for debugging
  Serial.begin(9600);

  // Initialize flow sensor pin as input
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Attach interrupt to flow sensor pin, trigger on rising edge
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowInterrupt, RISING);
}

void doChime(int pin, int currentTime)
{
  if (currentTime - lastChimeTime >= chimeIntervalDelay)
  {
    lastChimeTime = currentTime;
    tone(pin, chimeFreq1, chimeDuration);
    tone(pin, chimeFreq2, chimeDuration);
  }
}

float calculateRateOfChange(unsigned long currentTime, int currentFlowValue, int &previousFlowValue, unsigned long &previousTime)
{
  float rateOfChange = float(currentFlowValue - previousFlowValue) / float(currentTime - previousTime) * 1000;
  previousFlowValue = currentFlowValue;
  previousTime = currentTime;

  return rateOfChange;
}

void loop()
{
  int currentTime = millis();
  float rateOfFlowChange = 0;
  static int previousFlowValue = 0;
  static unsigned long previousTime = 0;
  static int flowRateCutOffCount = 0;

  // Every second, calculate and print the flow rate
  if (currentTime - lastTime >= 1000)
  {
    rateOfFlowChange = calculateRateOfChange(currentTime, flowValue, previousFlowValue, previousTime);

    Serial.print(currentTime);
    Serial.print(" secs : flow value: ");
    Serial.print(flowValue);
    Serial.print(", rate of change: ");
    Serial.println(rateOfFlowChange);

    if (flowValue >= alarmThreshold && rateOfFlowChange > flowRateCutOff)
    {
      alarmActive = true;
      activeSeconds++;
    }
    else
    {
      if (flowValue < alarmThreshold || flowRateCutOffCount >= flowRateCutOffCountThreshold)
      {
        alarmActive = false;
        activeSeconds = 0;
      }
      else
      {
        flowRateCutOffCount++;
      }
    }

    // Reset pulse count for next second
    flowValue = 0;
    lastTime = currentTime;

    // Decrease chime interval
    if (activeSeconds > 0 && activeSeconds % 10 == 0)
    {
      chimeIntervalDelay = chimeIntervalDelay / 2;
      Serial.print("Updating chime interval: ");
      Serial.println(chimeIntervalDelay, activeSeconds % 10 == 0);
    }
    if (activeSeconds == 0 && chimeIntervalDelay != initChimeIntervalDelay)
    {
      chimeIntervalDelay = initChimeIntervalDelay;
    }
  }

  // Do chime if it's time
  if (alarmActive)
  {
    doChime(BUZZER_PIN, currentTime);
  }
}
