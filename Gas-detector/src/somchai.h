#include <Arduino.h>
#include <Bounce2.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

//GAS sensor definition
#define GAS_PIN 25
int Gv = 0;
#define G_WARN 1700
#define G_DANGER 3000
#define G_SAFE 1000
//LED
#define LED_PIN 2
//Buzzer
#define BUZZER_PIN 4

enum FN_STATE{//fn running state essential for multitasking
    RUNNING,
    END
};

FN_STATE Gas_value_state=END;
int Gas_value(){
    Gas_value_state=RUNNING;
    Gv = analogRead(GAS_PIN);
    Gas_value_state=END;
}

FN_STATE Flick_LED_state=END;
int Flick_LED(int freq){
    Flick_LED_state=RUNNING;
    float T=(1/freq)*1000;//in ms
    float T_half=T/2;
    float t=(float)millis();
    if(fmod(t,T)<T_half){
        digitalWrite(LED_PIN, HIGH);
    }else{
        digitalWrite(LED_PIN, LOW);
    }
    Flick_LED_state=END;
}
FN_STATE Flick_Buzzer_state=END;
int Flick_Buzzer(int freq){
    Flick_Buzzer_state=RUNNING;
    float T=(1/freq)*1000;//in ms
    float T_half=T/2;
    float t=(float)millis();
    if(fmod(t,T)<T_half){
        digitalWrite(BUZZER_PIN, HIGH);
    }else{
        digitalWrite(BUZZER_PIN, LOW);
    }
    Flick_Buzzer_state=END;
}

//POST
FN_STATE POST_Record_state=END;
void POST_Record(String Status){
    POST_Record_state=RUNNING;
    String url="group2.exceed19.online/add";
    String json;//body for post request
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(2048);
    doc["gas_quantity"] = Gv;
    doc["time"] = NULL;//actually null value
    doc["status"] = Status;
    doc["isCommand"]=false;


    serializeJson(doc, json);
    int httpCode = http.POST(json);
    if (httpCode/100==2) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    } else {
        Serial.println("Error on HTTP request");
    }
    POST_Record_state=END;

}





