// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

///////////// Temperature
#include <OneWire.h>
#include <DallasTemperature.h>
// GPIO where the DS18B20 is connected to
#define TEMP_SENSOR_PIN 5 // D1
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP_SENSOR_PIN);
// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature tempSensors(&oneWire);

String getTemperature() {
  tempSensors.requestTemperatures();
  float tempValue = tempSensors.getTempCByIndex(0);
  Serial.println(tempValue);
  return String(tempValue);
}

// Set LED GPIO
const int ledPin = 2; // build_in LED
// Stores LED state
String ledState;

// wifi Access Point
const char *APssid = "Demo AP ESP8266"; // The name of the Wi-Fi network that will be created
const char *APpassword = "TakeControl"; // The password required to connect to it, leave blank for an open network

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "OFF";
    }
    else{
      ledState = "ON";
    }
    Serial.println(ledState);
    return ledState;
  }
  else if (var == "TEMPERATURE"){
    return getTemperature();
  }

}
 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Start the DS18B20 sensor
  tempSensors.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
  tempSensors.begin();                     // Start the temperature sensor
  if (tempSensors.getDeviceCount() == 0)
  {
    Serial.printf("No DS18x20 temperature sensor found on pin %d. Rebooting.\r\n", TEMP_SENSOR_PIN);
    Serial.flush();
    delay(1000);
    ESP.reset();
  }

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // AP Mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APssid, APpassword); // Start the access point
  Serial.print("Access Point \"");
  Serial.println(APssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP()); // Send the IP address of the ESP8266 to the computer

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

    // Route to load style.css file
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  // Route to set GPIO to LOW
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to HIGH
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTemperature().c_str());
  });
 
  // Start server
  server.begin();
}
 
void loop(){
  
}
