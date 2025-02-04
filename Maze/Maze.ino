/* Maze.ino
  Ο κώδικας αυτός έχει ως στόχο τον έλεγχο ενός ρομπότ το οποίο εξερευνά έναν λαβύρινθο. 
  Το λογισμικό ενσωματώνει αλγόριθμο A* για να βρει την πιο σύντομη διαδρομή προς τον στόχο,
  ενώ καταγράφει και στέλνει δεδομένα του λαβυρίνθου μέσω WiFi σε έναν server.
*/

/* ********** Συμπερίληψη των απαραίτητων βιβλιοθηκών ********** */
#include <Adafruit_NeoPixel.h> // Βιβλιοθήκη για τον έλεγχο των RGB LED
#include <WiFi.h>              // Βιβλιοθήκη για σύνδεση με Wi-Fi
#include <Wire.h>              // Βιβλιοθήκη για τον γυροσκόπιο
#include <HTTPClient.h>        // Βιβλιοθήκη για αποστολή HTTP αιτημάτων
#include <ArduinoJson.h>       // Βιβλιοθήκη για χειρισμό JSON δεδομένων

/* ********** Συμπερίληψη των απαραίτητων pins (άξονες σύνδεσης) ********** */
#define BUILDIN_RGB_LED_PIN 48 // Ορισμός του pin για το ενσωματωμένο RGB LED

#define TRIG_PIN 19 // Ορισμός του trigger pin για τον αισθητήρα απόστασης
#define ECHO_PIN 20 // Ορισμός του echo pin για τον αισθητήρα απόστασης

#define RECV_PIN 1 // Ορισμός του pin λήψης για τον αισθητήρα απόστασης

#define EXTERNAL_RGB_LED_RED_PIN 21   // Κόκκινο pin για το εξωτερικό RGB LED
#define EXTERNAL_RGB_LED_GREEN_PIN 47 // Πράσινο pin για το εξωτερικό RGB LED
#define EXTERNAL_RGB_LED_BLUE_PIN 48  // Μπλε pin για το εξωτερικό RGB LED

#define MOTOR_LEFT_A_PHASE_PIN 4  // Ορισμός του pin για την ταχύτητα του αριστερού κινητήρα
#define MOTOR_LEFT_A_ENABLE_PIN 5 // Ορισμός του pin ενεργοποίησης του αριστερού κινητήρα

#define MOTOR_RIGHT_B_PHASE_PIN 6  // Ορισμός του pin για την ταχύτητα του δεξιού κινητήρα
#define MOTOR_RIGHT_B_ENABLE_PIN 7 // Ορισμός του pin ενεργοποίησης του δεξιού κινητήρα

// Ορισμός των pins για τους αισθητήρες υπερύθρων (IR)
#define IR_SENSOR_1 3  // Αριστερός αισθητήρας
#define IR_SENSOR_2 8  // Αριστερός αισθητήρας
#define IR_SENSOR_3 15 // Κεντρικός αισθητήρας
#define IR_SENSOR_4 16 // Δεξιός αισθητήρας
#define IR_SENSOR_5 17 // Δεξιός αισθητήρας
#define IR_SENSOR_6 18 // Κεντρικός αισθητήρας

/* ********** Συμπερίληψη των απαραίτητων μεταβλητών ********** */
Adafruit_NeoPixel pixels(1, BUILDIN_RGB_LED_PIN, NEO_GRB + NEO_KHZ800); // Ορισμός του ενσωματωμένου RGB LED

const char *serverName = "http://192.168.1.4:3000/api/maze"; // Διεύθυνση IP του server
const char *ssid = "Vodafone-C43726133";                     // SSID για σύνδεση WiFi
const char *password = "CYC9fKp9x9kH44Kt";                   // Κωδικός για τη σύνδεση WiFi
WiFiServer server(80);                                       // Ορισμός του server για την σύνδεση WiFi

