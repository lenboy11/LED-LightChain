#include <Arduino.h>
#include <RGBLed.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Bounce2.h>

#define BLUE_PIN 12
#define RED_PIN 13
#define GREEN_PIN 15
#define BUT_PIN 0

#define SSID "LED Steuerung"
#define WEB_PAGE_FILEPATH "/index.html"

const int ledPin = LED_BUILTIN;
int blink = LOW;
unsigned long previousMillis = 0;
const long interval = 1000;

String processor(const String &var);
void updateLED();

// Button Pin für Flash Button
Bounce2::Button button = Bounce2::Button();
bool power = true;

/* Create RGB Led instance to control LED String*/
RGBLed led(RED_PIN, GREEN_PIN, BLUE_PIN, RGBLed::COMMON_CATHODE);
int v_red = 0;
int v_green = 0;
int v_blue = 0;

//EEPROM Variabels
int s_red = 0;
int s_green = 0;
int s_blue = 0;

/* Create DNS server instance to enable captive portal. */
DNSServer dnsServer;

/* Create webserver instance for serving the StringTemplate example. */
AsyncWebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

void setup()
{
  SPIFFS.begin();

  Serial.begin(9600);
  Serial.println("Startet Serial!");

  led.off();

  button.attach(BUT_PIN, INPUT);
  button.interval(5);
  button.setPressedState(LOW);

  pinMode(A0, INPUT);
  Serial.println(analogRead(A0));
  randomSeed(analogRead(A0));

  /* Configure access point with static IP address */
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(SSID);

  /* Start DNS server for captive portal. Route all requests to the ESP IP address. */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);

  /* Define the handler for when the server receives a GET request for the root uri. */
  server.onNotFound([](AsyncWebServerRequest *request) { request->send(SPIFFS, "/index.html", String(), false, processor); });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Turn LED on
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    //Set random colour
    v_blue = random(256);
    v_green = random(256);
    v_red = random(256);
    updateLED();
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Turn LED off
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    //Set LED off
    v_blue = 0;
    v_green = 0;
    v_red = 0;
    updateLED();
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Slider
  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam("red"))
    {
      inputMessage = request->getParam("red")->value();
      v_red = inputMessage.toInt();
      updateLED();
    }
    else if (request->hasParam("green"))
    {
      inputMessage = request->getParam("green")->value();
      v_green = inputMessage.toInt();
      updateLED();
    }
    else if (request->hasParam("blue"))
    {
      inputMessage = request->getParam("blue")->value();
      v_blue = inputMessage.toInt();
      updateLED();
    }
    else
    {
      inputMessage = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void loop()
{
  dnsServer.processNextRequest();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    Serial.printf("Red: %d, Green: %d, Blue: %d \n", v_red, v_green, v_blue);
    previousMillis = currentMillis;
    // if (blink == LOW)
    // {
    //   blink = HIGH;
    // }
    // else
    // {
    //   blink = LOW;
    // }
    // digitalWrite(ledPin, blink);
  }

  wifi_softap_get_station_num();

  button.update();

  if (button.pressed())
  {
    if (power)
    {
      s_blue = v_blue;
      s_green = v_green;
      s_red = v_red;
      v_green = 0;
      v_red = 0;
      v_blue = 0;
      updateLED();
      power = false;
    }
    else
    {
      v_blue = s_blue;
      v_green = s_green;
      v_red = s_red;
      power = true;
      updateLED();
    }
  }
}

String processor(const String &var)
{
  String ledState = "Error";
  if (var == "REDVALUE")
  {
    return String(v_red);
  }
  else if (var == "GREENVALUE")
  {
    return String(v_green);
  }
  else if (var == "BLUEVALUE")
  {
    return String(v_blue);
  }
  return "";
}

void updateLED()
{
  if (power)
  {
    led.setColor(v_red, v_green, v_blue);
  }
}
