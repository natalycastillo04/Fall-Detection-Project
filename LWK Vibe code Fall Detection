/*
  Fall Detection using MPU6050 + Arduino Uno R4
  ------------------------------------------------
  Libraries required (Library Manager):
    - Adafruit MPU6050
    - Adafruit Unified Sensor
    - Wire (built-in)

  Wiring (Uno R4 WiFi/Minima, I2C):
    MPU6050 VCC -> 5V (breakout usually has onboard 3.3V regulator)
    MPU6050 GND -> GND
    MPU6050 SCL -> SCL (or A5)
    MPU6050 SDA -> SDA (or A4)

  Algorithm:
    1. Compute the magnitude of total acceleration (in g) each sample.
    2. FREE FALL phase: magnitude drops below FREEFALL_THRESH for at
       least FREEFALL_MIN_MS  -> body is briefly weightless.
    3. IMPACT phase: within IMPACT_WINDOW_MS after free fall ends,
       magnitude spikes above IMPACT_THRESH -> sudden stop / hit.
    4. STILLNESS check: after impact, acceleration settles near 1g and
       stays fairly constant for STILLNESS_MS -> person is down/not moving.
    If all three conditions occur in sequence, a fall is declared.
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// ---- Tunable thresholds ----
const float FREEFALL_THRESH   = 0.4;   // g, below this = free fall
const float IMPACT_THRESH     = 2.5;   // g, above this = impact
const float STILL_LOW         = 0.85;  // g, lower bound for "at rest"
const float STILL_HIGH        = 1.15;  // g, upper bound for "at rest"

const unsigned long FREEFALL_MIN_MS   = 100;   // min duration to count as free fall
const unsigned long IMPACT_WINDOW_MS  = 1000;  // must see impact within this long after free fall
const unsigned long STILLNESS_MS      = 1500;  // must stay still this long after impact
const unsigned long SAMPLE_INTERVAL_MS = 20;   // ~50 Hz sampling

// ---- State machine ----
enum FallState { IDLE, FREEFALL, IMPACT_WAIT, STILL_CHECK };
FallState state = IDLE;

unsigned long stateStartTime = 0;
unsigned long freefallDuration = 0;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 ready. Monitoring for falls...");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) return;
  lastSampleTime = now;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Convert m/s^2 -> g, then magnitude
  float ax = a.acceleration.x / 9.80665;
  float ay = a.acceleration.y / 9.80665;
  float az = a.acceleration.z / 9.80665;
  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  runFallStateMachine(magnitude, now);
}

void runFallStateMachine(float magnitude, unsigned long now) {
  switch (state) {

    case IDLE:
      if (magnitude < FREEFALL_THRESH) {
        state = FREEFALL;
        stateStartTime = now;
      }
      break;

    case FREEFALL:
      if (magnitude < FREEFALL_THRESH) {
        freefallDuration = now - stateStartTime;
      } else {
        // free fall ended; did it last long enough?
        if (freefallDuration >= FREEFALL_MIN_MS) {
          state = IMPACT_WAIT;
          stateStartTime = now;
        } else {
          state = IDLE; // too short, probably noise
        }
      }
      break;

    case IMPACT_WAIT:
      if (magnitude > IMPACT_THRESH) {
        state = STILL_CHECK;
        stateStartTime = now;
      } else if (now - stateStartTime > IMPACT_WINDOW_MS) {
        state = IDLE; // no impact detected in time, reset
      }
      break;

    case STILL_CHECK:
      if (magnitude < STILL_LOW || magnitude > STILL_HIGH) {
        // still moving significantly, restart the stillness timer
        stateStartTime = now;
      } else if (now - stateStartTime >= STILLNESS_MS) {
        triggerFallAlert();
        state = IDLE;
      }
      break;
  }
}

void triggerFallAlert() {
  Serial.println("!!! FALL DETECTED !!!");
  // TODO: add your response here, e.g.:
  //   digitalWrite(BUZZER_PIN, HIGH);
  //   send SMS/notification via GSM or WiFi module
  //   log timestamp to SD card
}
