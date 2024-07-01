
int led1_r = 21; 
int led1_g = 47;
int led1_b = 48;

void setup() {

  Serial.begin(115200);

  pinMode(led1_r, OUTPUT);
  pinMode(led1_g, OUTPUT);
  pinMode(led1_b, OUTPUT);

}

void loop() {

  // Red color
  Serial.println("Red color");
  setColor(4095, 0, 0);
  delay(3000);

  // Green color
  Serial.println("Green color");
  setColor(0, 4095, 0);
  delay(3000);

  // Blue color
  Serial.println("Blue color");
  setColor(0, 0, 4095);
  delay(3000);

   // Yellow color
  Serial.println("Yellow color");
  setColor(4095, 4095, 0);
  delay(3000);

  // Magenta color
  Serial.println("Magenta color");
  setColor(4095, 0, 4095);
  delay(3000);

}


// Function to set the color of the RGB LED
void setColor(int red, int green, int blue) {

  // Write values to the RGB pins
  analogWrite(led1_r, red);
  analogWrite(led1_g, green);
  analogWrite(led1_b, blue);

}
