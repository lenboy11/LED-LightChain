
#define RED_PIN 13
#define GREEN_PIN 15
#define BLUE_PIN 12
#define BUT_PIN 0

#include <Arduino.h>
#include <RGBLed.h>
// #include <DNSServer.h> (only if own wifi is used)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Bounce2.h>
#include <SPI.h>
#include <SD.h>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;


// External variables from password.cpp
extern const char* ssidExtern;
extern const char* passwordExtern;

// For Controller with one wifi (only if own wifi is used) 
// #define SSID "LED Testzentrum"
// #define WEB_PAGE_FILEPATH "/index.html"

// For Controller added to home network via wifi (only if added to home network is used) 
const char* ssid = ssidExtern;
const char* password = passwordExtern;



const int ledPin = LED_BUILTIN;
int blink = LOW;
unsigned long previousMillis = 0;
const long interval = 1000;

// Declaring Functions Used (Every Function needs to be added here!)
String processor(const String &var);
void updateLED(int r, int g, int b);
void fadeLED(int r, int g, int b, int fv, int re);
void colorFade(String color, int reps);
void randomFade(int reps);
void overwriteIP(IPAddress ip, const char* jsFile);

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

/* Create DNS server instance to enable captive portal. (only if own wifi is used) */
// DNSServer dnsServer;

/* Create webserver instance for serving the StringTemplate example. */
AsyncWebServer server(80);

/* Soft AP network parameters (only if own wifi is used) */
// IPAddress apIP(192, 168, 4, 1);
// IPAddress netMsk(255, 255, 255, 0);

// Testing in Development Phase
  String testString = "C855";

  void test(String S){

    char test[3];
    S.substring(0,2).toCharArray(test, 3);
    int out = (int)strtol(test, NULL, 16);
    Serial.println("test:");
    Serial.println(test);
    Serial.println(out);
  }

