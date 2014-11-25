#include <SPI.h>

// McpDigitalPot library available from https://github.com/dreamcat4/McpDigitalPot
#include <McpDigitalPot.h>

// Webduino library available from https://github.com/sirleech/Webduino
#include <Ethernet.h>
#include <WebServer.h>

// no-cost stream operator as described at 
// http://sundial.org/arduino/?page_id=119
template<class T>
inline Print &operator <<(Print &obj, T arg)
{ obj.print(arg); return obj; }

// Digital pins
#define POWER_PIN 7
#define VOLUME_SLAVE_SELECT_PIN 8

// Instantiate the volumeControl object
const float rAB_ohms = 5090.00; // 5k Ohm
McpDigitalPot volumeControl = McpDigitalPot(VOLUME_SLAVE_SELECT_PIN, rAB_ohms);

// Ethernet
static uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x38, 0x52 };
static uint8_t ip[] = { 192, 168, 1, 10 };

// Web server
#define PREFIX ""
WebServer webserver(PREFIX, 80);

// Global variables
int currentVolume = 0;
boolean powered = false;

/*
 * SerialEvent occurs whenever a new data comes in the
 * hardware serial RX.
 */
void serialEvent() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    Serial.println(command);
    
    if (command == "on") {
      on();
    }
    else if (command == "off") {
      off();
    }
    else {
      char* commandChars = new char[command.length() + 1];
      command.toCharArray(commandChars, command.length() + 1);
      int volume = atoi(commandChars);
      setVolume(volume);
    }
    
    Serial.print("\n> ");
  }
}

/*
 * Turn the speaker power on
 */
void on() {
  digitalWrite(POWER_PIN, 0);
  powered = true;
  Serial.println("ON");
}

/*
 * Turn the speaker power off
 */
void off() {
  digitalWrite(POWER_PIN, 1);
  powered = false;
  Serial.println("OFF");
}

/*
 * Returns true if the speakers are currently powered
 */
boolean isPowered() {
  return powered;
}

/*
 * Set the speaker volume to the given volume (range 0 - 128)
 */
void setVolume(int volume) {
  if (volume < 0 || volume > 128) {
    Serial.print("INVALID VOLUME ");
    Serial.println(volume);
    return;
  }

  Serial.print("SET VOLUME ");
  Serial.println(volume);
  volumeControl.setResistance(0, volume);
  currentVolume = volume;
}

/*
 * Get the current speaker volume
 */
int getVolume() {
  return currentVolume;
}

/*
 * GET /
 * Renders the main volume control page
 */
void index(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  if (type == WebServer::GET) {
    server.httpSuccess();
    server.print(F("<h1>Volume control</h1>"));
  }
  else {
    server.httpFail();
  }
}

/*
 * GET /power
 * Renders a JSON response with the current power state
 *
 * POST /power
 * Sets the current power state
 *
 * Params:
 *    power - "on" | "off"
 */
void power(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  char name[16], value[16];

  if (type == WebServer::GET) {
    server.httpSuccess("application/json");
    server << "{\"power\":\"" << (isPowered() ? "on" : "off") << "\"}";
  }
  else if (type == WebServer::POST) {
    server.httpNoContent();
    server.readPOSTparam(name, 16, value, 16);
    if (strcmp(name, "power") == 0) {
      if (strcmp(value, "on") == 0) {
        on();
      }
      else if (strcmp(value, "off") == 0) {
        off();
      }
      else {
        server.httpFail();
      }
    }
    else {
      server.httpFail();
    }
  }
  else {
    server.httpFail();
  }
}

/*
 * GET /volume
 * Renders a JSON response with the current volume level
 *
 * POST /volume
 * Sets the current volume level
 *
 * Params:
 *    volume - an integer 0 <= n <= 128
 */
void volume(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  char name[16], value[16];

  if (type == WebServer::GET) {
    server.httpSuccess("application/json");
    server << "{\"volume\":" << getVolume() << "}";
  }
  else if (type == WebServer::POST) {
    server.httpNoContent();
    server.readPOSTparam(name, 16, value, 16);
    if (strcmp(name, "volume") == 0) {
      setVolume(atoi(value));
    }
    else {
      server.httpFail();
    }
  }
  else {
    server.httpFail();
  }
}

void setup() {
  int state = 0;

  // initialize SPI:
  SPI.begin();

  // initialize Serial
  Serial.begin(9600);

  // initialize digital pins
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  // Initialize digital pot
  volumeControl.scale = 128.0;
  volumeControl.setResistance(0, 0);

  // Initialize ethernet
  Ethernet.begin(mac, ip);

  // Initialize web server
  webserver.setDefaultCommand(&index); // GET /
  webserver.addCommand("power", &power); // GET /power & POST /power
  webserver.addCommand("volume", &volume); // GET /volume & POST /volume
  webserver.begin();

  Serial.print("> ");
}

void loop() {
  char buf[64];
  int len = 64;

  // process incoming connections forever
  webserver.processConnection(buf, &len);
}
