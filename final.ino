#define BLYNK_PRINT Serial  
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFiClientSecure.h>

char auth[] = "g0VL8g5IsBbHupUr0qnn5MObUh9Mwjm6"; //The auth code is sent by Blynk on the registered email

char ssid[] = "Karan1201";  //Enter your WIFI Name
char pass[] = "karan1234";  //Enter your WIFI Password

#define DHTPIN 2          // The source of DHT11 is connected to D4 s
#define DHTTYPE DHT11
#define ON_Board_LED 2    

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
const char* host = "script.google.com";
const int httpsPort = 443;


WiFiClientSecure client; .
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbwmlg6KODiSfd-Cur-kWrDHmvhw_7gOFZaqcGlg_MDzU9cVBP1YtwS__eoS61Ff8TpGKw"; // The GAS ID is the google script url

void setup()
{
  Serial.begin(115200);
  delay(500);
  dht.begin();
  delay(500);
  WiFi.begin(ssid, pass); 
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); // On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); // To Turn off Led On Board


  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  
  }
  
  digitalWrite(ON_Board_LED, HIGH); // Turn off the On Board LED when it is connected to the wifi router.
  //If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
  Blynk.begin(auth, ssid, pass);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(11, 0);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("Humi:");
  lcd.setCursor(11, 1);
  lcd.print("%"); 
}

unsigned long currTime = 0;
unsigned long delayNeeded = 3 * 60 * 1000;    // we need 3 minute pause before resending to spreadsheet
bool initial = true;


void loop()
{
  if(millis() - currTime >= delayNeeded || initial) {
    
    float h = dht.readHumidity();      // Readibg the temperature and humidity from the sensor
    float t = dht.readTemperature();
    
  
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor !");
      delay(500);
      return;
    }
    String Temp = "Temperature : " + String(t) + " °C";
    String Humi = "Humidity : " + String(h) + " %";
    Serial.println(Temp);
    Serial.println(Humi);
    sendData(t, h); // Calls the sendData Subroutine
    currTime = millis();
    initial = false;
  }
  
  Blynk.run(); // Initiates Blynk App
}

void sendData(float tem, int hum) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //Connecting to the Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //Processing the data and sending it.
  if (client.verify(fingerprint, host)) {
  Serial.println("certificate matches");
  } else {
  Serial.println("certificate doesn't match");
  }
  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
 

  //Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V5, t);  
  Blynk.virtualWrite(V6, h);  
  lcd.setCursor(5, 0);
  lcd.print(t);
  lcd.setCursor(5, 1);
  lcd.print(h);
  delay(1000);

} 
