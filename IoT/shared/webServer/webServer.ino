#include <dummy.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <SimpleWebServer.h>

void (*loopFunc)();

const char ap_pass[] = "zeebo123";
const char host[] = "192.168.1.95";

char ssid[32];
char wifiPassword[64];

SimpleWebServer server;
WiFiClient client;

void displaySetupForm(WiFiClient client, int numParams, char** keys, char** values) {
  client.write("<!DOCTYPE html><html> <head> <meta charset=\"utf-8\"> </head> <body> <form action=\"/\" method=\"POST\"> <input name=\"ssid\" placeholder=\"SSID\" /> <input name=\"password\" type=\"password\" placeholder=\"Password\" /> <input type=\"submit\" value=\"Save\" /> </form> </body></html>");
  client.flush();
}

void saveSettings(WiFiClient client, int numParams, char** keys, char** values) {

  Serial.println("Saving settings");
  for (int i = 0; i < numParams; i++) {
    Serial.print("Key: ");
    Serial.println(keys[i]);
    if (strcmp("ssid", keys[i]) == 0) {
      Serial.print("SSID Param Found: ");
      Serial.println(values[i]);
      strcpy(ssid, values[i]);
    }
    if (strcmp("password", keys[i]) == 0) {
      Serial.print("PASS Param Found: ");
      Serial.println(values[i]);
      strcpy(wifiPassword, values[i]);
    }
  }

  Serial.print("SSID: ");
  Serial.println(ssid);

  Serial.print("PASS: ");
  Serial.println(wifiPassword);
  
  client.write("<!DOCTYPE html><html><body><p>Saving.</p><p>Restarting device with new configuration. Hold configure button while restarting to get back to the configuration page.</p></body></html>");
  client.flush();

  bool erase = false;
  for (int i = 0; i < 32; i++) {
    EEPROM.write(i, ssid[i]);
    if (ssid[i] == 0) {
      break;
    }
  }
  
  for (int i = 0; i < 64; i++) {
    EEPROM.write(i + 32, wifiPassword[i]);
    if (wifiPassword[i] == 0) {
      break;
    }
  }

  EEPROM.commit();

  Serial.println("Restarting");

  pinMode(0, INPUT_PULLUP);
  ESP.restart();
}

void configServerLoop() {
  server.handleRequest();
}
void runConfigServer() {
  Route* home = new Route("/", GET, &displaySetupForm);
  server.addRoute(home);
  Serial.println("Done adding Route");
  Route* postHome = new Route("/", POST, &saveSettings);
  server.addRoute(postHome);
  Serial.println("Done adding Route");

  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);

  Serial.print("MAC Address: ");
  for (int i = 0; i < WL_MAC_ADDR_LENGTH; i++) {
    Serial.print(i);
    Serial.print(" ");
  }
  Serial.println();
  
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 " + macID;

  Serial.print("AP Name: ");
  Serial.println(AP_NameString);

  WiFi.softAP(AP_NameString.c_str(), ap_pass);

  Serial.println("AP Running.");

  loopFunc = &configServerLoop;
}

bool loadWifiConfig() {
  Serial.println("Loading wifi config");
  
  bool hasSsid = false;
  bool hasPassword = false;
  
  Serial.println();
  Serial.print("SSID: ");
  for (int i = 0; i < 32; i++) {
    ssid[i] = EEPROM.read(i);
    Serial.print(ssid[i]);
    if (ssid[i] == 0) {
      break;
    }
    if (ssid[i] < 32 || ssid[i] > 126) {
      return false;
    }
    hasSsid = true;
  }
  Serial.println();
  Serial.print("Password: ");
  for (int i = 0; i < 64; i++) {
    wifiPassword[i] = EEPROM.read(i + 32);
    Serial.print(wifiPassword[i]);
    if (wifiPassword[i] == 0) {
      break;
    }
    if (wifiPassword[i] < 32 || wifiPassword[i] > 126) {
      return false;
    }
    hasPassword = true;
  }
  Serial.println();

  return hasSsid && hasPassword;
}

void controlLoop() {
  if (WiFi.status() != WL_CONNECTED) {
    
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.println(wifiPassword);
    
    WiFi.begin(ssid, wifiPassword);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
  }

  if (!client.connected()) {
    Serial.println();
    Serial.print("Attempting to connect to server at ");
    Serial.println(host);
    // Use WiFiClient class to create TCP connections
    while (!client.connect(host, 7788)) {
      Serial.println("Connection attempt failed.");
      delay(1000);
    }

    Serial.println("Connected.");
  }

  char bytes[1024];
  int length = client.readBytesUntil('\0', bytes, 1024);

  if (length > 0) {
    Serial.print("Pin: ");
    Serial.println(bytes[0] - '0');
    Serial.print("Value: ");
    Serial.println(bytes[1] - '0');
    pinMode(bytes[0] - '0', OUTPUT);
    digitalWrite(bytes[0] - '0', bytes[1] - '0');
  }
  
  Serial.println();
  Serial.println("Looping...");
}

void setup() {
  EEPROM.begin(512);
  Serial.begin(115200);
  pinMode(12, INPUT);
  while(!Serial);
  delay(500);
  Serial.println("Booting ESP8266");
  Serial.print("Pin 13 High? ");
  Serial.println(digitalRead(12));
  if (!loadWifiConfig() || digitalRead(12)) {
    Serial.println("Wifi configuration not found. Loading Config Site");
    runConfigServer();
  }
  else {
    Serial.println("Wifi configuration FOUND. Controlling the circuits");
    loopFunc = &controlLoop;
  }
}

void loop() {
  if (loopFunc) {
    loopFunc();
  }
}

