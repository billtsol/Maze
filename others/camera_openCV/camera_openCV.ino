/*
  Ο κώδικας συνδέει ένα ESP32-CAM με WiFi και ρυθμίζει έναν web server 
  που εξυπηρετεί live βίντεο και εικόνες από την κάμερα
  σε τρεις διαφορετικές αναλύσεις: χαμηλή, μεσαία και υψηλή.

*/
#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

// Στοιχεία σύνδεσης στο WiFi
const char* WIFI_SSID = "Vodafone-C43726133";   // Το όνομα του δικτύου WiFi (SSID)
const char* WIFI_PASS = "CYC9fKp9x9kH44Kt";     // Ο κωδικός πρόσβασης του WiFi

// Δημιουργία web server στην πόρτα 80
WebServer server(80);

// Ορισμός των αναλύσεων της κάμερας
static auto loRes = esp32cam::Resolution::find(320, 240);  // Χαμηλή ανάλυση
static auto midRes = esp32cam::Resolution::find(350, 530); // Μεσαία ανάλυση
static auto hiRes = esp32cam::Resolution::find(800, 600);  // Υψηλή ανάλυση

// Συνάρτηση για αποστολή JPEG εικόνας
void serveJpg() {
  // Λήψη εικόνας από την κάμερα
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", ""); // Send 503 error if capture failed
    return;
  }

  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  // Ορισμός μήκους περιεχομένου της απόκρισης HTTP
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg"); // Στείλε HTTP 200 OK με τύπο περιεχομένου "image/jpeg"

  // Πάρε τον client και γράψε τα δεδομένα του καρέ στον client
  WiFiClient client = server.client();
  frame->writeTo(client);
}

// Συνάρτηση διαχείρισης για εικόνα χαμηλής ανάλυσης
void handleJpgLo() {
  // Αλλαγή της ανάλυσης της κάμερας σε χαμηλή
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg(); // Αποστολή της εικόνας
}

// Συνάρτηση διαχείρισης για εικόνα υψηλής ανάλυσης
void handleJpgHi() {
  // Αλλαγή της ανάλυσης της κάμερας σε υψηλή
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg(); // Αποστολή της εικόνας
}

// Συνάρτηση διαχείρισης για εικόνα μεσαίας ανάλυσης
void handleJpgMid() {
  // Αλλαγή της ανάλυσης της κάμερας σε μεσαία
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg(); // Αποστολή της εικόνας
}

void setup() {
  Serial.begin(115200); // Αρχικοποίηση σειριακής επικοινωνίας στα 115200 baud
  Serial.println();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);  // Ορισμός pins του μοντέλου κάμερας (π.χ. Ai-Thinker)
    cfg.setResolution(hiRes);      // Αρχική ανάλυση σε υψηλή
    cfg.setBufferCount(2);         // Ορισμός του αριθμού buffer σε 2
    cfg.setJpeg(80);               // Ποιότητα JPEG (80%)

    // Αρχικοποίηση της κάμερας με την καθορισμένη διαμόρφωση
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL"); // Check if camera initialization was successful
  }

  // Αρχικοποίηση της σύνδεσης WiFi
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA); // Ορισμός λειτουργίας WiFi σε σταθμό (σύνδεση σε υπάρχον δίκτυο)
  WiFi.begin(WIFI_SSID, WIFI_PASS); // Έναρξη σύνδεσης WiFi
  while (WiFi.status() != WL_CONNECTED) { // Αναμονή μέχρι να συνδεθεί
    delay(500);
  }

  // Εμφάνιση της διεύθυνσης IP και των διαθέσιμων διαδρομών στον σειριακό παρακολούθηση
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");  // Διαδρομή για εικόνα χαμηλής ανάλυσης
  Serial.println("  /cam-hi.jpg");  // Διαδρομή για εικόνα υψηλής ανάλυσης
  Serial.println("  /cam-mid.jpg"); // Διαδρομή για εικόνα μεσαίας ανάλυσης

  // Ορισμός διαδρομών για τις διαφορετικές αναλύσεις
  server.on("/cam-lo.jpg", handleJpgLo);  // Διαδρομή χαμηλής ανάλυσης
  server.on("/cam-hi.jpg", handleJpgHi);  // Διαδρομή υψηλής ανάλυσης
  server.on("/cam-mid.jpg", handleJpgMid); // Διαδρομή μεσαίας ανάλυσης

  server.begin(); // Εκκίνηση του web server
}

void loop() {
  server.handleClient(); // Διαχείριση εισερχόμενων αιτημάτων πελατών
}
