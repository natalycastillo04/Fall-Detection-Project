// photoresistor and led 

// analog pin for photosensor
int photosensorAnalog = A0;

// digital led pin 
int ledPin = 5; 

//dark threshold 
int darkThreshold = 100; 


void setup() {

  // set ledPin as an output
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  
  // read the light level using photosensor
  int lightLevel = analogRead(photosensorAnalog);

  // if theres light outside, turn led off
  if (lightLevel >= darkThreshold) {
    digitalWrite(ledPin, LOW);
  }
  else {
    // if dark out, turn led on 
    digitalWrite(ledPin, HIGH);
  }
}