

/* ********** Include the necessary libraries ********** */
#include <Adafruit_NeoPixel.h> // RGB Led library
#include <WiFi.h>              // WiFi library
#include <Wire.h>              // Gyroscope library
#include <HTTPClient.h>        // Http client library
#include <ArduinoJson.h>       // JSON library

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

const char *serverName = "http://192.168.1.4:3000/api/maze"; // Define the server ip address
const char *ssid = "Vodafone-C43726133";                     // Define the SSID for the WiFi
const char *password = "CYC9fKp9x9kH44Kt";                   // Define the password for the WiFi
WiFiServer server(80);                                       // Define the server for the WiFi

const int MPU_ADDR = 0x68;                 // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Define the variables for the gyroscope

const int base_speed = 100; // Define the base speed for the motors

/* */
const int mazeWidth = 20;
const int mazeHeight = 20;
const int startX = 0;
const int startY = 0;
const int goalX = 19;
const int goalY = 19;

// Δομή κόμβου του A*
struct Node
{
  int x, y;     // Θέση στο λαβύρινθο
  int g, h, f;  // Κόστος διαδρομής, εκτιμώμενη απόσταση και συνολικό κόστος
  Node *parent; // Δείκτης στον προηγούμενο κόμβο
};

// Μέγεθος πίνακα λαβύρινθου 20x20
int mazeMap[mazeWidth][mazeHeight];

// Αρχική συντεταγμένη του ρομπότ
int startX = 0;
int startY = 0;

// Δισδιάστατος πίνακας για τους κόμβους
Node nodes[mazeWidth][mazeHeight];

bool run = true;

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

  initializeMazeMap();
}

/* ********** Set up loop function ********** */
void loop()
{

  if (run)
  {
    // Node* goalNode = aStar();

    // if (goalNode != nullptr) {
    //   Serial.println("Found path:");
    //   followPath(goalNode);
    // } else {
    //   Serial.println("No path found.");
    // }

    // Ρύθμιση διαδρομής για αποστολή του maze σε JSON
    sentDataToServer();
    run = false;
  }
}

/* ********** Set up external functions ********** */
void sentDataToServer() {
  // Check Wi-Fi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    // Καθορισμός προορισμού του αιτήματος
    http.begin(serverName);

    // Ρύθμιση κεφαλίδας HTTP για JSON
    http.addHeader("Content-Type", "application/json");

    // Δημιουργία JSON payload για τον πίνακα maze
    StaticJsonDocument<512> doc;
    JsonArray mazeData = doc.createNestedArray("maze");

    for (int i = 0; i < mazeWidth; i++)
    {
      JsonArray row = mazeData.createNestedArray();
      for (int j = 0; j < mazeHeight; j++)
      {
        row.add(maze[i][j]);
      }
    }

    // Μετατροπή του JSON σε String για αποστολή
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Αποστολή του HTTP POST αιτήματος με το JSON payload
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

// Αρχικοποίηση πίνακα με άγνωστα εμπόδια
void initializeMazeMap() {
  for (int i = 0; i < mazeWidth; i++) {
    for (int j = 0; j < mazeHeight; j++) {
      mazeMap[i][j] = 1; // Θεωρούμε αρχικά ότι όλα τα σημεία είναι εμπόδια
    }
  }
  mazeMap[startY][startX] = 0; // Ξεκινάει από το σημείο του ρομπότ
}

void exploreMaze() {
  // Αρχικές συντεταγμένες του ρομπότ
  int x = startX;
  int y = startY;

  // Προσδιορισμός τοίχων
  int left_wall = 0, right_wall = 0, front_wall = 0;

  // Επαναλαμβανόμενη κίνηση για εξερεύνηση
  while (true) {
    // Ανάγνωση αισθητήρων
    readIRSensorsValues(&left_wall, &right_wall, &front_wall);

    // Ενημέρωση πίνακα με βάση την ανίχνευση εμποδίων
    if (front_wall) {
      mazeMap[y][x] = 1; // Καταγράφουμε το εμπόδιο μπροστά

      // Αν υπάρχει τοίχος μπροστά, ελέγχουμε πλάγια
      if (!left_wall) {
        turnLeft();
        moveForward();
        x--; // Μετακίνηση αριστερά στον πίνακα
      } else if (!right_wall) {
        turnRight();
        moveForward();
        x++; // Μετακίνηση δεξιά στον πίνακα
      } else {
        // Αν υπάρχουν τοίχοι και στις τρεις κατευθύνσεις, το ρομπότ σταματά
        break;
      }
    } else {
      // Αν δεν υπάρχει εμπόδιο μπροστά, προχωράμε ευθεία
      moveForward();
      y++; // Προχώρημα μπροστά στον πίνακα
      mazeMap[y][x] = 0; // Καταγράφουμε το σημείο ως προσπελάσιμο
    }

    // Έλεγχος συνόρων και διακοπή αν το ρομπότ πλησιάσει τα άκρα του πίνακα
    if (x < 0 || x >= mazeWidth || y < 0 || y >= mazeHeight) {
      break;
    }
  }
}

void readIRSensorsValues(int *left_wall, int *right_wall, int *front_wall) {
  // Read the values from he IR sensors
  int left1 = analogRead(IR_SENSOR_1); // left sensor
  int left2 = analogRead(IR_SENSOR_2); // left sensor

  int right1 = analogRead(IR_SENSOR_4); // right sensor
  int right2 = analogRead(IR_SENSOR_5); // right sensor

  int front1 = analogRead(IR_SENSOR_3); // Straight sensor
  int front2 = analogRead(IR_SENSOR_6); // Straight sensor

  // Convert the values to 0 and 1.
  if (left1 > 4000 || left2 > 4000)
  {
    (*left_wall) = 1;
  }

  if (right1 > 4000 || right2 > 4000)
  {
    (*right_wall) = 1;
  }

  if (front1 > 4000 || front2 > 4000)
  {
    (*front_wall) = 1;
  }
}

// Δημιουργία διαδρομής με χρήση του A*
Node *aStar() {
  for (int x = 0; x < mazeWidth; x++)
  {
    for (int y = 0; y < mazeHeight; y++)
    {
      nodes[x][y].x = x;
      nodes[x][y].y = y;
      nodes[x][y].g = INT_MAX;
      nodes[x][y].h = heuristic(x, y, goalX, goalY);
      nodes[x][y].f = INT_MAX;
      nodes[x][y].parent = nullptr;
    }
  }

  Node *openList[mazeWidth * mazeHeight];
  int openListSize = 0;

  Node *startNode = &nodes[startX][startY];
  startNode->g = 0;
  startNode->f = startNode->h;
  openList[openListSize++] = startNode;

  while (openListSize > 0)
  {
    Node *current = openList[0];
    int currentIndex = 0;
    for (int i = 1; i < openListSize; i++)
    {
      if (openList[i]->f < current->f)
      {
        current = openList[i];
        currentIndex = i;
      }
    }

    openList[currentIndex] = openList[--openListSize];

    if (current->x == goalX && current->y == goalY)
    {
      return current;
    }

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
      int nx = current->x + dx[i];
      int ny = current->y + dy[i];

      if (nx < 0 || nx >= mazeWidth || ny < 0 || ny >= mazeHeight || maze[nx][ny] == 1)
      {
        continue;
      }

      Node *neighbor = &nodes[nx][ny];
      int tentative_g = current->g + 1;

      if (tentative_g < neighbor->g)
      {
        neighbor->parent = current;
        neighbor->g = tentative_g;
        neighbor->f = neighbor->g + neighbor->h;

        bool inOpenList = false;
        for (int j = 0; j < openListSize; j++)
        {
          if (openList[j] == neighbor)
          {
            inOpenList = true;
            break;
          }
        }
        if (!inOpenList)
        {
          openList[openListSize++] = neighbor;
        }
      }
    }
  }
  return nullptr;
}