const int MPU_ADDR = 0x68;                 // Διεύθυνση I2C του αισθητήρα MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Ορισμός των μεταβλητών για το γυροσκόπιο

// Αρχικός προσανατολισμός
enum Direction { NORTH, EAST, SOUTH, WEST };
Direction currentDirection = NORTH;

const int base_speed = 100; // Ορισμός της βασικής ταχύτητας για τους κινητήρες

/* ********** Ορισμός του λαβυρίνθου και των σημείων εκκίνησης και τερματισμού ********** */
const int mazeWidth = 20;   // Πλάτος του λαβυρίνθου
const int mazeHeight = 20;  // Ύψος του λαβυρίνθου
const int startX = 0;       // Συντεταγμένη Χ για την εκκίνηση
const int startY = 0;       // Συντεταγμένη Υ για την εκκίνηση
const int goalX = 19;       // Συντεταγμένη Χ για τον στόχο
const int goalY = 19;       // Συντεταγμένη Υ για τον στόχο

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
void loop() {
  // Βήμα 1: Εξερεύνηση του λαβύρινθου
  /* Καλούμε τη συνάρτηση για να ξεκινήσει την εξερεύνηση του λαβύρινθου. 
      Το ρομπότ χαρτογραφεί τον χώρο καθώς κινείται */
  exploreMaze();

  // Βήμα 2: Αποστολή δεδομένων στον server
  sentDataToServer();

  // Βήμα 3: Ενεργοποίηση των RGB LEDs σε πράσινο χρώμα
  /* Ενεργοποιεί τα RGB LEDs σε πράσινο χρώμα 
      ως ένδειξη επιτυχούς συλλογής δεδομένων και αποστολής
  */
  turnOnRGBLeds(0, 255, 0);  // Πράσινο χρώμα

  // Βήμα 4: Αναμονή για 15 δευτερόλεπτα
  delay(15000);

  // Βήμα 5: Λύση του λαβύρινθου με χρήση A*
  Node *goalNode = aStar();
  followPath(goalNode);
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

// Συνάρτηση για την εξερεύνηση του λαβύρινθου
void exploreMaze() {
  int x = startX;
  int y = startY;

  int left_wall = 0, right_wall = 0, front_wall = 0;

  while (true) {
    // Ανάγνωση αισθητήρων
    readIRSensorsValues(&left_wall, &right_wall, &front_wall);

    // Έλεγχος για εμπόδιο μπροστά
    if (front_wall) {
      mazeMap[y][x] = 1; // Καταγράφουμε το εμπόδιο μπροστά

      // Αλλαγή κατεύθυνσης με βάση τους τοίχους και προσανατολισμό
      if (!left_wall) {
        turnLeft();
        updateDirection(-90);
        moveForward();
        if (currentDirection == NORTH) y--;
        else if (currentDirection == EAST) x++;
        else if (currentDirection == SOUTH) y++;
        else if (currentDirection == WEST) x--;
      } else if (!right_wall) {
        turnRight();
        updateDirection(90);
        moveForward();
        if (currentDirection == NORTH) y--;
        else if (currentDirection == EAST) x++;
        else if (currentDirection == SOUTH) y++;
        else if (currentDirection == WEST) x--;
      } else {
        // Αν υπάρχουν τοίχοι σε όλες τις κατευθύνσεις, σταματάμε
        break;
      }
    } else {
      // Αν δεν υπάρχει εμπόδιο μπροστά, προχωράμε ευθεία
      moveForward();
      if (currentDirection == NORTH) y--;
      else if (currentDirection == EAST) x++;
      else if (currentDirection == SOUTH) y++;
      else if (currentDirection == WEST) x--;

      mazeMap[y][x] = 0; // Καταγράφουμε το σημείο ως προσπελάσιμο
    }

    // Έλεγχος αν το ρομπότ πλησιάζει στα όρια του λαβύρινθου
    if (x < 0 || x >= mazeWidth || y < 0 || y >= mazeHeight) {
      break;
    }
  }
}

void readIRSensorsValues(int *left_wall, int *right_wall, int *front_wall) {
  int left1 = analogRead(IR_SENSOR_1);
  int left2 = analogRead(IR_SENSOR_2);

  int right1 = analogRead(IR_SENSOR_4);
  int right2 = analogRead(IR_SENSOR_5);

  int front1 = analogRead(IR_SENSOR_3);
  int front2 = analogRead(IR_SENSOR_6);

  if (left1 > 4000 || left2 > 4000){
    (*left_wall) = 1;
  }

  if (right1 > 4000 || right2 > 4000){
    (*right_wall) = 1;
  }

  if (front1 > 4000 || front2 > 4000){
    (*front_wall) = 1;
  }
}

// Δημιουργία διαδρομής με χρήση του A*
Node *aStar() {
  // Αρχικοποίηση όλων των κόμβων του λαβύρινθου
  for (int x = 0; x < mazeWidth; x++){
    for (int y = 0; y < mazeHeight; y++) {
      // Ορισμός των συντεταγμένων και της αρχικής τιμής των παραμέτρων των κόμβων
      nodes[x][y].x = x;
      nodes[x][y].y = y;
      nodes[x][y].g = INT_MAX; // Αρχική τιμή του κόστους από την αρχή (μη δυνατό κόστος)
      nodes[x][y].h = heuristic(x, y, goalX, goalY); // Υπολογισμός της εκτιμώμενης απόστασης προς τον στόχο
      nodes[x][y].f = INT_MAX; // Αρχική τιμή του συνολικού κόστους (f = g + h)
      nodes[x][y].parent = nullptr; // Δεν υπάρχει γονικός κόμβος στην αρχή
    }
  }

  // Δημιουργία λίστας για τους ανοιχτούς κόμβους
  Node *openList[mazeWidth * mazeHeight];
  int openListSize = 0;

  // Ορισμός του κόμβου εκκίνησης
  Node *startNode = &nodes[startX][startY];
  startNode->g = 0;  // Το κόστος από την αρχή είναι 0
  startNode->f = startNode->h; // Το συνολικό κόστος είναι μόνο το εκτιμώμενο κόστος (h)
  openList[openListSize++] = startNode; // Προσθήκη του κόμβου εκκίνησης στη λίστα ανοιχτών κόμβων

  // Εκτέλεση του αλγορίθμου A*
  while (openListSize > 0){
    // Επιλογή του κόμβου με το μικρότερο συνολικό κόστος (f) από τη λίστα ανοιχτών κόμβων
    Node *current = openList[0];
    int currentIndex = 0;
    for (int i = 1; i < openListSize; i++){
      if (openList[i]->f < current->f){
        current = openList[i];
        currentIndex = i;
      }
    }

    // Αφαίρεση του επιλεγμένου κόμβου από τη λίστα ανοιχτών
    openList[currentIndex] = openList[--openListSize];

    // Έλεγχος αν φτάσαμε στον στόχο
    if (current->x == goalX && current->y == goalY){
      return current; // Επιστροφή του τελικού κόμβου (στοχος)
    }

    // Πίνακες για την κίνηση στους τέσσερις βασικούς προσανατολισμούς (δεξιά, αριστερά, πάνω, κάτω)
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    // Εξέταση των γειτονικών κόμβων (προς όλες τις κατευθύνσεις)
    for (int i = 0; i < 4; i++){
      int nx = current->x + dx[i]; // Νέα θέση για τον άξονα x
      int ny = current->y + dy[i]; // Νέα θέση για τον άξονα y

      // Έλεγχος αν ο γειτονικός κόμβος είναι εκτός του λαβυρίνθου ή είναι τοίχος (maze[nx][ny] == 1)
      if (nx < 0 || nx >= mazeWidth || ny < 0 || ny >= mazeHeight || maze[nx][ny] == 1){
        continue; // Παράλειψη αυτών των κόμβων
      }

      // Ανάθεση του γειτονικού κόμβου
      Node *neighbor = &nodes[nx][ny];
      int tentative_g = current->g + 1; // Υπολογισμός του προσωρινού κόστους για τον γειτονικό κόμβο

      // Αν το προσωρινό κόστος είναι μικρότερο από το τρέχον, ενημερώνουμε τον κόμβο
      if (tentative_g < neighbor->g){
        neighbor->parent = current; // Ορισμός του γονικού κόμβου
        neighbor->g = tentative_g; // Ενημέρωση του κόστους από την αρχή
        neighbor->f = neighbor->g + neighbor->h; // Ενημέρωση του συνολικού κόστους (f)

        // Έλεγχος αν ο γειτονικός κόμβος υπάρχει ήδη στη λίστα ανοιχτών
        bool inOpenList = false;
        for (int j = 0; j < openListSize; j++){
          if (openList[j] == neighbor){
            inOpenList = true; // Ο γειτονικός κόμβος είναι ήδη στη λίστα ανοιχτών
            break;
          }
        }

        // Αν δεν υπάρχει ήδη, προσθέτουμε τον γειτονικό κόμβο στη λίστα ανοιχτών
        if (!inOpenList){
          openList[openListSize++] = neighbor;
        }
      }
    }
  }
  
  // Αν δεν βρεθεί λύση (στόχος δεν είναι προσβάσιμος), επιστρέφουμε nullptr
  return nullptr;
}

// Καθοδήγηση του ρομπότ στη διαδρομή που βρέθηκε
void followPath(Node *goalNode) {
  Node *currentNode = goalNode;

  // Αναστρέφουμε τη διαδρομή
  Node *path[mazeWidth * mazeHeight];
  int pathLength = 0;

  while (currentNode) {
    path[pathLength++] = currentNode;
    currentNode = currentNode->parent;
  }

  /* 
  Η turnLeft() περιστρέφει το ρομπότ αριστερά κατά 90 μοίρες. 
  Αν χρειάζεται περιστροφή δεξιά, γίνεται με διαδοχικές αριστερές στροφές
  μέχρι να φτάσει στην επιθυμητή κατεύθυνση.
  */

  // Αρχική κατεύθυνση του ρομπότ (0 = Up, 1 = Right, 2 = Down, 3 = Left)
  int currentDirection = 0; 

  for (int i = pathLength - 1; i > 0; i--) {
    Node *from = path[i];
    Node *to = path[i - 1];

    // Υπολογίζουμε την επόμενη κίνηση
    if (to->x == from->x + 1) { // Move Down
      while (currentDirection != 2) {
        turnLeft();
        currentDirection = (currentDirection + 3) % 4;
      }
      moveForward();
    } 
    else if (to->x == from->x - 1) { // Move Up
      while (currentDirection != 0) {
        turnLeft();
        currentDirection = (currentDirection + 3) % 4;
      }
      moveForward();
    } 
    else if (to->y == from->y + 1) { // Move Right
      while (currentDirection != 1) {
        turnLeft();
        currentDirection = (currentDirection + 3) % 4;
      }
      moveForward();
    } 
    else if (to->y == from->y - 1) { // Move Left
      while (currentDirection != 3) {
        turnLeft();
        currentDirection = (currentDirection + 3) % 4;
      }
      moveForward();
    }
  }
}

// Movement functions
void moveForward() {
  forward_movement(100, 100); // Slow speed for both motors 
  delay(1500); // Προχωράει 10cm
}

// Συνάρτηση για στροφή αριστερά
void turnLeft() {
  forward_movement(-150, 150); // Ρύθμιση για στροφή αριστερά (αρνητική ταχύτητα για αριστερό τροχό)
  delay(2500);                  // Καθυστέρηση για την διάρκεια της στροφής
}

// Συνάρτηση για στροφή δεξιά
void turnRight() {
  forward_movement(150, -150); // Ρύθμιση για στροφή δεξιά (αρνητική ταχύτητα για δεξιό τροχό)
  delay(2500);                  // Καθυστέρηση για την διάρκεια της στροφής
}

// Συνάρτηση για κίνηση προς τα εμπρός με καθορισμένη ταχύτητα στους δύο κινητήρες
void forward_movement(int speedA, int speedB) {
  // Έλεγχος αν η ταχύτητα του αριστερού κινητήρα είναι αρνητική (υποδεικνύει κίνηση προς τα πίσω)
  if (speedA < 0){
    speedA = 0 - speedA;
    // Ορισμός φάσης του κινητήρα αριστερά σε LOW για κίνηση προς τα πίσω
    digitalWrite(MOTOR_LEFT_A_PHASE_PIN, LOW);
  }
  else{
    // Ορισμός φάσης του κινητήρα αριστερά σε HIGH για κίνηση προς τα εμπρός
    digitalWrite(MOTOR_LEFT_A_PHASE_PIN, HIGH);
  }

  // Έλεγχος αν η ταχύτητα του δεξιού κινητήρα είναι αρνητική (υποδεικνύει κίνηση προς τα πίσω)
  if (speedB < 0){
    speedB = 0 - speedB;
    // Ορισμός φάσης του κινητήρα δεξιά σε HIGH για κίνηση προς τα εμπρός
    digitalWrite(MOTOR_RIGHT_B_PHASE_PIN, HIGH);
  }
  else{
    // Ορισμός φάσης του κινητήρα δεξιά σε LOW για κίνηση προς τα πίσω
    digitalWrite(MOTOR_RIGHT_B_PHASE_PIN, LOW);
  }

  // Εφαρμογή σήματος PWM στον αριστερό κινητήρα για τον έλεγχο της ταχύτητας
  analogWrite(MOTOR_LEFT_A_ENABLE_PIN, speedA);
  // Εφαρμογή σήματος PWM στον δεξιό κινητήρα για τον έλεγχο της ταχύτητας
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

// Συνάρτηση για την απόκτηση απόστασης σε εκατοστά
long getDistanceCM() {
  // Αποστολή παλμού διάρκειας 10 μικροδευτερολέπτων στην ακίδα ενεργοποίησης (trigger)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);      // Εγγύηση χαμηλού σήματος για 2 μικροδευτερόλεπτα
  digitalWrite(TRIG_PIN, HIGH); // Αποστολή παλμού υψηλού σήματος
  delayMicroseconds(10);     // Παλμός υψηλού σήματος για 10 μικροδευτερόλεπτα
  digitalWrite(TRIG_PIN, LOW);  // Επαναφορά σε χαμηλό σήμα

  // Μέτρηση χρόνου επιστροφής του παλμού ηχούς (echo)
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Υπολογισμός απόστασης σε cm (ταχύτητα ήχου = 34300 cm/s, διπλή διαδρομή)
  long distanceCM = duration * 0.034 / 2;

  return distanceCM; // Επιστροφή της απόστασης σε εκατοστά
}

// Συνάρτηση για την ανάγνωση των τιμών του γυροσκοπίου
void readGyroscope() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43); // Το καταχωρητής του γυροσκοπίου
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

// Συνάρτηση για να ενημερώνουμε τον προσανατολισμό
void updateDirection(int turn) {
  // Ανάλογα με τη στροφή, αλλάζουμε προσανατολισμό
  if (turn == -90) { // Αριστερή στροφή
    currentDirection = (Direction)((currentDirection + 3) % 4);
  } else if (turn == 90) { // Δεξιά στροφή
    currentDirection = (Direction)((currentDirection + 1) % 4);
  } else if (turn == 180) { // Στροφή 180 μοιρών
    currentDirection = (Direction)((currentDirection + 2) % 4);
  }
}
