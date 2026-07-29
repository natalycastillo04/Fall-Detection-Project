#include <Adafruit_NeoPixel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Buzzer
const int buzzerPin = 8;

//UltraSonic Sensor
const int echoPin = 9;
const int trigPin = 10;
const int motionThresh = 20;

// Automatic LED LightBar
const int ledPin = 11;
const int numPixels = 8;
const int ldrPin = A0;
const int darkThresh = 200;

// Accelerometer
Adafruit_MPU6050 acc; 


Adafruit_NeoPixel pixels(numPixels, ledPin, NEO_GRBW + NEO_KHZ800);
void neopixelStick(int lightLevel, float distance);
float motionDetect();
bool fallDetect();
void serialMonitor(int lightLevel, float distance);
void strobe();

// SETUP
void setup() 
{
  Serial.begin(9600);

  // LED Bar Setup
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Set middle two pixels green for "on" state
  pixels.setPixelColor(3, pixels.Color(0, 255, 0));
  pixels.setPixelColor(4, pixels.Color(0, 255, 0));
  pixels.setBrightness(150);
  pixels.show();

  // Ultrasonic Sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

// Accelerometer Setup
  pinMode(buzzerPin, OUTPUT);

  if (!acc.begin()) {
  Serial.println("MPU6050 not found!");
  while (1);
}
  acc.setAccelerometerRange(MPU6050_RANGE_8_G);
  acc.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  

}

// LOOP
void loop() 
{

  float distance = motionDetect();

  int lightLevel = analogRead(ldrPin);
  neopixelStick(lightLevel, distance);

  serialMonitor(lightLevel, distance);

  if (fallDetect())
  {
    Serial.println("Fall DETECTED!");
  }

  delay(50);
}

// LED Bar
void neopixelStick(int lightLevel, float distance)
{
  if (distance <= motionThresh)
  {
    strobe();
  }
  else if (lightLevel < darkThresh) 
  {
    // Dark: Turn all LEDs on
    for (int i = 0; i < numPixels; i++) 
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0, 255)); // white
    }
    pixels.show();
  }

  else
  {
    // Bright: Turn all LEDs off
    pixels.clear();
    pixels.setPixelColor(3, pixels.Color(0, 255, 0));
    pixels.setPixelColor(4, pixels.Color(0, 255, 0));
    pixels.setBrightness(50);
    pixels.show();

  }
}

// Ultrasonic Sensor
float motionDetect()
{
  float distance;
  long duration;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  distance = (0.0343 * duration) / 2;

  if (distance == 0)
  {
    return 999;
  }

  return distance;
}

bool fallDetect()
{
  sensors_event_t accel, g, temp;
  acc.getEvent(&accel, &g, &temp);

  /* 
  Serial.print("Acceleration X: ");
  Serial.print(accel.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(accel.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(accel.acceleration.z);
  Serial.println(" m/s^2");
  */

  float previous, current;

  while()
  previous = accel.acceleration.y;
  Serial.print("Previous: ");
  Serial.print(previous);
  Serial.print(".   |.    ");
 

  delay(3000);

  current = accel.acceleration.y;
  Serial.print("Current: ");
  Serial.print(current);
  Serial.print(".   |.    ");
 


  float diff_y = abs(current - previous);
  Serial.println(diff_y);

  if (diff_y >= 2.4){
    return true;
  }

  return false;
}

void strobe()
{
  static bool on = false;

  on = !on;   // Flip between ON and OFF

  if (on)
  {
    for (int i = 0; i < numPixels; i++)
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
  }
  else
  {
    pixels.clear();
  }

  pixels.show();

  delay(75);   // Flash speed
}



void serialMonitor(int lightLevel, float distance)
{
  /*
  Serial.print("LDR: ");
  Serial.print(lightLevel);

  Serial.print(" | Distance: ");
  Serial.print(distance, 1);
  Serial.print(" cm");

  Serial.print(" | LEDs: ");
  if (lightLevel < darkThresh)
    Serial.print("WHITE");
  else
    Serial.print("GREEN");

  Serial.print(" | Motion: ");
  if (distance <= motionThresh)
    Serial.println("YES");
  else
    Serial.println("NO");
  */
}
