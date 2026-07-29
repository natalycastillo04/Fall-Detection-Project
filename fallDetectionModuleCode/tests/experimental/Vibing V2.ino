#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

/*
  MPU6050 FALL-DETECTION TEST
  Arduino Uno R4 + MPU6050

  Detection sequence:
  1. Low acceleration
  2. High impact acceleration
  3. High angular velocity
*/

// -------------------------
// Adjustable thresholds
// -------------------------

// Possible free-fall or loss of support
const float LOWER_ACCEL_THRESHOLD = 0.55;  // g

// Possible impact
const float UPPER_ACCEL_THRESHOLD = 2.20;  // g

// Rapid rotation
const float GYRO_THRESHOLD = 180.0;        // degrees per second

// Maximum time allowed between low acceleration and impact
const unsigned long IMPACT_WINDOW = 1000;  // milliseconds

// Maximum time allowed between impact and rotation
const unsigned long GYRO_WINDOW = 500;     // milliseconds

// Time before detector resets after a confirmed fall
const unsigned long RESET_DELAY = 3000;    // milliseconds

// How often sensor data is printed
const unsigned long PRINT_INTERVAL = 200;  // milliseconds

// -------------------------
// Fall-detection states
// -------------------------

enum FallState {
  MONITORING,
  LOW_ACCEL_DETECTED,
  IMPACT_DETECTED,
  FALL_CONFIRMED
};

FallState fallState = MONITORING;

// -------------------------
// Timing variables
// -------------------------

unsigned long lowAccelTime = 0;
unsigned long impactTime = 0;
unsigned long fallDetectedTime = 0;
unsigned long previousPrintTime = 0;

// -------------------------
// Setup
// -------------------------

void setup() {
  Serial.begin(115200);

  // Wait briefly for Serial Monitor, but do not wait forever
  unsigned long serialStartTime = millis();

  while (!Serial && millis() - serialStartTime < 3000) {
    delay(10);
  }

  Serial.println();
  Serial.println("====================================");
  Serial.println("MPU6050 Fall Detection Test");
  Serial.println("Arduino Uno R4");
  Serial.println("====================================");

  Wire.begin();

  Serial.println("Searching for MPU6050...");

  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not detected.");
    Serial.println("Check the following connections:");
    Serial.println("VCC -> 5V or 3.3V");
    Serial.println("GND -> GND");
    Serial.println("SDA -> SDA");
    Serial.println("SCL -> SCL");

    while (true) {
      delay(100);
    }
  }

  Serial.println("MPU6050 detected successfully.");

  /*
    Use the ±8 g range so strong impacts are less likely
    to exceed the sensor's measurement range.
  */
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  /*
    Use the ±500 degrees/second gyroscope range.
  */
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  /*
    Apply a moderate digital filter.
  */
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

  delay(500);

  Serial.println();
  Serial.println("Threshold settings:");
  Serial.print("Lower acceleration threshold: ");
  Serial.print(LOWER_ACCEL_THRESHOLD);
  Serial.println(" g");

  Serial.print("Upper acceleration threshold: ");
  Serial.print(UPPER_ACCEL_THRESHOLD);
  Serial.println(" g");

  Serial.print("Gyroscope threshold: ");
  Serial.print(GYRO_THRESHOLD);
  Serial.println(" deg/s");

  Serial.println();
  Serial.println("Fall detector is active.");
  Serial.println("Open Serial Monitor at 115200 baud.");
  Serial.println();
}

// -------------------------
// Main loop
// -------------------------

void loop() {
  sensors_event_t accelEvent;
  sensors_event_t gyroEvent;
  sensors_event_t tempEvent;

  // Read MPU6050 sensor data
  mpu.getEvent(&accelEvent, &gyroEvent, &tempEvent);

  /*
    Adafruit acceleration values are in meters per second squared.
    Divide by 9.80665 to convert them into g.
  */
  float ax = accelEvent.acceleration.x / 9.80665;
  float ay = accelEvent.acceleration.y / 9.80665;
  float az = accelEvent.acceleration.z / 9.80665;

  /*
    Adafruit gyroscope values are in radians per second.
    Convert them into degrees per second.
  */
  float gx = gyroEvent.gyro.x * 180.0 / PI;
  float gy = gyroEvent.gyro.y * 180.0 / PI;
  float gz = gyroEvent.gyro.z * 180.0 / PI;

  /*
    Calculate total acceleration magnitude:

    Acc = sqrt(Ax² + Ay² + Az²)
  */
  float totalAcceleration = sqrt(
    ax * ax +
    ay * ay +
    az * az
  );

  /*
    Calculate total angular velocity magnitude:

    W = sqrt(Gx² + Gy² + Gz²)
  */
  float totalAngularVelocity = sqrt(
    gx * gx +
    gy * gy +
    gz * gz
  );

  unsigned long currentTime = millis();

  // Run the fall-detection state machine
  detectFall(
    totalAcceleration,
    totalAngularVelocity,
    currentTime
  );

  // Print sensor values for testing and calibration
  if (currentTime - previousPrintTime >= PRINT_INTERVAL) {
    previousPrintTime = currentTime;

    printSensorData(
      ax,
      ay,
      az,
      gx,
      gy,
      gz,
      totalAcceleration,
      totalAngularVelocity
    );
  }

  // Approximately 100 sensor samples per second
  delay(10);
}

