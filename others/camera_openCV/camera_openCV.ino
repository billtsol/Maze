#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

// WiFi credentials
const char* WIFI_SSID = "VODAFONE_0152";   // Your WiFi SSID
const char* WIFI_PASS = "2egpa5u95tfh73b8"; // Your WiFi password

// Create a web server on port 80
WebServer server(80);

// Set camera resolutions
static auto loRes = esp32cam::Resolution::find(320, 240);  // Low resolution
static auto midRes = esp32cam::Resolution::find(350, 530); // Medium resolution
static auto hiRes = esp32cam::Resolution::find(800, 600);  // High resolution

// Function to serve a JPEG image
void serveJpg() {
  // Capture a frame from the camera
  auto frame = esp32cam::capture();
  if (frame == nullptr) { // Check if capture was successful
    Serial.println("CAPTURE FAIL");
    server.send(503, "", ""); // Send 503 error if capture failed
    return;
  }

  // Log capture success and frame details
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  // Set the content length of the HTTP response
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg"); // Send HTTP 200 OK with content type "image/jpeg"

  // Get the client and write the frame data to the client
  WiFiClient client = server.client();
  frame->writeTo(client);
}

// Handler function for low-resolution image
void handleJpgLo() {
  // Change the camera resolution to low
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg(); // Serve the image
}

// Handler function for high-resolution image
void handleJpgHi() {
  // Change the camera resolution to high
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg(); // Serve the image
}

// Handler function for medium-resolution image
void handleJpgMid() {
  // Change the camera resolution to medium
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg(); // Serve the image
}

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  Serial.println();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);  // Set camera module pins (e.g., Ai-Thinker module)
    cfg.setResolution(hiRes);      // Set initial resolution to high
    cfg.setBufferCount(2);         // Set buffer count to 2
    cfg.setJpeg(80);               // Set JPEG quality (80%)

    // Initialize the camera with the specified configuration
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL"); // Check if camera initialization was successful
  }

  // Initialize WiFi connection
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA); // Set WiFi mode to station (connect to an existing network)
  WiFi.begin(WIFI_SSID, WIFI_PASS); // Start WiFi connection
  while (WiFi.status() != WL_CONNECTED) { // Wait until connected
    delay(500);
  }

  // Print the IP address and available endpoints to the serial monitor
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");  // Low resolution image endpoint
  Serial.println("  /cam-hi.jpg");  // High resolution image endpoint
  Serial.println("  /cam-mid.jpg"); // Medium resolution image endpoint

  // Define routes for different resolutions
  server.on("/cam-lo.jpg", handleJpgLo);  // Low resolution route
  server.on("/cam-hi.jpg", handleJpgHi);  // High resolution route
  server.on("/cam-mid.jpg", handleJpgMid); // Medium resolution route

  server.begin(); // Start the web server
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}
