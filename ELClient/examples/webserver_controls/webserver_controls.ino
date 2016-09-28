/**
 * This is a sample for trying out each HTML control
 * 
 * Test board:
 *    ESP8266 RX    <--level shifter-->    Arduino TX
 *    ESP8266 TX    <--level shifter-->    Arduino RX
 *    ESP8266 GPIO0 <--level shifter-->    Arduino RESET   (optional)
 *    
 *    Arduino LED is on PIN 13
 *    Connect an 1K trimmer to Arduino (voltage):  VCC <-> A0 <-> GND
 * 
 * 
 * Video:
 *    https://www.youtube.com/watch?v=vBESCO0UhYI
 */

#include <ELClient.h>
#include <ELClientWebServer.h>
#include "Pages.h"

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);

// Initialize the MQTT client
ELClientWebServer webServer(&esp);

void setup()
{
  Serial.begin(115200);
  
  esp.SetReceiveBufferSize(384);
  
  // register the JSON URL handlers
  webServer.registerHandler(F("/LED.html.json"), ledHtmlCallback);
  webServer.registerHandler(F("/User.html.json"), userHtmlCallback);
  webServer.registerHandler(F("/Voltage.html.json"), voltageHtmlCallback);
  webServer.setup();

  ledInit();
  userInit();
  voltageInit();
}

static uint32_t last;

void loop()
{
  esp.Process();

  ledLoop();
  voltageLoop();
  
  if ((millis()-last) > 4000) {
    webServer.registerCallback();  // just for esp-link reset: reregister callback at every 4s
    last = millis();
  }
}

