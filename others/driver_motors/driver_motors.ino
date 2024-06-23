// Tutorial
// https://lastminuteengineers.com/drv8833-arduino-tutorial/

// Left Motor
int aphase = 4;
int aenbl = 5;

// Right Motor
int bphase = 6;
int benbl = 7;

// O1 deksia.
// O2 aristera.

int base_speed = 100;

void setup() {

  Serial.begin(115200);

  pinMode(aphase, OUTPUT);
  pinMode(aenbl, OUTPUT);
  pinMode(bphase, OUTPUT);
  pinMode(benbl, OUTPUT);

}

void loop() {
  forward_movement(base_speed, base_speed);
  delay(5000);

  forward_movement(-base_speed, -base_speed);
  delay(5000);
}

void forward_movement(int speedA, int speedB) {
  if (speedA < 0) {
    speedA = 0 - speedA;
    digitalWrite(aphase, LOW);
  }
  else {
    digitalWrite(aphase, HIGH);
  }

  if (speedB < 0) {
    speedB = 0 - speedB;
    digitalWrite(bphase, HIGH);
  }
  else {
    digitalWrite(bphase, LOW);
  }

  analogWrite(aenbl, speedA);
  analogWrite(benbl, speedB);
}