#include <Arduino.h>
#include <Bounce2.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

int Gv = 0;
#define G_WARN
#define G_DANGER
#define G_SAFE
int ledPin = 3;

const int analogInPin = 25;
int Gas_value(){
    Gv = analogRead(analogInPin);
    return Gv;
}


int Flick_LED(int freq){
    freq = analogRead(analogInPin);

    // analogWrite(ledPin, val / 4); // ส่งค่าสัญญาณ analog ไปควบคุมความสว่างหลอดไฟ LED
    // delay(5);

}

int Flick_Buzzer(int freq){
    freq = analogRead(analogInPin);
        return freq;
}

//POST
void POST_data(String Status){
    String url="<Request for post url>";
    String json;//body for post request
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(2048);
    doc["Status"] = Status;

    serializeJson(doc, json);
    int httpCode = http.POST(json);
    if (httpCode/100==2) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    } else {
        Serial.println("Error on HTTP request");
    }

}





