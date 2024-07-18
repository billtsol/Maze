/* ********** Maze Robot **********






*/

/* ********** Include the necessary libraries ********** */
#include <Adafruit_NeoPixel.h> // RGB Led library
#include <WiFi.h>              // WiFi library
#include <Wire.h>              // Gyroscope library
#include <HTTPClient.h>        // Http client library

/* ********** Include the necessary pins ********** */
#define BUILDIN_RGB_LED_PIN 48 // Define the pins for the RGB Led

#define TRIG_PIN 19 // Enable the trigger pin for the distance sensor
#define ECHO_PIN 20 // Enable the echo pin for the distance sensor

#define RECV_PIN 1 // Enable the receive pin for the distance sensor

#define EXTERNAL_RGB_LED_RED_PIN 21   // Red pin for the external RGB LED
#define EXTERNAL_RGB_LED_GREEN_PIN 47 // Green pin for the external RGB LED
#define EXTERNAL_RGB_LED_BLUE_PIN 48  // Blue pin for the external RGB LED

#define MOTOR_LEFT_A_PHASE_PIN 4  // Define the pin for the left motor speed
#define MOTOR_LEFT_A_ENABLE_PIN 5 // Define the enable pin for the left motor

#define MOTOR_RIGHT_B_PHASE_PIN 6  // Define the pin for the right motor speed
#define MOTOR_RIGHT_B_ENABLE_PIN 7 // Define the enable pin for the right motor

// Define the ir sensors pins
#define IR_SENSOR_1 3  // left sensor
#define IR_SENSOR_2 8  // left sensor
#define IR_SENSOR_3 15 // Straight sensor
#define IR_SENSOR_4 16 // right sensor
#define IR_SENSOR_5 17 // right sensor
#define IR_SENSOR_6 18 // Straight sensor

/* ********** Include the necessary variables ********** */
Adafruit_NeoPixel pixels(1, BUILDIN_RGB_LED_PIN, NEO_GRB + NEO_KHZ800); // Define the build in    RGB Led

const char *serverName = "http://192.168.2.2:3000/api/data"; // Define the server ip address
const char *ssid = "VODAFONE_0152";                          // Define the SSID for the WiFi
const char *password = "2egpa5u95tfh73b8";                   // Define the password for the WiFi
WiFiServer server(80);                                       // Define the server for the WiFi

const int MPU_ADDR = 0x68;                 // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Define the variables for the gyroscope

const int base_speed = 100; // Define the base speed for the motors

/* ********** Setup function ********** */
void setup()
{

  Serial.begin(115200); // Configure the serial port
  delay(20);

  pixels.begin(); // Start the build in RGB Led

  /* ********** Set the pin mode for all devices ********** */
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

  /* ********** Set up the gyroscope ********** */
  Wire.begin(13, 14, 100000); // sda, scl, clock speed
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU−6050)
  Wire.endTransmission(true);
  Serial.println("Setup complete");

  /* ********** Set up the WiFi ********** */
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

/* ********** Set up loop function ********** */
void loop()
{
}

/* ********** Set up external functions ********** */
void mazeSolver()
{
  int left_wall, right_wall, straigt_wall;
  left_wall = right_wall = straigt_wall = 0;

  readIRSensorsValues(&left_wall, &right_wall, &straigt_wall);

  // Send data to the server to create the map.
  sentDataToServer(left_wall, right_wall, straigt_wall);

  // Calculate the new speed values for the motors
}

void sentDataToServer(int left_wall, int right_wall, int straight_wall)
{
  // Check Wi-Fi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    // Specify request destination
    http.begin(serverName);

    // Set HTTP header
    http.addHeader("Content-Type", "application/json"); // JSON format

    // Prepare JSON data
    String jsonPayload = "{
      \"left_wall\": \" " + String(left_wall) + "\",
      \"right_wall\": \" " + String(right_wall) + "\",
      \"straight_wall\": \" " + String(straight_wall) + "\"}";

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonPayload);

    // Check the response code
    if (httpResponseCode > 0)
    {
      String response = http.getString(); // Get response payload
      Serial.println(httpResponseCode);   // Print HTTP response code
      Serial.println(response);           // Print response
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    // End HTTP connection
    http.end();
  }
  else
  {
    Serial.println("Error in Wi-Fi connection");
  }

  delay(5000); // Send a request every 10 seconds (adjust as needed)
}

void readIRSensorsValues(int *left_wall, int *right_wall, int *straight_wall)
{
  // Read the values from he IR sensors
  int left1 = analogRead(IR_SENSOR_1); // left sensor
  int left2 = analogRead(IR_SENSOR_2); // left sensor

  int right1 = analogRead(IR_SENSOR_4); // right sensor
  int right2 = analogRead(IR_SENSOR_5); // right sensor

  int straight1 = analogRead(IR_SENSOR_3); // Straight sensor
  int straight2 = analogRead(IR_SENSOR_6); // Straight sensor

  // Convert the values to 0 and 1.
  if (left1 > 4000 || left2 > 4000)
  {
    (*left_wall) = 1;
  }

  if (right1 > 4000 || right2 > 4000)
  {
    (*right_wall) = 1;
  }

  if (straight1 > 4000 || straight2 > 4000)
  {
    (*straight_wall) = 1;
  }
}

void forward_movement(int speedA, int speedB)
{
  // Check if speedA is negative (indicating reverse direction)
  if (speedA < 0)
  {
    // Convert speedA to positive for PWM control
    speedA = 0 - speedA;
    // Set motor phase to LOW for reverse direction on the left motor
    digitalWrite(MOTOR_LEFT_A_PHASE_PIN, LOW);
  }
  else
  {
    // Set motor phase to HIGH for forward direction on the left motor
    digitalWrite(MOTOR_LEFT_A_PHASE_PIN, HIGH);
  }

  // Check if speedB is negative (indicating reverse direction)
  if (speedB < 0)
  {
    // Convert speedB to positive for PWM control
    speedB = 0 - speedB;
    // Set motor phase to HIGH for forward direction on the right motor
    digitalWrite(MOTOR_RIGHT_B_PHASE_PIN, HIGH);
  }
  else
  {
    // Set motor phase to LOW for reverse direction on the right motor
    digitalWrite(MOTOR_RIGHT_B_PHASE_PIN, LOW);
  }

  // Apply PWM signal to the left motor to control its speed
  analogWrite(MOTOR_LEFT_A_ENABLE_PIN, speedA);
  // Apply PWM signal to the right motor to control its speed
  analogWrite(MOTOR_RIGHT_B_ENABLE_PIN, speedB);
}
