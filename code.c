#define trigPin #define echoPin 
#define LEDlampRed 
#define LEDlampYellow 
#define LEDlampGreen 
#define soundbuzzer int sound = 500;
void setup() 
{
  Serial.begin (9600); 
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  pinMode(LEDlampRed, OUTPUT); 
  pinMode(LEDlampYellow, OUTPUT); 
  pinMode(LEDlampGreen, OUTPUT); 
  pinMode(soundbuzzer, OUTPUT); 
}
void loop() 
{
  long durationindigit, distanceincm; 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW); 
  durationindigit = pulseIn(echoPin, HIGH); 
  distanceincm = (durationindigit/5) / 29.1; 
  if (distanceincm < 50) 
  {
    digitalWrite(LEDlampGreen, HIGH); 
  }
  Else 
  {
    digitalWrite(LEDlampGreen, LOW); 
}
if (distance < 20) 
{
  digitalWrite(LEDlampYellow, HIGH); 
}
Else 
{
  digitalWrite(LEDlampYellow,LOW); 
  }
  if (distance < 5) 
{
  digitalWrite(LEDlampRed, HIGH); sound = 1000; 
}
Else 
{
  digitalWrite(LEDlampRed,LOW); 
  }
  if (distanceincm > 5 || distanceinsm <= 0) 
{
  Serial.println("Outside the permissible range of distances"); 
  noTone(soundbuzzer); 
}
Else 
{
  Serial.print(distance); 
Serial.println(" cm"); 
tone(buzzer, sound); 
}
delay(300); 
}
