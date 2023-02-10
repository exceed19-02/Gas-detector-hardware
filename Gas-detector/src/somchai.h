#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>


#define Buzzer_PIN 2
#define Gas_PIN 25
#define LED_PIN 4
#define servoPin 5
#define servoOpen 0
#define servoClose 90
#define wifi_name "OPPO_KUY"
#define wifi_pass "oppopass"



enum FN_STATE{//essential for multitask
    RUNNING,
    END
};




#define G_WARN 1700
#define G_DANGER 3000
#define G_SAFE 1000

int Gv = 0;
FN_STATE Gas_value_state=END;
void Gas_value(void* param){
    Gas_value_state=RUNNING;
    Gv = analogRead(Gas_PIN);
    Gas_value_state=END;
}

void Gas_setup(){
    pinMode(Gas_PIN, INPUT);
}




void Buzzer_setup(){
    pinMode(Buzzer_PIN, OUTPUT);
    digitalWrite(Buzzer_PIN, LOW);
}
void TurnOffBuzzer(){
    digitalWrite(Buzzer_PIN, LOW);
}
float T_Buzzer=0;
void Set_Buzzer_freq(int freq){
    if(freq==0){
        digitalWrite(Buzzer_PIN, LOW);
    }else{
        T_Buzzer=(1/freq)*1000;//in ms
    }   
}
FN_STATE FlashBuzzer_state=END;
void FlashBuzzer(void * param){
    FlashBuzzer_state=RUNNING;
    float T=T_Buzzer;
    float T_half=T/2;
    float t=(float)millis();
    if(fmod(t,T)<T_half){
        digitalWrite(Buzzer_PIN, HIGH);
    }else{
        digitalWrite(Buzzer_PIN, LOW);
    }
    FlashBuzzer_state=END;
}








void LED_setup(){
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}
void TurnOffLED(){
    digitalWrite(LED_PIN, LOW);
}
float T_LED=0;
void Set_LED_freq(int freq){
    if(freq==0){
        digitalWrite(LED_PIN, LOW);
    }else{
        T_LED=(1/freq)*1000;//in ms
    }   
}
FN_STATE FlashLED_state=END;
void FlashLED(void* param){
    FlashLED_state=RUNNING;
    float T=T_LED;
    float T_half=T/2;
    float t=(float)millis();
    if(fmod(t,T)<T_half){
        digitalWrite(LED_PIN, HIGH);
    }else{
        digitalWrite(LED_PIN, LOW);
    }
    FlashLED_state=END;
}




Servo servo;

void OpenWindow(){
    servo.write(servoOpen);
}
void CloseWindow(){
    servo.write(servoClose);
}
void Servo_setup(){
    servo.attach(servoPin);
    CloseWindow();
}



void Internet_setup() {
    WiFi.begin(wifi_name, wifi_pass);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("OK! IP=");
    Serial.println(WiFi.localIP());
}

FN_STATE POST_Record_state=END;
void POST_Record(String Status){
    POST_Record_state=RUNNING;
    String url="https://ecourse.cpe.ku.ac.th/exceed02/add";
    String json;//body for post request
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(2048);
    doc["gas_quantity"] = Gv;
    doc["time"] = NULL;
    doc["status"] = Status;
    doc["isCommand"]=false;


    serializeJson(doc, json);
    int httpCode = http.POST(json);
    if (httpCode/100==2) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    } else {
        Serial.println("Error on POST_Record");
        Serial.println(httpCode);
    }
    POST_Record_state=END;

}

FN_STATE GET_Window_Command_state=END;
void GET_Window_Command(void* param){
    GET_Window_Command_state=RUNNING;
    String url="https://ecourse.cpe.ku.ac.th/exceed02/record/command";
    DynamicJsonDocument doc(2048);
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode/100==2){
        String payload = http.getString();
        deserializeJson(doc, payload);
        bool isCommand=doc["isCommand"];
        if(isCommand){
            servo.write(servoOpen);
        }else{
            servo.write(servoClose);
        }
    }else{
        Serial.println("Error on GET_Window_Command");
        Serial.println(httpCode);
    }
    GET_Window_Command_state=END;
}


FN_STATE PUT_OpenWindow_state=END;
void PUT_OpenWindow(bool windowOpen){
    PUT_OpenWindow_state=RUNNING;
    String baseurl="https://ecourse.cpe.ku.ac.th/exceed02/update/";
    String url=baseurl+(windowOpen?"true":"false");
    HTTPClient http;
    http.begin(url);
    int httpCode = http.PUT("");
    if (httpCode/100==2) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    } else {
        Serial.println("Error on PUT_OpenWindow");
        Serial.println(httpCode);
    }
    PUT_OpenWindow_state=END;
}




void setup_devices(){
    LED_setup();
    Gas_setup();
    Buzzer_setup();
    Servo_setup();
    Internet_setup();
}