// Καθοδήγηση του ρομπότ στη διαδρομή που βρέθηκε
void followPath(Node *goalNode) {
  Node *currentNode = goalNode;

  // Αναστρέφουμε τη διαδρομή
  Node *path[mazeWidth * mazeHeight];
  int pathLength = 0;

  while (currentNode)
  {
    path[pathLength++] = currentNode;
    currentNode = currentNode->parent;
  }

  for (int i = pathLength - 1; i > 0; i--)
  {
    Node *from = path[i];
    Node *to = path[i - 1];

    if (to->x == from->x + 1)
    {
      Serial.println("Move Down");
    }
    else if (to->x == from->x - 1)
    {
      Serial.println("Move Up");
    }
    else if (to->y == from->y + 1)
    {
      Serial.println("Move Right");
    }
    else if (to->y == from->y - 1)
    {
      Serial.println("Move Left");
    }
  }
}

// Movement functions
void moveForward() {
  forward_movement(150, 150); 
  delay(1500); // Προχωρα 10cm
}

void turnLeft() {
  forward_movement(-150, 150); // Adjust for left turn
  delay(2500);                  // Delay for turning duration
}

void turnRight() {
  forward_movement(150, -150); // Adjust for right turn
  delay(2500);                  // Delay for turning duration
}

void forward_movement(int speedA, int speedB) {
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

// Υπολογισμός απόστασης Manhattan
int heuristic(int x, int y, int goalX, int goalY) {
  return abs(x - goalX) + abs(y - goalY);
}

void turnOnRGBLeds(int red, int blue, int green) {
  digitalWrite(EXTERNAL_RGB_LED_RED_PIN, red);
  digitalWrite(EXTERNAL_RGB_LED_GREEN_PIN, green);
  digitalWrite(EXTERNAL_RGB_LED_BLUE_PIN, blue);
}

// Function to get distance in centimeters
long getDistanceCM() {
    // Send a 10-microsecond pulse to the trigger pin
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the time taken for the echo pulse to be received
    long duration = pulseIn(ECHO_PIN, HIGH);

    // Calculate distance in cm (speed of sound = 34300 cm/s, round trip)
    long distanceCM = duration * 0.034 / 2;

    return distanceCM;
}
