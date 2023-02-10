#include <Arduino.h>
#include <Fsm.h>
#include "somchai.h"
void on_SAFE_enter();
void on_SAFE_body();
void on_SAFE_exit();
void on_WARNING_enter();
void on_WARNING_body();
void on_WARNING_exit();
void on_DANGER_enter();
void on_DANGER_body();
void on_DANGER_exit();
enum {
    SAFEtoWARNING,
    SAFEtoDANGER,
    WARNINGtoDANGER,
    WARNINGtoSAFE,
    DANGERtoSAFE
};



State SAFE(&on_SAFE_enter, &on_SAFE_body, &on_SAFE_exit);
State WARNING(&on_WARNING_enter, &on_WARNING_body, &on_WARNING_exit);
State DANGER(&on_DANGER_enter, &on_DANGER_body, &on_DANGER_exit);
Fsm fsm(&SAFE);
String status;

void POST_Record_t(void * param){
    POST_Record(status);
}
void Gas_value_t(void * param){
    Gas_value();
}
int Flick_LED_freq=0;
void Flick_LED_t(void * param){
    Flick_LED(Flick_LED_freq);
}
int Flick_Buzzer_freq=0;
void Flick_Buzzer_t(void * param){
    Flick_Buzzer(Flick_Buzzer_freq);
}

void on_SAFE_enter() {
    status="SAFE";
    Serial.println(status);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("SAFE enter");
    xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 1, NULL, 0);
}
void on_SAFE_body() {
    TaskHandle_t PAR1 = NULL;
    TaskHandle_t PAR2 = NULL;
    int PAR2_time_stamp=millis();
    
    while(1){
        if(Gas_value_state==END){
            vTaskDelete(PAR1);
            xTaskCreatePinnedToCore(Gas_value_t, "Gas_value", 1024*10, NULL, 1, &PAR1, 0);
        }
        if(POST_Record_state==END){
            vTaskDelete(PAR2);
            if(millis()-PAR2_time_stamp>5*60*1000){
                PAR2_time_stamp=millis();
                xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 2, &PAR2, 0);
            }
        }

        if (Gv > G_DANGER) {
            fsm.trigger(SAFEtoDANGER);
            break;
        }
        if (Gv > G_WARN) {
            fsm.trigger(SAFEtoWARNING);
            break;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
        
    }
    vTaskDelete(PAR1);
    vTaskDelete(PAR2);
}
void on_SAFE_exit() {
    Serial.println("SAFE exit");
}
void on_WARNING_enter(){
    status="WARNING";
    Serial.println(status);
    Flick_LED_freq=5;
    Flick_Buzzer_freq=10;
    Serial.println("WARNING enter");
}
void on_WARNING_body() {
    TaskHandle_t PAR1 = NULL;
    TaskHandle_t PAR2 = NULL;
    int PAR2_time_stamp=millis();
    TaskHandle_t PAR3 = NULL;
    TaskHandle_t PAR4 = NULL;
    
    while(1){
        if(Gas_value_state==END){
            vTaskDelete(PAR1);
            xTaskCreatePinnedToCore(Gas_value_t, "Gas_value", 1024*10, NULL, 1, &PAR1, 0);
        }
        if(POST_Record_state==END){
            vTaskDelete(PAR2);
            if(millis()-PAR2_time_stamp>5*60*1000){
                PAR2_time_stamp=millis();
                xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 2, &PAR2, 0);
            }
        }
        if(Flick_LED_state==END){
            vTaskDelete(PAR3);
            xTaskCreatePinnedToCore(Flick_LED_t, "Flick_LED", 1024*10, NULL, 1, &PAR3, 0);
        }
        if(Flick_Buzzer_state==END){
            vTaskDelete(PAR4);
            xTaskCreatePinnedToCore(Flick_Buzzer_t, "Flick_Buzzer", 1024*10, NULL, 1, &PAR4, 0);
        }

        if (Gv > G_DANGER) {
            fsm.trigger(WARNINGtoDANGER);
            break;
        }
        if(Gv < G_WARN){
            fsm.trigger(WARNINGtoSAFE);
            break;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
        
    }
    vTaskDelete(PAR1);
    vTaskDelete(PAR2);
}
void on_WARNING_exit() {
    Serial.println("WARNING exit");
}
void on_DANGER_enter() {
    Serial.println("DANGER");
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
}
void on_DANGER_body() {
    if (Gas_value() < G_SAFE) {
        fsm.trigger(DANGERtoSAFE);
    }
}
void on_DANGER_exit() {
    Serial.println("DANGER exit");
}



void setup() {
    Serial.begin(115200);
    //can add time transistion too!
    fsm.add_transition(&SAFE, &WARNING, SAFEtoWARNING, NULL);
    fsm.add_transition(&SAFE, &DANGER, SAFEtoDANGER, NULL);
    fsm.add_transition(&WARNING, &DANGER, WARNINGtoDANGER, NULL);
    fsm.add_transition(&WARNING, &SAFE, WARNINGtoSAFE, NULL);
    fsm.add_transition(&DANGER, &SAFE, DANGERtoSAFE, NULL);

}

void loop() {
  // put your main code here, to run repeatedly:
}