// This example demonstrates control over SPI to the Microchip McpDigitalPot Digital potentometer
// SPI Pinouts are for Arduino Uno and Arduino Duemilanove board (will differ for Arduino MEGA)

// Download these into your Sketches/libraries/ folder...

// The Spi library by Cam Thompson. It was originally part of FPU library (micromegacorp.com)
// Available from http://arduino.cc/playground/Code/Fpu or http://www.arduino.cc/playground/Code/Spi
// Including SPI.h vv below initializea the MOSI, MISO, and SPI_CLK pins as per ATMEGA 328P
#include <SPI.h>

// McpDigitalPot library available from https://github.com/dreamcat4/McpDigitalPot
#include <McpDigitalPot.h>

// Wire up the SPI Interface common lines:
// #define SPI_CLOCK            13 //arduino   <->   SPI Slave Clock Input     -> SCK (Pin 02 on McpDigitalPot DIP)
// #define SPI_MOSI             11 //arduino   <->   SPI Master Out Slave In   -> SDI (Pin 03 on McpDigitalPot DIP)
// #define SPI_MISO             12 //arduino   <->   SPI Master In Slave Out   -> SDO (Pin 13 on McpDigitalPot DIP)

// Then choose any other free pin as the Slave Select (pin 10 if the default but doesnt have to be)
#define MCP_DIGITAL_POT_SLAVE_SELECT_PIN 8 //arduino   <->   Chip Select               -> CS  (Pin 01 on McpDigitalPot DIP)

// Its recommended to measure the rated end-end resistance (terminal A to terminal B)
// Because this can vary by a large margin, up to -+ 20%. And temperature variations.
float rAB_ohms = 5090.00; // 5k Ohm

// Instantiate McpDigitalPot object, with default rW (=117.5 ohm, its typical resistance)
McpDigitalPot digitalPot = McpDigitalPot( MCP_DIGITAL_POT_SLAVE_SELECT_PIN, rAB_ohms );

// rW - Wiper resistance. This is a small additional constant. To measure it
// use the example, setup(). Required for accurate calculations (to nearest ohm)
// Datasheet Page 5, gives typical values MIN=75ohm, MAX @5v=160ohm,@2.7v=300ohm
// Usually rW should be somewhere between 100 and 150 ohms.
// Instantiate McpDigitalPot object, after measuring the real rW wiper resistance
// McpDigitalPot digitalPot = McpDigitalPot( MCP_DIGITAL_POT_SLAVE_SELECT_PIN, rAB_ohms, rW_ohms );

const int power = 7;
const int powerIndicator = 6;

int currentVolume = 0;

void setup() {
  int state = 0;
  
  // initialize SPI:
  SPI.begin();

  // initialize Serial
  Serial.begin(9600);
  
  // initialize digital pins
  pinMode(power, OUTPUT);
  digitalWrite(power, HIGH);
  pinMode(powerIndicator, INPUT);

  digitalPot.scale = 128.0;
  digitalPot.setResistance(0, 0);
  
  Serial.print("> ");
}

void loop() {
  
}

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
  digitalWrite(power, 0);
  Serial.println("ON");
}

/*
 * Turn the speaker power off
 */
void off() {
  digitalWrite(power, 1);
  Serial.println("OFF");
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
  digitalPot.setResistance(0, volume);
  currentVolume = volume;
}
