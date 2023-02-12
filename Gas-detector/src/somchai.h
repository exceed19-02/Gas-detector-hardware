#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>


#define LED_PIN 21
#define Gas_PIN 34
#define Buzzer_PIN 5
#define servoPin 18
//open > close
#define servoOpen 110
#define servoClose 35
#define wifi_name "OPPO_KUY"
#define wifi_pass "oppopass"

const char* wifi_names[]{"OPPO_KUY","OPPO_KUY_2G","OPPO_KUY_5G"};
const char* wifi_passs[]{"oppopass","oppopass","oppopass"};



enum FN_STATE{//essential for multitask
    RUNNING,
    END
};

#define G_WARNING 1500
#define G_DANGER 2000
#define G_SAFE 1000


int Gv = 0;
FN_STATE Gas_value_state=END;
TaskHandle_t Gas_value_h = NULL;
unsigned int Gas_value_timestamp=0;
void Gas_value(void* param){
    Gas_value_state=RUNNING;
    while(1){
        Gv = analogRead(Gas_PIN);
        if(millis()-Gas_value_timestamp>1000){
            Gas_value_timestamp=millis();
            Serial.print("Gas value: ");
            Serial.println(Gv);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    Gas_value_state=END;
}

void Gas_setup(){
    pinMode(Gas_PIN, INPUT);
    xTaskCreatePinnedToCore(Gas_value, "Gas_value", 1024*10, NULL, 1, &Gas_value_h, 0);
}







float T_LED=0.0f;
void Set_LED_freq(float freq){
    if(freq==0){
        digitalWrite(LED_PIN, LOW);
        T_LED=0.0f;
    }else{
        T_LED=(1.0f/freq);//in s
    }   
}
void TurnOff_LED(){
    Set_LED_freq(0);
}
FN_STATE FlashLED_state=END;
TaskHandle_t FlashLED_h = NULL;
void FlashLED(void * param){
    FlashLED_state=RUNNING;
    while(1){
        if(T_LED!=0.0f){
            float T=T_LED;
            float T_half=T/2;
            float t=(float)millis()/1000.0f;//in s
            if(fmod(t,T)<T_half){
                digitalWrite(LED_PIN, HIGH);
            }else{
                digitalWrite(LED_PIN, LOW);
            }
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    FlashLED_state=END;
}
void LED_setup(){
    pinMode(LED_PIN, OUTPUT);
    Set_LED_freq(0);
    Serial.println("LED_setup");
    xTaskCreatePinnedToCore(FlashLED, "FlashLED", 1024*10, NULL, 1, &FlashLED_h, 0);
}

float T_Buzzer=0.0f;
void Set_Buzzer_freq(float freq){
    if(freq==0){
        digitalWrite(Buzzer_PIN, LOW);
        T_Buzzer=0.0f;
    }else{
        T_Buzzer=(1.0f/freq);//in s
    }   
}
void TurnOff_Buzzer(){
    Set_Buzzer_freq(0);
}
FN_STATE FlashBuzzer_state=END;
TaskHandle_t FlashBuzzer_h = NULL;
void FlashBuzzer(void * param){
    FlashBuzzer_state=RUNNING;
    while(1){
        if(T_Buzzer!=0.0f){
            float T=T_Buzzer;
            float T_half=T/2;
            float t=(float)millis()/1000.0f;//in s
            if(fmod(t,T)<T_half){
                digitalWrite(Buzzer_PIN, HIGH);
            }else{
                digitalWrite(Buzzer_PIN, LOW);
            }
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    FlashBuzzer_state=END;
}
void Buzzer_setup(){
    pinMode(Buzzer_PIN, OUTPUT);
    Set_Buzzer_freq(0);
    Serial.println("Buzzer_setup");
    xTaskCreatePinnedToCore(FlashBuzzer, "FlashBuzzer", 1024*10, NULL, 1, &FlashBuzzer_h, 0);
}





Servo servo;
bool windowOpen=false;
int servoAngle_0=0;
int servoAngle_i=0;
int servoAngle_f=0;
unsigned int servo_timestamp=0;
const float servo_transitionTime=1.0f;//in s
float _Tcos_servo=2*servo_transitionTime;
FN_STATE Servo_update_state=END;
TaskHandle_t Servo_update_h = NULL;
void Servo_update(void* param){
    Servo_update_state=RUNNING;
    while(1){
        float T=_Tcos_servo;
        float t=(float)(millis()-servo_timestamp)/1000.0f;//in s
        int servoAngle=0;
        if(t>servo_transitionTime){t=servo_transitionTime;}
        else{Serial.println("Servo update");}
        servoAngle=(-cos(t*PI/servo_transitionTime)+1)/2*(servoAngle_f-servoAngle_0)+servoAngle_0;
        servo.write(servoAngle);
        servoAngle_i=servoAngle;
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    Servo_update_state=END;
}
void OpenWindow(){
    windowOpen=true;
    servoAngle_0=servoAngle_i;
    servoAngle_f=servoOpen;
    servo_timestamp=millis();
}
void CloseWindow(){
    windowOpen=false;
    servoAngle_0=servoAngle_i;
    servoAngle_f=servoClose;
    servo_timestamp=millis();
}
void Servo_setup(){
    servo.attach(servoPin); 
}






///Currently design request pattern
FN_STATE POST_Record_state=END;
unsigned int POST_Record_timestamp=0;
String Status="SAFE";
bool POST_Record_instant=false;
TaskHandle_t POST_Record_h = NULL;
void POST_Record(void* param){
    POST_Record_state=RUNNING;
    String url="https://ecourse.cpe.ku.ac.th/exceed02/add";
    while(1){
        if(POST_Record_instant==true || millis()-POST_Record_timestamp>=5*60*1000){
        //if(POST_Record_instant==true || millis()-POST_Record_timestamp>=1000){
            if(POST_Record_instant==true){
                POST_Record_instant=false;
            }
            POST_Record_timestamp=millis();
            String json;
            HTTPClient http;
            http.begin(url);
            http.addHeader("Content-Type", "application/json");
            DynamicJsonDocument doc(2048);
            doc["gas_quantity"] = Gv;
            doc["status"] = Status;
            Serial.print("Send status : ");
            Serial.println(Status);

            serializeJson(doc, json);
            int httpCode = http.POST(json);
            if (httpCode/100==2) {
                String payload = http.getString();
                Serial.println(httpCode);
                Serial.println(payload);
            } else {
                Serial.println(httpCode);
                Serial.println("POST failed");
            }
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    POST_Record_state=END;
}


TaskHandle_t GET_Window_Command_h = NULL;
FN_STATE GET_Window_Command_state=END;
unsigned int GET_Window_Command_timestamp=0;
bool Window_Commandable=true;
void GET_Window_Command(void* param){
    GET_Window_Command_state=RUNNING;
    String url="https://ecourse.cpe.ku.ac.th/exceed02/record/command";
    while(1){
        if(Window_Commandable==true){
            if(millis()-GET_Window_Command_timestamp>=1000){
                GET_Window_Command_timestamp=millis();
                DynamicJsonDocument doc(2048);
                HTTPClient http;
                http.begin(url);
                int httpCode = http.GET();
                if(httpCode/100==2){
                    String payload = http.getString();
                    deserializeJson(doc, payload);
                    bool isOpen=doc["isOpen"];
                    if(isOpen){
                        Serial.print("Window Open ");
                        OpenWindow();
                    }else{
                        Serial.print("Window Close");
                        CloseWindow();
                    }
                    Serial.println(isOpen);
                }else{
                    Serial.println("Error on GET_Window_Command");
                    Serial.println(httpCode);
                }
            }
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    GET_Window_Command_state=END;
}


FN_STATE PUT_OpenWindow_state=END;
bool PUT_OpenWindow_request=false;
void PUT_OpenWindow(bool windowOpen){
    PUT_OpenWindow_state=RUNNING;
    while(1){
        if(PUT_OpenWindow_request==true){
        
            String baseurl="https://ecourse.cpe.ku.ac.th/exceed02/update/";
            String url=baseurl+(windowOpen?"true":"false");

            HTTPClient http;
            http.begin(url);
            int httpCode = http.PUT("");
            if(httpCode/100==2){
                String payload = http.getString();
                Serial.println(httpCode);
                Serial.println(payload);
            }else{
                Serial.println("Error on PUT_OpenWindow");
                Serial.println(httpCode);
            }
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    PUT_OpenWindow_state=END;
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

    xTaskCreatePinnedToCore(POST_Record, "POST_Record", 1024*10, NULL, 1, &POST_Record_h, 1);
    xTaskCreatePinnedToCore(GET_Window_Command, "GET_Window_Command", 1024*10, NULL, 1, &GET_Window_Command_h, 1);
}



void setup_devices(){
    LED_setup();
    Gas_setup();
    Buzzer_setup();
    Servo_setup();
    Internet_setup();
}