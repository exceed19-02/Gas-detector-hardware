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


void on_Initial_enter() {
    Serial.println("Initial enter");
}
void on_Initial_body() {
    fsm.trigger(InitialtoSAFE);
    Serial.println("Initial body");
}
void on_SAFE_enter() {
    Serial.println("SAFE enter");
    Status = "SAFE";
    POST_Record_instant = true;
    TurnOff_LED();
    TurnOff_Buzzer();
    Window_Commandable = true;
}
void on_SAFE_body() {
    Serial.println("SAFE body");
    while(1){
        if (Gv > G_DANGER) {
            fsm.trigger(SAFEtoWARNING);
        }else if (Gv > G_WARNING) {
            fsm.trigger(SAFEtoDANGER);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
void on_WARNING_enter() {
    Serial.println("WARNING enter");
    Status = "WARNING";
    POST_Record_instant = true;
    Set_LED_freq(1);
    Set_Buzzer_freq(1);
    Window_Commandable = true;
}
void on_WARNING_body() {
    Serial.println("WARNING body");
    while(1){
        if (Gv > G_DANGER) {
            fsm.trigger(WARNINGtoDANGER);
        }else if (Gv < G_SAFE) {
            fsm.trigger(WARNINGtoSAFE);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
void on_DANGER_enter() {
    Serial.println("DANGER enter");
    Status = "DANGER";
    POST_Record_instant = true;
    Set_LED_freq(4);
    Set_Buzzer_freq(4);
    Window_Commandable = false;
}
void on_DANGER_body() {
    Serial.println("DANGER body");
    while(1){
        if (Gv < G_SAFE) {
            fsm.trigger(DANGERtoSAFE);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}


void setup() {
    Serial.begin(115200);
    LED_setup();
    Buzzer_setup();
    Internet_setup();
    Gas_setup();
    Servo_setup();

    fsm.add_transition(&Initial, &SAFE, InitialtoSAFE, NULL);
    fsm.add_transition(&SAFE, &WARNING, SAFEtoWARNING, NULL);
    fsm.add_transition(&SAFE, &DANGER, SAFEtoDANGER, NULL);
    fsm.add_transition(&WARNING, &DANGER, WARNINGtoDANGER, NULL);
    fsm.add_transition(&WARNING, &SAFE, WARNINGtoSAFE, NULL);
    fsm.add_transition(&DANGER, &SAFE, DANGERtoSAFE, NULL);    

}
void loop(){
    fsm.run_machine();
    vTaskDelay(10/portTICK_PERIOD_MS);
}
