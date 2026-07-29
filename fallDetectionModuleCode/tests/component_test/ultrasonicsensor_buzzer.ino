const int trigPin = 10;
const int echoPin = 11;
const int buzzerPin = 12;
float distance;
long duration;

void setup() 
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
}

void loop() 
{

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  distance = (0.0343 * duration) / 2;

  if (distance == 0)
  {
    distance = 999;
    noTone(buzzerPin);
  }

  else
  {
      if (distance <= 3)
    {
      tone(buzzerPin, 1000);
    }

    else if (distance <= 12)
    {
     tone(buzzerPin, 500);
      delay(200);
      noTone(buzzerPin);
      delay(200);
   }

    else if(distance <= 20)
    {
      tone(buzzerPin, 300);
      delay(400);
      noTone(buzzerPin);
      delay(400);
    }

    else
    {
      noTone(buzzerPin);
    }

  }

  
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(50);
}
