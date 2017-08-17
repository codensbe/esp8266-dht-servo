#include "Servo.h"
#include "DHT.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

#define DHTPIN 10
#define DHTTYPE DHT22

const char* ssid = "ssid";
const char* password = "password";

// Create an instance of the server on port 80
ESP8266WebServer server(80);

Servo servo;
DHT dht(DHTPIN, DHTTYPE);

// Airco powered on/off
bool airco = false;

// Airco will power on when requestedTemp + buffer > temperature
float requestedTemp = 22.50;
float buffer = 1;

// Angle the servo has to turn to hit the airco button
float servoAngle = 30;

// Current temperature and humidity
float temperature;
float humidity;

void setup() 
{ 
  dht.begin();

  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Routes
  server.on("/modify", handleModifyPath);
  server.on("/", handleRootPath);

  server.begin();
  Serial.println("Server listening");
} 
 
void loop() 
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if(temperature - buffer > requestedTemp && airco == false) {
    turnServo(servoAngle);
    airco = true;
  } else if(temperature <= requestedTemp && airco == true) {
    turnServo(servoAngle);
    airco = false;
  }
  
  server.handleClient();
  
  delay(2000);
}

void handleModifyPath() {
  for (int i = 0; i < server.args(); i++) {
    if(server.argName(i) == "temperature"){
      String temperatureString = server.arg(i);
      requestedTemp = temperatureString.toFloat();
    }
    if(server.argName(i) == "buffer"){
      String bufferString = server.arg(i);
      buffer = bufferString.toFloat();
    }
    if(server.argName(i) == "airco"){
      String aircoString = server.arg(i);
      if(aircoString == "on"){
        airco = true;
      } else if(aircoString == "off"){
        airco = false;
      }
    }

    if(server.argName(i) == "angle"){
      String angleString = server.arg(i);
      servoAngle = angleString.toFloat();
    }
  } 
  
  // Show current variables
  server.send(200, "text/html", createPage());
}

void handleRootPath() {
  server.send(200, "text/html", createPage());
}

String createPage() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  String aircoStatus = airco ? "on" : "off";
  
  return "Temperature: " + String(temperature) + "<br />Humidity: " + String(humidity) + "<br /><br/> Requeste temperature: " + String(requestedTemp) + "<br />Buffer: " + String(buffer) + "<br />Airco: " + aircoStatus + "<br />Angle: " + servoAngle;
}

void turnServo(float angle) {
  // Servo gets attached and detached on each turn due to servo wiggle issue
  servo.attach(2);
  int pos;

  for(pos = 0; pos <= angle; pos += 1) {
    servo.write(pos);
    delay(15);
  } 

  for(pos = angle; pos>=0; pos-=1) {
    servo.write(pos);
    delay(15);
  }

  servo.detach();
}
