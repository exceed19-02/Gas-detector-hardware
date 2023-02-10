#include <Arduino.h>
#include <Fsm.h>
#include "somchai.h"

void on_Initial_enter();
void on_Initial_body();
void on_SAFE_enter();
void on_SAFE_body();
void on_WARNING_enter();
void on_WARNING_body();
void on_DANGER_enter();
void on_DANGER_body();
enum {
    InitialtoSAFE,
    SAFEtoWARNING,
    SAFEtoDANGER,
    WARNINGtoDANGER,
    WARNINGtoSAFE,
    DANGERtoSAFE
};

State Initial(&on_Initial_enter, &on_Initial_body, NULL);
State SAFE(&on_SAFE_enter, &on_SAFE_body, NULL);
State WARNING(&on_WARNING_enter, &on_WARNING_body, NULL);
State DANGER(&on_DANGER_enter, &on_DANGER_body, NULL);
Fsm fsm(&Initial);
String status;

void POST_Record_t(void * param){
    POST_Record(status);
}
void PUT_OpenWindow_true(void * param){
    PUT_OpenWindow(true);
}

void on_Initial_enter() {}
void on_Initial_body() {
    fsm.trigger(InitialtoSAFE);
}
void on_SAFE_enter() {
    status="SAFE";
    Serial.println("SAFE enter");
    xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 1, NULL, 0);
    TurnOffLED();
    TurnOffBuzzer();
}
void on_SAFE_body() {
    TaskHandle_t PAR1 = NULL;
    TaskHandle_t PAR2 = NULL;
    int PAR2_time_stamp=millis();
    TaskHandle_t PAR3 = NULL;
    
    
    while(1){
        if(Gas_value_state==END){
            vTaskDelete(PAR1);
            xTaskCreatePinnedToCore(Gas_value, "Gas_value", 1024*10, NULL, 1, &PAR1, 0);
        }
        if(POST_Record_state==END){
            vTaskDelete(PAR2);
            if(millis()-PAR2_time_stamp>5*60*1000){
                PAR2_time_stamp=millis();
                xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 2, &PAR2, 0);
            }
        }
        if(GET_Window_Command_state==END){
            vTaskDelete(PAR3);
            xTaskCreatePinnedToCore(GET_Window_Command, "GET_Window_Command", 1024*10, NULL, 2, &PAR3, 0);
        }
        if(Gv >= G_DANGER){
            fsm.trigger(SAFEtoDANGER);
            break;
        }
        if(Gv >= G_WARN){
            fsm.trigger(SAFEtoWARNING);
            break;
        }
        delay(50);
    }
}
void on_WARNING_enter() {
    status="WARNING";
    Serial.println("WARNING enter");
    xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 1, NULL, 0);
    Set_LED_freq(5);
    Set_Buzzer_freq(10);
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
            xTaskCreatePinnedToCore(Gas_value, "Gas_value", 1024*10, NULL, 1, &PAR1, 0);
        }
        if(POST_Record_state==END){
            vTaskDelete(PAR2);
            if(millis()-PAR2_time_stamp>5*60*1000){
                PAR2_time_stamp=millis();
                xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 2, &PAR2, 0);
            }
        }
        if(FlashLED_state==END){
            vTaskDelete(PAR3);
            xTaskCreatePinnedToCore(FlashLED, "FlashLED", 1024*10, NULL, 2, &PAR3, 0);
        }
        if(FlashBuzzer_state==END){
            vTaskDelete(PAR4);
            xTaskCreatePinnedToCore(FlashBuzzer, "FlashBuzzer", 1024*10, NULL, 2, &PAR4, 0);
        }

        if(Gv >= G_DANGER){
            fsm.trigger(WARNINGtoDANGER);
            break;
        }
        if(Gv < G_WARN){
            fsm.trigger(WARNINGtoSAFE);
            break;
        }
        delay(50);
    }
}

void on_DANGER_enter() {
    status="DANGER";
    Serial.println("DANGER enter");
    xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 1, NULL, 0);
    Set_LED_freq(20);
    Set_Buzzer_freq(40);
    xTaskCreatePinnedToCore(PUT_OpenWindow_true, "PUT_OpenWindow", 1024*10, NULL, 2, &PAR5, 0);
}
void on_DANGER_body() {
    TaskHandle_t PAR1 = NULL;
    TaskHandle_t PAR2 = NULL;
    int PAR2_time_stamp=millis();
    TaskHandle_t PAR3 = NULL;
    TaskHandle_t PAR4 = NULL;
    TaskHandle_t PAR5 = NULL;
    
    
    while(1){
        if(Gas_value_state==END){
            vTaskDelete(PAR1);
            xTaskCreatePinnedToCore(Gas_value, "Gas_value", 1024*10, NULL, 1, &PAR1, 0);
        }
        if(POST_Record_state==END){
            vTaskDelete(PAR2);
            if(millis()-PAR2_time_stamp>5*60*1000){
                PAR2_time_stamp=millis();
                xTaskCreatePinnedToCore(POST_Record_t, "POST_Record", 1024*10, NULL, 2, &PAR2, 0);
            }
        }
        if(FlashLED_state==END){
            vTaskDelete(PAR3);
            xTaskCreatePinnedToCore(FlashLED, "FlashLED", 1024*10, NULL, 2, &PAR3, 0);
        }
        if(FlashBuzzer_state==END){
            vTaskDelete(PAR4);
            xTaskCreatePinnedToCore(FlashBuzzer, "FlashBuzzer", 1024*10, NULL, 2, &PAR4, 0);
        }
        if(PUT_OpenWindow_state==END){
            vTaskDelete(PAR5);
            xTaskCreatePinnedToCore(PUT_OpenWindow_true, "PUT_OpenWindow", 1024*10, NULL, 2, &PAR5, 0);
        }
        if(Gv < G_SAFE){
            fsm.trigger(DANGERtoSAFE);
            break;
        }
        delay(50);
    }
}

void setup(){
    Serial.begin(115200);
    Serial.println("Start");
    setup_devices();
    fsm.add_transition(&Initial, &SAFE, InitialtoSAFE,NULL);
    fsm.add_transition(&SAFE, &WARNING, SAFEtoWARNING,NULL);
    fsm.add_transition(&SAFE, &DANGER, SAFEtoDANGER,NULL);
    fsm.add_transition(&WARNING, &DANGER, WARNINGtoDANGER,NULL);
    fsm.add_transition(&WARNING, &SAFE, WARNINGtoSAFE,NULL);
    fsm.add_transition(&DANGER, &SAFE, DANGERtoSAFE,NULL);
}
void loop(){
    fsm.run_machine();
    delay(50);
}