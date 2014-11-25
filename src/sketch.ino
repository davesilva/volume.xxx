#include <SPI.h>

// McpDigitalPot library available from https://github.com/dreamcat4/McpDigitalPot
#include <McpDigitalPot.h>

// Webduino library available from https://github.com/sirleech/Webduino
#include <Ethernet.h>
#include <WebServer.h>

// index.html file
#include "index_html.h"

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
    server.printP(indexHtml);
  }
  else {
    server.httpFail();
  }
}

/*
 * GET /speakers
 * Renders a JSON response with the current power state and
 * volume level of the speakers
 *
 * POST /speakers
 * Sets the current power state and volume level
 *
 * Params:
 *    volume - an integer 0 <= n <= 128
 *    power - true | false
 */
void speakers(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  char name[16], value[16];
  bool repeat;

  if (type == WebServer::GET) {
    server.httpSuccess("application/json");
    server << "{";
    server << "\"power\":" << (isPowered() ? "true" : "false") << ",";
    server << "\"volume\":" << getVolume();
    server << "}";
  }
  else if (type == WebServer::POST) {
    server.httpNoContent();

    do {
      repeat = server.readPOSTparam(name, 16, value, 16);

      if (strcmp(name, "volume") == 0) {
        setVolume(atoi(value));
      }
      else if (strcmp(name, "power") == 0) {
        if (strcmp(value, "true") == 0) {
          on();
        }
        else if (strcmp(value, "false") == 0) {
          off();
        }
        else {
          server.httpFail();
          break;
        }
      }
      else {
        server.httpFail();
        break;
      }
    } while (repeat);
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
  webserver.addCommand("speakers", &speakers); // GET /speakers & POST /speakers
  webserver.begin();

  Serial.print("> ");
}

void loop() {
  char buf[64];
  int len = 64;

  // process incoming connections forever
  webserver.processConnection(buf, &len);
}

