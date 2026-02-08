#include "Arduino.h"
#include "WiFi.h"
#include "wifi_user.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "HardwareSerial.h"
#include "PubSubClient.h"
#include "cJSON.h"
#include "lvgl.h"




extern "C" void app_main(){
    initArduino();  //INIT ARDUINO  
   


    Serial.begin(115200);          
   

    
    
    
    //开机动画
    //power_animation();
    
    
    while(1) {
      
        

        vTaskDelay(1000);
    }
}

