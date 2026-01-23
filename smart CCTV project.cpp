#include <WiFi.h>
#include <HTTPClient.h>

// --- HARDWARE PIN DEFINITIONS ---
#define ECHO_PIN 19
#define TRIG_PIN 18
#define LED_RED 21

// --- WIFI SETTINGS ---
const char* WIFI_SSID = "B40Tegar";
const char* WIFI_PASS = "MiskinSejati";

// --- MOBIUS SETTINGS ---
const char* mobiusServer = "http://192.168.0.21:7579/Mobius/Smartsurveillance";

// --- SENSOR TUNING ---
const unsigned long intervalMs = 200;      // check interval
const unsigned long echoTimeoutUs = 30000;
const float dangerOnCm  = 10.0;            // trigger threshold
const float dangerOffCm = 12.0;            // hysteresis

// --- AMBULANCE LED EFFECT ---
const unsigned long ambulanceDurationMs = 10000; // 10 seconds
const unsigned long flashStepMs = 90;            // LED flash speed

// --- STATE ---
bool tooClose = false;
bool ambulanceActive = false;
unsigned long ambulanceStartMs = 0;
unsigned long lastFlashMs = 0;
bool ledState = false;

// --- FUNCTION: CONNECT WIFI ---
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setSleep(false);

  Serial.print("Connecting to WiFi");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("❌ WiFi Failed. Check Password or Hotspot.");
  }
}

// --- FUNCTION: SEND DATA TO MOBIUS ---
void sendToMobius(String container, String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(mobiusServer) + "/" + container;

    http.begin(url);
    http.addHeader("X-M2M-RI", String(millis())); // unique request ID
    http.addHeader("X-M2M-Origin", "S");
    http.addHeader("Content-Type", "application/json;ty=4");
    http.addHeader("Connection", "close");

    String requestBody = "{\"m2m:cin\": {\"con\": \"" + data + "\"}}";
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      Serial.print("📤 Mobius [" + container + "]: " + data + " -> ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("❌ Mobius Error: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

// --- FUNCTION: READ ULTRASONIC ---
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, echoTimeoutUs);
  if (duration == 0) return -1;
  return (duration * 0.034f) / 2.0f;
}

// --- START AMBULANCE ---
void startAmbulance() {
  ambulanceActive = true;
  ambulanceStartMs = millis();
  lastFlashMs = 0;
  ledState = false;

  Serial.println("🚨 Ambulance LED effect START (10s)");
  sendToMobius("alarm", "1");
}

// --- UPDATE AMBULANCE ---
void updateAmbulance() {
  if (!ambulanceActive) return;

  unsigned long now = millis();

  // Stop after 10 seconds
  if (now - ambulanceStartMs >= ambulanceDurationMs) {
    ambulanceActive = false;
    ledState = false;
    digitalWrite(LED_RED, LOW);

    Serial.println("✅ Ambulance LED effect END");
    sendToMobius("alarm", "0");
    return;
  }

  // Toggle LED at flashStepMs
  if (now - lastFlashMs >= flashStepMs) {
    ledState = !ledState;
    digitalWrite(LED_RED, ledState ? HIGH : LOW);
    lastFlashMs = now;
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);

  // quick boot test
  digitalWrite(LED_RED, HIGH);
  delay(150);
  digitalWrite(LED_RED, LOW);

  connectWiFi();
  Serial.println("🚀 Setup complete");
}

// --- LOOP ---
void loop() {
  // keep LED flashing responsive
  updateAmbulance();

  // distance check timer
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < intervalMs) return;
  lastCheck = millis();

  float d = readDistance();
  Serial.print("📏 Distance: ");
  Serial.println(d);

  // send distance to Mobius
  if (d > 0) sendToMobius("distance", String(d));

  // Too-close state with hysteresis
  if (!tooClose && d > 0 && d <= dangerOnCm) {
    tooClose = true;
    if (!ambulanceActive) startAmbulance();
  }
  else if (tooClose && d >= dangerOffCm) {
    tooClose = false;
  }

  // Safety: stop if sensor fails
  if (d < 0) {
    Serial.println("⚠️ Sensor Error / Out of Range");
    tooClose = false;
    if (ambulanceActive) {
      ambulanceActive = false;
      digitalWrite(LED_RED, LOW);
      sendToMobius("alarm", "OFF");
    }
  }
}