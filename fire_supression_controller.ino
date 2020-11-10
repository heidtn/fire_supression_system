/*
 * Fire supression controller
 * This is a simple controller for the fire supression system for the onboard 3D printer.  Using an MQ-2 sensor,
 * the device detects when a fire event occurs and triggers a valve releasing CO2 into the chamber and extinguishing
 * the fire.
 * 
 * Button code pulled from here: https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html
 * 
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#define VALVE_PIN D0
#define SMOKE_SENSOR_PIN A0
#define SMOKE_THRESHOLD 500  // TODO(MC) tune this value appropriately 
#define VALVE_ONTIME 10000

// function prototypes
void trigger_co2(int ms);
bool check_smoke(int smoke_val);
int get_smoke();
void handle_root();
void connect_to_wifi();
void setup_server();
void handle_gas_button();

// globals
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);


void setup() {
  Serial.begin(115200);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW);
  connect_to_wifi();

  Serial.println("Fire supression system active");
}

void loop() {
  auto smoke_val = get_smoke();
  Serial.print("Gas value: ");
  Serial.println(smoke_val);
  if(check_smoke(smoke_val)) {
    trigger_co2(VALVE_ONTIME);
    Serial.println("SMOKE EVENT DETECTED!!!");
  }
  server.handleClient();
  delay(1000);
}

void setup_server() {
  server.on("/", HTTP_GET, handle_root);     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/GAS", HTTP_POST, handle_gas_button);  // Call the 'handleLED' function when a POST request is made to URI "/LED"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void connect_to_wifi() {
  wifiMulti.addAP(ssid, password);
  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           

  if (MDNS.begin("fire_supression")) { 
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
}

void handle_root() {                         // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<form action=\"/GAS\" method=\"POST\"><input type=\"submit\" value=\"Toggle Gas\"></form>");
}

int get_smoke() {
  // todo(MC) might want to add some smoothing here depending on noise
  return analogRead(SMOKE_SENSOR_PIN);
}

bool check_smoke(int smoke_val) {
  // todo(MC) might want to add sample hysterisis 
  return smoke_val > SMOKE_THRESHOLD;
}

void trigger_co2(int ms) {
  digitalWrite(VALVE_PIN, HIGH);
  delay(ms);
  digitalWrite(VALVE_PIN, LOW);
}