// -------------------------
// Fall-detection logic
// -------------------------

void detectFall(
  float totalAcceleration,
  float totalAngularVelocity,
  unsigned long currentTime
) {
  switch (fallState) {

    case MONITORING:
      /*
        Stage 1:
        Look for acceleration below the lower threshold.
      */
      if (totalAcceleration < LOWER_ACCEL_THRESHOLD) {
        fallState = LOW_ACCEL_DETECTED;
        lowAccelTime = currentTime;

        Serial.println();
        Serial.println("Stage 1: Low acceleration detected.");
        Serial.print("Acceleration: ");
        Serial.print(totalAcceleration, 2);
        Serial.println(" g");
      }
      break;

    case LOW_ACCEL_DETECTED:
      /*
        Stage 2:
        Look for an impact within the allowed time window.
      */
      if (totalAcceleration > UPPER_ACCEL_THRESHOLD) {
        fallState = IMPACT_DETECTED;
        impactTime = currentTime;

        Serial.println("Stage 2: Impact detected.");
        Serial.print("Acceleration: ");
        Serial.print(totalAcceleration, 2);
        Serial.println(" g");
      }

      /*
        Cancel the event if no impact occurs in time.
      */
      else if (currentTime - lowAccelTime > IMPACT_WINDOW) {
        Serial.println("Event cancelled: no impact followed low acceleration.");
        fallState = MONITORING;
      }
      break;

    case IMPACT_DETECTED:
      /*
        Stage 3:
        Look for rapid rotation after the impact.
      */
      if (totalAngularVelocity > GYRO_THRESHOLD) {
        fallState = FALL_CONFIRMED;
        fallDetectedTime = currentTime;

        Serial.println("Stage 3: Rapid rotation detected.");
        Serial.print("Angular velocity: ");
        Serial.print(totalAngularVelocity, 1);
        Serial.println(" deg/s");

        Serial.println();
        Serial.println("********************************");
        Serial.println("POSSIBLE FALL DETECTED");
        Serial.println("********************************");
        Serial.println();
      }

      /*
        Cancel the event if the rotation threshold
        is not reached in time.
      */
      else if (currentTime - impactTime > GYRO_WINDOW) {
        Serial.println("Event cancelled: rotation was too low.");
        fallState = MONITORING;
      }
      break;

    case FALL_CONFIRMED:
      /*
        Wait before returning to normal monitoring.
      */
      if (currentTime - fallDetectedTime > RESET_DELAY) {
        Serial.println("Fall detector reset.");
        Serial.println();

        fallState = MONITORING;
      }
      break;
  }
}

// -------------------------
// Print sensor information
// -------------------------

void printSensorData(
  float ax,
  float ay,
  float az,
  float gx,
  float gy,
  float gz,
  float totalAcceleration,
  float totalAngularVelocity
) {
  Serial.print("Accel X: ");
  Serial.print(ax, 2);

  Serial.print(" | Y: ");
  Serial.print(ay, 2);

  Serial.print(" | Z: ");
  Serial.print(az, 2);

  Serial.print(" | Total: ");
  Serial.print(totalAcceleration, 2);
  Serial.print(" g");

  Serial.print(" || Gyro X: ");
  Serial.print(gx, 1);

  Serial.print(" | Y: ");
  Serial.print(gy, 1);

  Serial.print(" | Z: ");
  Serial.print(gz, 1);

  Serial.print(" | Total: ");
  Serial.print(totalAngularVelocity, 1);
  Serial.print(" deg/s");

  Serial.print(" || State: ");
  printState();
}

// -------------------------
// Print current detector state
// -------------------------

void printState() {
  switch (fallState) {
    case MONITORING:
      Serial.println("MONITORING");
      break;

    case LOW_ACCEL_DETECTED:
      Serial.println("LOW ACCELERATION");
      break;

    case IMPACT_DETECTED:
      Serial.println("IMPACT DETECTED");
      break;

    case FALL_CONFIRMED:
      Serial.println("FALL CONFIRMED");
      break;
  }
}
