#include <Wire.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup()
{

  Serial.begin(115200);

  Wire.begin(13, 14, 100000); // sda, scl, clock speed
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU−6050)
  Wire.endTransmission(true);
  Serial.println("Setup complete");

  delay(10);
}

void loop()
{

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR);
  Wire.requestFrom(MPU_ADDR, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) &  0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  // Print raw values
  Serial.print("Accel (g)");
  Serial.print(" X=");
  Serial.print(AcX);
  Serial.print(" Y=");
  Serial.print(AcY);
  Serial.print(" Z=");
  Serial.print(AcZ);
  Serial.print(" Gyro (deg/s)");
  Serial.print(" X=");
  Serial.print(GyX);
  Serial.print(" Y=");
  Serial.print(GyY);
  Serial.print(" Z=");
  Serial.println(GyZ);
  
  // Interpret accelerometer values
  Serial.print("Orientation: ");
  if (AcX < -15000) {
    Serial.print("Tilted Left, ");
  } else if (AcX > 15000) {
    Serial.print("Tilted Right, ");
  } else {
    Serial.print("Level Left-Right, ");
  }
  
  if (AcY < -15000) {
    Serial.print("Tilted Forward, ");
  } else if (AcY > 15000) {
    Serial.print("Tilted Backward, ");
  } else {
    Serial.print("Level Forward-Backward, ");
  }
  
  if (AcZ < 0) {
    Serial.println("Upside Down");
  } else {
    Serial.println("Upright");
  }
  
  // Interpret gyroscope values
  Serial.print("Movement: ");
  if (GyX < -2000) {
    Serial.print("Rotating Left, ");
  } else if (GyX > 2000) {
    Serial.print("Rotating Right, ");
  } else {
    Serial.print("Stable X-axis, ");
  }
  
  if (GyY < -2000) {
    Serial.print("Rotating Forward, ");
  } else if (GyY > 2000) {
    Serial.print("Rotating Backward, ");
  } else {
    Serial.print("Stable Y-axis, ");
  }
  
  if (GyZ < -2000) {
    Serial.println("Rotating Counterclockwise");
  } else if (GyZ > 2000) {
    Serial.println("Rotating Clockwise");
  } else {
    Serial.println("Stable Z-axis");
  }

  delay(1000);
}
