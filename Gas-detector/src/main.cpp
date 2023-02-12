#include <Arduino.h>
#include <Fsm.h>
#include "somchai.h"
void setup() {
    Serial.begin(115200);
    LED_setup();
    Buzzer_setup();
    Internet_setup();
    Gas_setup();
    Servo_setup();
    POST_Record_instant=true;
    
}

void loop(){
    if(Status=="SAFE"){
        Status="SAFE";
        TurnOff_LED();
        TurnOff_Buzzer();
        Window_Commandable=true;
        if(Gv>=G_DANGER){
            Status="DANGER";
            POST_Record_instant=true;
        }else if(Gv>=G_WARNING){
            Status="WARNING";
            POST_Record_instant=true;
        }
    }
    if(Status=="WARNING"){
        Status="WARNING";
        Set_LED_freq(1);
        Set_Buzzer_freq(1);
        Window_Commandable=true;
        if(Gv<G_WARNING){
            Status="SAFE";
            POST_Record_instant=true;
        }else if(Gv>=G_DANGER){
            Status="DANGER";
            POST_Record_instant=true;
        }
    }
    if(Status=="DANGER"){
        Status="DANGER";
        Set_LED_freq(4);
        Set_Buzzer_freq(4);
        OpenWindow();
        PUT_OpenWindow_request=true;
        Window_Commandable=false;
        if(Gv<G_SAFE){
            Status="SAFE";
            POST_Record_instant=true;
        }
    }
    //Serial.println(Gv);
//
    //vTaskDelay(500);
    //OpenWindow();
    

    //Set_LED_freq(1);
    //Set_Buzzer_freq(1);
    //Serial.println("slow");
    //delay(4000);
    //Set_LED_freq(4);
    //Set_Buzzer_freq(4);
    //Serial.println("fast");
    //delay(4000);
}