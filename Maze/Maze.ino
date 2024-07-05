/* ********************************** Maze Robot **********************************





*/

/* ********************************** Include the necessary libraries ********************************** */
#include <Adafruit_NeoPixel.h> // RGB Led library
#include <WiFi.h>              // WiFi library
#include <Wire.h>              // Gyroscope library

/* ********************************** Include the necessary pins ********************************** */
#define BUILDIN_RGB_LED_PIN 48 // Define the pins for the RGB Led

#define TRIG_PIN = 19; // Enable the trigger pin for the distance sensor
#define ECHO_PIN = 20; // Enable the echo pin for the distance sensor

#define RECV_PIN = 1; // Enable the receive pin for the distance sensor

#define EXTERNAL_RGB_LED_RED_PIN = 21;   // Red pin for the external RGB LED
#define EXTERNAL_RGB_LED_GREEN_PIN = 47; // Green pin for the external RGB LED
#define EXTERNAL_RGB_LED_BLUE_PIN = 48;  // Blue pin for the external RGB LED

#define MOTOR_LEFT_A_PHASE_PIN = 4;  // Define the pin for the left motor speed
#define MOTOR_LEFT_A_ENABLE_PIN = 5; // Define the enable pin for the left motor

#define MOTOR_RIGHT_B_PHASE_PIN = 6;  // Define the pin for the right motor speed
#define MOTOR_RIGHT_B_ENABLE_PIN = 7; // Define the enable pin for the right motor

// Define the ir sensors pins
#define IR_SENSOR_1 3;
#define IR_SENSOR_2 8;
#define IR_SENSOR_3 15;
#define IR_SENSOR_4 16;
#define IR_SENSOR_5 17;
#define IR_SENSOR_6 18;

/* ********************************** Include the necessary variables ********************************** */
Adafruit_NeoPixel pixels(1, BUILDIN_RGB_LED_PIN, NEO_GRB + NEO_KHZ800); // Define the buildin RGB Led

const char *ssid = "JM-03DC80";    // Define the SSID for the WiFi
const char *password = "password"; // Define the password for the WiFi
WiFiServer server(80);             // Define the server for the WiFi

const int MPU_ADDR = 0x68;                 // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Define the variables for the gyroscope

const int base_speed = 100; // Define the base speed for the motors

/* ********************************** Setup function ********************************** */
void setup()
{

  Serial.begin(115200); // Configure the serial port
  delay(20);

  pixels.begin(); // Start the buildin RGB Led

  /* ********************************** Set the pin mode for all devices ********************************** */
  pinMode(RECV_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MOTOR_LEFT_A_PHASE_PIN, OUTPUT);
  pinMode(MOTOR_LEFT_A_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_RIGHT_B_PHASE_PIN, OUTPUT);
  pinMode(MOTOR_RIGHT_B_ENABLE_PIN, OUTPUT);

  pinMode(EXTERNAL_RGB_LED_RED_PIN, OUTPUT);
  pinMode(EXTERNAL_RGB_LED_GREEN_PIN, OUTPUT);
  pinMode(EXTERNAL_RGB_LED_BLUE_PIN, OUTPUT);

  /* ********************************** Set up the gyroscope ********************************** */
  Wire.begin(13, 14, 100000); // sda, scl, clock speed
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU−6050)
  Wire.endTransmission(true);
  Serial.println("Setup complete");

  /* ********************************** Set up the WiFi ********************************** */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

/* ********************************** Set up loop function ********************************** */
void loop()
{
}

/* ********************************** Set up external functions ********************************** */
