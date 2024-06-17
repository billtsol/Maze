
const unsigned int TRIG_PIN=19;
const unsigned int ECHO_PIN=20;

void setup()
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(115200);

  delay(10);

}

void loop(){


  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN,  HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration= pulseIn(ECHO_PIN, HIGH);
  int distance= duration/29/2;
  if(duration==0){
    Serial.println("Warning: no pulse from sensor");
  }
  else{
    Serial.print("distance to nearest object: ");
    Serial.print(distance);
    Serial.print(" cm");
    Serial.println("");
  }
  delay(100);

}
