
int RECV_PIN = 1;

void setup() {
  Serial.begin(115200);
  pinMode(RECV_PIN, INPUT);
}

void loop(){

  int ir_reciver = digitalRead(RECV_PIN);
  if (ir_reciver != 1) {
    Serial.println(ir_reciver);
  }

}