// Setup the Controller
void setup() {
  SPIFFS.begin();

  Serial.begin(115200);  // Use Serial 9600 if own wifi is used
  Serial.println("Startet Serial!");

  led.off();

  button.attach(BUT_PIN, INPUT);
  button.interval(5);
  button.setPressedState(LOW);

  pinMode(A0, INPUT);
  Serial.println(analogRead(A0));
  randomSeed(analogRead(A0));

  while( !Serial );

  // Connect to Wi-Fi (only if added to home network is used)
  IPAddress local_IP(192, 168, 178, 94);
  IPAddress dns(192, 168, 1, 1);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress subnet2(255, 255, 0, 0);

  if (!WiFi.config(local_IP, gateway, subnet2, primaryDNS, secondaryDNS)) {
    Serial.println("STA2 Failed to configure");
    if (!WiFi.config(local_IP, dns, gateway, subnet)) {
      Serial.println("STA1 Failed to configure");
    }
  }

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    delay(6000);
    status = WiFi.status();
  }

  // Print ESP Local IP Address (only if added to home network is used) 
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIP());

  /* Configure access point with static IP address (only if own wifi is used) */
  // WiFi.mode(WIFI_AP);
  // WiFi.softAPConfig(apIP, apIP, netMsk);
  // WiFi.softAP(SSID);

  // overwrite IP-Adress to scripts.js - File
  overwriteIP(WiFi.localIP(), "/script.js");
  overwriteIP(WiFi.localIP(), "/visualScript.js");
    

  /* Start DNS server for captive portal. Route all requests to the ESP IP address. (only if own wifi is used) */
  // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  // dnsServer.start(53, "*", apIP);

  /* Define the handler for when the server receives a GET request for the root uri. */
  server.onNotFound([](AsyncWebServerRequest *request) { request->send(SPIFFS, "/index.html", String(), false, processor); });

  // Route to load css files
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/style.css", "text/css");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/colorpicker.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/colorpicker.css", "text/css");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Route to load js files
  server.on("/iro.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/iro.js", "application/javascript");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/jscolor.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/jscolor.js", "application/javascript");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/main.js", "application/javascript");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/script.js", "application/javascript");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/visualScript.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/visualScript.js", "application/javascript");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Route to visual HTML file
  server.on("/visual", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/visuals.html", String(), false, processor);
  });

  // Route to load font files
  server.on("/font/font.woff2", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/font/font.woff2", "text/woff2");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/font/font2.woff2", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/font/font2.woff2", "text/woff2");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Turn LED on
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    //Set random colour
    updateLED(random(256), random(256), random(256));
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Turn LED off
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    //Set LED off
    updateLED(0, 0, 0);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Slider
  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage = "No message sent";
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    int new_red = v_red;
    int new_green = v_green;
    int new_blue = v_blue;
    if (request->hasParam("red"))
    {
      inputMessage = request->getParam("red")->value();
      new_red = inputMessage.toInt();
    }
    if (request->hasParam("green"))
    {
      inputMessage = request->getParam("green")->value();
      new_green = inputMessage.toInt();
    }
    if (request->hasParam("blue"))
    {
      inputMessage = request->getParam("blue")->value();
      new_blue = inputMessage.toInt();
    }
    if (request->hasParam("rgb"))
    {
      inputMessage = request->getParam("rgb")->value();
      new_red = inputMessage.substring(0,3).toInt();
      new_green = inputMessage.substring(3,6).toInt();
      new_blue = inputMessage.substring(6,9).toInt();
    }
    if (request->hasParam("rgbHex")) // Does not work yet or does it?
    {
      inputMessage = request->getParam("rgb")->value();
      char redChar[3]; 
      inputMessage.substring(0,2).toCharArray(redChar, 3);
      new_red = (int)strtol(redChar, NULL, 16);
      char greenChar[3]; 
      inputMessage.substring(2,4).toCharArray(greenChar, 3);
      new_green = (int)strtol(greenChar, NULL, 16);
      char blueChar[3]; 
      inputMessage.substring(4,6).toCharArray(blueChar, 3);
      new_blue = (int)strtol(blueChar, NULL, 16);
    }
    if (request->hasParam("fade")) {
      inputMessage = request->getParam("fade")->value();
      int fade_value = inputMessage.toInt();
      Serial.println("Von 001: Request hasParam fade");
      Serial.println(new_red);
      fadeLED(new_red, new_green, new_blue, fade_value, 0);
    } else {
      updateLED(new_red, new_green, new_blue);
    }
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Random Fade
  server.on("/fade", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage = "No message sent";
    // Call Random Fade Function
    int reps = 10;
    if (request->hasParam("reps")) {
      inputMessage = request->getParam("reps")->value();
      reps = inputMessage.toInt();
    }
    if (request->hasParam("color"))
    {
      inputMessage = request->getParam("color")->value();
      colorFade(inputMessage, reps);
    } else {
      randomFade(reps);
    }
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Get Color
  server.on("/getColor", HTTP_GET, [](AsyncWebServerRequest *request) {
    String resp = "r=" + String(v_red) + ",g=" + String(v_green) + ",b=" + String(v_blue);
    Serial.println(resp);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", resp );
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server.begin();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  test(testString);
}

void loop()
{
  // dnsServer.processNextRequest(); (only if own wifi is used)

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
      updateLED(0, 0, 0);
      power = false;
    }
    else
    {
      power = true;
      updateLED(s_red, s_green, s_blue);
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

void updateLED(int c_red, int c_green, int c_blue)
{
  if (power)
  {
    v_red = c_red; v_green = c_green; v_blue = c_blue;
    led.setColor(v_red, v_green, v_blue);
  }
}

void fadeLED(int new_red, int new_green, int new_blue, int fade, int reps)
{
  int old_red = v_red;
  int old_green = v_green;
  int old_blue = v_blue;
  int i = 0;
  Serial.println("r");
  Serial.println(new_red);
  Serial.println("g");
  Serial.println(new_green);
  Serial.println("b");
  Serial.println(new_blue);
  do {
    for( int cc = 1; cc <= fade; cc = cc + 1 ) {
      updateLED(old_red*(1 - cc/fade) + new_red*(cc/fade) , old_green*(1 - cc/fade) + new_green*(cc/fade), old_blue*(1 - cc/fade) + new_blue*(cc/fade));
      delay(1000);
    }
    Serial.println("r");
    Serial.println(new_red);
    Serial.println("g");
    Serial.println(new_green);
    Serial.println("b");
    Serial.println(new_blue);
    i = i+1;
    new_red = old_red;
    old_red = v_red;
    new_green = old_green;
    old_green = v_green;
    new_blue = old_blue;
    old_blue = v_blue;
  } while( i <= 2*reps);
}

void colorFade(String color, int reps) {
  int new_red = v_red;
  int new_green = v_green;
  int new_blue = v_blue;
  Serial.println("Von 002: colorFade");
  if (color.compareTo("red") == 0) {
    fadeLED(0, new_green, new_blue, new_red, 0);
    new_red = 255;
  } else if (color.compareTo("green") == 0) {
    fadeLED(new_red, 0, new_blue, new_green, 0);
    new_red = 255;
  } else if (color.compareTo("blue") == 0) {
    fadeLED(new_red, new_green, 0, new_blue, 0);
    new_red = 255;
  } else if (color.compareTo("all") == 0) {
    fadeLED(0, 0, 0, new_red + new_green + new_blue, 0);
    for( int i = 0; i < reps; i = i + 1 ){
      fadeLED(255, 0, 0, 255, 0);
      fadeLED(0, 255, 0, 255, 0);
      fadeLED(0, 0, 255, 255, 0);
    }
    reps = 0;
  } else if (color.compareTo("random") == 0) {
    randomFade(reps);
  }
  Serial.println("Von 002: colorFade");
  fadeLED(new_red, new_green, new_blue, 255, reps);
}

void randomFade(int reps) {
  int new_red = v_red;
  int new_green = v_green;
  int new_blue = v_blue;
  int fadeSpeed = 10;
  Serial.println("Von 003: randomFade");
  for( int cc = 0; cc != reps; cc = cc + 1) {
    new_red = random(256);
    new_green = random(256);
    new_blue = random(256);
    fadeSpeed = abs(new_red - v_red) + abs(new_green - v_green) + abs(new_blue - v_blue);
    fadeLED(new_red, new_green, new_blue, fadeSpeed, reps);
  }
}

// writes the current IP Adress in the jsFile
void overwriteIP(IPAddress ip, const char* jsFile) {
  string line;
    // open input file
  ifstream in(jsFile);
  if( !in.is_open()) {
    cout << "Input file failed to open\n";
  } else {
    // now open temp output file
    ofstream out("outfileTemp.txt");
    // loop to read/write the file.  Note that you need to add code here to check
    // if you want to write the line
    string start = "var ip = 'http://";
    string end = "/' \n";
    getline(in, line);
    out << start << ip.toString() << end; 
    while( getline(in,line) ) {
      out << line << "\n";
    }
    in.close();
    out.close();    
    // delete the original file
    SD.remove(jsFile);
    // rename old to new
    SPIFFS.rename("outfileTemp.txt", jsFile);
  }
}