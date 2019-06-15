#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <SoftwareSerial.h>
#include <TinyGPS.h>

//gps 포트 설정
static const uint32_t GPSBaud = 9600;

//와이파이 아이디 비번
const char* ssid = "happy";
const char* password = "00000940";

TinyGPS gps;
SoftwareSerial ss(4, 5); //rx(gpi04==d2), tx(gpi05==d1)

int status=0; //킥보드 초기상태(릴레이 0차단 1작동)

void setup () {
  pinMode(12, OUTPUT);
  Serial.begin(115200);
  wifiSet();
  ss.begin(9600);

}

void loop() {

  getStatus(); // status확인
  //delay(3000);
  
  getGps();
  delay(3000);    //Send a request every 30 seconds
  

}


void getGps() {
  Serial.println("getGPS...");
  
  bool newData = false;
 for (unsigned long start = millis(); millis() - start < 3000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.println();
    Serial.print("flat은: ");
    String lat = String(flat, 6);
    Serial.println(lat);
    Serial.print("flng은: ");
    String lng = String(flon, 6);
    Serial.println(lng);

    setKickGps(lat,lng);
  }
 
}

void setKickGps(String lat, String lng){

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin("http://3.17.25.223/api/kick/gps/set/201435003/"+lat+"/"+lng);  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
    Serial.println(httpCode);
    if (httpCode == 200) { //Check the returning code  200 성공코드

      //
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload

      StaticJsonDocument<300> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if ( error ) {
        Serial.println(F("Parsing failed!"));
        return;
      }

      // Decode JSON/Extract values

      const int result = doc["result"];


      Serial.println(F("Response:"));
      Serial.println(result);
      Serial.println(status);

    }

    http.end();   //Close connection

  }
  
}


void wifiSet() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {  //와이파이 연결될때까지 반복

    delay(1000);
    Serial.print("Connecting..");

  }
}
void getStatus() {

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin("http://3.17.25.223/api/kick/status/201435003");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
    Serial.println(httpCode);
    if (httpCode == 200) { //Check the returning code  200 성공코드

      //
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload

      StaticJsonDocument<300> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if ( error ) {
        Serial.println(F("Parsing failed!"));
        return;
      }

      // Decode JSON/Extract values

      const int result = doc["result"];
      status = doc["status"];

      Serial.println(F("Response:"));
      Serial.println(result);
      Serial.println(status);

      //킥보드 통신으로 전원제어
      if(status == 1){
        digitalWrite(12, HIGH);
      }else{
        digitalWrite(12, LOW);
      }

    }

    http.end();   //Close connection

  }
}
