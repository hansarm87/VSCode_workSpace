#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "IPUP";
const char* password = "HansOgPeter2300";
const char* hueBridgeIP = "192.168.0.2";
const char* hueUsername = "3dOuJDjKS0i1NiAyIBK0T1Zt0VYPXGQ3sE-o1iqb";
int relayPin1 = D1;

// Function prototypes
bool isMotionDetected();
bool isMotionDetectedFromJSON(String jsonResponse);

// Timer variables
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const int onDelayTime = 5 * 60 * 1000; // 5 minutes in milliseconds

bool relayOn = false; // To track the state of the relay

void setup() {
  Serial.begin(115200);
  pinMode(relayPin1, OUTPUT);

  // Ensure the relay is initially off
  digitalWrite(relayPin1, HIGH);
  relayOn = false;

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Check if motion is detected
  if (isMotionDetected()) {
    // If motion is detected and relay is not already on, turn on the relay
    if (!relayOn) {
      digitalWrite(relayPin1, LOW); // Note the change to LOW for normally open relay
      relayOn = true;
      previousTime = millis(); // Reset the timer
    }
  } else {
    // If no motion, turn off the relay after 5 minutes
    if (relayOn && (currentTime - previousTime >= onDelayTime)) {
      digitalWrite(relayPin1, HIGH); // Note the change to HIGH for normally open relay
      relayOn = false;
    }
  }

  // Update the current time
  currentTime = millis();

  delay(100); // Adjust delay as needed
}

bool isMotionDetected() {
  // Use HTTPClient with WiFiClient object to send a GET request to the Philips Hue API
  WiFiClient client;
  HTTPClient http;

  // Construct the URL for the Philips Hue API (replace with your specific sensor ID)
  String url = "http://" + String(hueBridgeIP) + "/api/" + String(hueUsername) + "/sensors/2";

  http.begin(client, url); // Use WiFiClient object as an argument

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("Response from Hue API:");
    Serial.println(response);

    // Parse the JSON response to check if motion is detected
    return isMotionDetectedFromJSON(response);
  } else {
    Serial.print("Hue API request failed, response code: ");
    Serial.println(httpResponseCode);
    return false;
  }

  // Close the connection
  http.end();
}

bool isMotionDetectedFromJSON(String jsonResponse) {
  // Parse the JSON response
  DynamicJsonDocument doc(1024); // Adjust the size based on your JSON response size
  deserializeJson(doc, jsonResponse);

  // Check if "presence" key is present and set to true
  if (doc.containsKey("state") && doc["state"].containsKey("presence")) {
    return doc["state"]["presence"];
  } else {
    // If key is not present, assume no motion
    return false;
  }
}
