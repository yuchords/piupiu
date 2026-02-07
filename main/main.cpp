#include "Arduino.h"
#include "WiFi.h"
#include "wifi_user.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "HardwareSerial.h"
#include "PubSubClient.h"
#include "cJSON.h"
// #include "json_generator.h"
#include "my_dht11.h"
#include "my_mqtt.h"

// HardwareSerial STM(1);
const int resetPin = 1;                    //设置重置按键引脚,用于删除WiFi信息
const int adc_pin = 8;                    //测量电池电压
int connectTimeOut_s = 15;                 //WiFi连接超时时间，单位秒
const int RED_PIN  = 10;
const int BLUE_PIN = 2;
const int GREEN_PIN = 3;




const int humanPin = 6;
//***************下面是 MQTT服务器的配置 后面要换成你的////////////

const char* mqtt_server = "59.110.5.173";  //这里换成mqtt服务器地址
const int mqtt_port = 1883;   //同样端口也要换 TCP1883 SSL 或者 Websocket根据服务器上不同进行更改
const char* mqtt_user = "lin";
const char* mqtt_password = "123456";
// const char *ca_cert = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
// MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
// d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
// QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
// MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
// b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
// 9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
// CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
// nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
// 43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
// T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
// gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
// BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
// TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
// DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
// hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
// 06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
// PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
// YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
// CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
// -----END CERTIFICATE-----
// )EOF";
WiFiClient espClient;
PubSubClient mqtt_client;//如果不是ssl连接 就写espClinet 如果是ssl就写esp_client
WiFiClient tcpClient;

/***************************************************************/

/***********************MQTT发布函数***************************/
struct Light {  //light的所有信息 控制信息
    uint8_t red, green, blue;
    uint8_t function;
    uint8_t brightness;
    uint8_t switches;
    uint8_t battery;
};
; //light的电量
static Light light;  //light结构体存入light信息
SemaphoreHandle_t controlMutex;
void mqtt_publish() {  //发布所有话题
    // xSemaphoreTake(controlMutex, portMAX_DELAY);  //多线程读写变量 互斥
    // mqtt_client.publish("battery", String(light.battery).c_str(),true);
    // xSemaphoreGive(controlMutex);

    cJSON *root;
    char *json_string;

    // Create the root JSON object
    root = cJSON_CreateObject();
    if (root == NULL) {
        printf("Error creating JSON object\n");
        return ;
    }

    // Add timestamp (uint16_t can be represented as a number)
    // uint16_t timestamp_val = 1678886400; // Example timestamp (March 15, 2023 12:00:00 PM UTC)
    // cJSON_AddNumberToObject(root, "timestamp", timestamp_val);

    // Add temperature (float)
    float temp_val = 0 ;
    float humi_val = 0 ;
    bool readPin_val = false;
    if(DHT_DataRead(&temp_val, &humi_val)!=ESP_OK){return;}
    
    cJSON_AddNumberToObject(root, "temp", temp_val);

    cJSON_AddNumberToObject(root, "humi", humi_val);
    int readPin = digitalRead(humanPin);
    if(readPin == 0 ){
        readPin_val = false;
    }else{
        readPin_val = true;
    }
    cJSON_AddBoolToObject(root,"human",readPin_val);

    // Convert the cJSON object to a formatted JSON string
    json_string = cJSON_Print(root);
    if (json_string == NULL) {
        printf("Error creating JSON string\n");
        cJSON_Delete(root);
        return ;
    }

    mqtt_client.publish("environment",json_string);
    Serial.println(json_string);
    // Clean up
    cJSON_Delete(root);
    free(json_string); // Remember to free the string returned by cJSON_Print


}
//////////////////////////////////////////////////////////////////////

void light_ctrl(int color)
{
    digitalWrite(RED_PIN,color&0x01);
    digitalWrite(GREEN_PIN,(color&0x02)>>1);
    digitalWrite(BLUE_PIN,(color&0x04)>>2);
    
}

static uint8_t uart_tx[10];
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if(topic && strcmp(topic, "light_ctrl") == 0) {

        // 创建JSON解析对象
        Serial.println("收到light ctrl主题消息：");
        char data_recv [30];
        memset(data_recv,0,30);
        for(int i = 0 ; i < length ; ++i){
            data_recv[i] = (char)payload[i];
        }
        Serial.println(data_recv);
        cJSON *root = cJSON_ParseWithLength((char*)payload, length);
        if(root == NULL) return;

        // 解析各个字段（带默认值）
        cJSON *item;

        // 亮度处理（0-255）

        item = cJSON_GetObjectItemCaseSensitive(root, "color");
        int colorSelected= item->valueint;
        light_ctrl(colorSelected);
        Serial.print("color set to : ");
        Serial.println(colorSelected);

        cJSON_Delete(root);

    }
}
//连接mqtt函数
void connectMQTT() {
    while(!mqtt_client.connected()) {
        String client_id = "esp32-client-lin" ;
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        //下面进行mqtt服务器的连接
        if (mqtt_client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("Connected to MQTT Server");
            //开始初始化话题 通过订阅和发布想要的话题
            mqtt_client.subscribe("light_ctrl", 0);

        } else {
            //如果连接失败
            Serial.print("Failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");//再次尝试连接
            vTaskDelay(5000);
        }
    }
}


void mqtt_task(void* pvParameters) {
    //CA认证客户端
    mqtt_client.setClient(tcpClient);

    mqtt_client.setServer(mqtt_server, mqtt_port);//设置服务器地址与端口
    mqtt_client.setKeepAlive(60);
    mqtt_client.setCallback(mqttCallback);//设置回调
    //下面开始连接mqtt服务器
    while(!mqtt_client.connected()) {
        //初始化连接mqtt服务器
        Serial.println("尝试初始化连接MQTT服务器");
        connectMQTT();
        vTaskDelay(3000);
    }
    Serial.println("MQTT服务器连接成功");
    //订阅操作 以及 发布操作

    int pub_cnt=0;

    //进行到这里时 mqtt服务器已经连接上了 下面开始循环任务
    while(1) {

        pub_cnt++;
        if(!mqtt_client.connected()) {
            Serial.println("尝试重新连接MQTT服务器");
            connectMQTT(); //如果掉线了就重新连接
            vTaskDelay(3000);
            continue; //如果没有连接到mqtt服务器 就进行读写 就会报错
        }
        //下面是我们要发布的话题 进行发布操作
        if(pub_cnt==10) {
            vTaskDelay(50);

            mqtt_publish(); //调用发布函数 发布一个json格式的 light信息

            pub_cnt=0;//1
            mqtt_client.loop();//这个就是mqtt的循环函数 在这里面mqtt客户端会去订阅话题 更新话题数据

            vTaskDelay(100);
        }
    }
    //STM串口解析任务
}

//创建MQTT 与 STM32串口信息解析任务
static void task_spawn(){
    xTaskCreate(mqtt_task,"MQTT_Client",2048*2,NULL,2,NULL);
}


static void web_task(void* pvParameters){
    while(1) {


        checkDNS_HTTP();                  //检测客户端DNS&HTTP请求，也就是检查配网页面那部分
        checkConnect(true);               //检测网络连接状态，参数true表示如果断开重新连接

        if(WiFi.status()==WL_CONNECTED) {
            Serial.println("WIFI连接成功，初始化其他任务");
            //删除这个web线程 并且调用函数创建MQTT任务 STM32通信解析任务
            task_spawn();



            // return;//这里return就会退出出loop函数
        }
        vTaskDelay(30);
    }
}





extern "C" void app_main(){
    initArduino();  //初始化arduino
    //ceshi
    // light.battery=30; 测试用的
   pinMode(resetPin, INPUT_PULLUP);     //按键上拉输入模式(默认上拉高电平输入,按下时接到低电平)
    pinMode(adc_pin,INPUT);            //设置adc pin为输入
    pinMode(humanPin,INPUT_PULLDOWN);
    pinMode(RED_PIN,OUTPUT);
    pinMode(BLUE_PIN,OUTPUT);
    pinMode(GREEN_PIN,OUTPUT);

    // analogReadResolution(12);             //设置adc分辨率12位
    Serial.begin(115200);                //波特率
    controlMutex = xSemaphoreCreateMutex();// 在setup中初始化互斥锁
    // Serial.println("heelo");

    vTaskDelay(100);
    DHT11_Init();
    LEDinit();                           //LED用于显示WiFi状态
    connectToWiFi(connectTimeOut_s);     //连接wifi，传入的是wifi连接等待时间15s
    if(WiFi.status()!=WL_CONNECTED) {
        //检测到wifi未连接 就创建下面的任务 启动webserver服务器
        xTaskCreate(web_task,"web_server",4096,NULL,1,NULL);
    }else {
        task_spawn();
    }

    while(1) {
        // Serial.println("awfr");
        //按键检测任务 如果长按按键 就清除配网信息重新设置WIFI
        if (!digitalRead(resetPin)) //长按5秒(P0)清除网络配置信息
        {
            vTaskDelay(5000);          //哈哈哈哈，这样不准确
            if (!digitalRead(resetPin))
            {
                Serial.println("\n按键已长按5秒,正在清空网络连保存接信息.");
                restoreWiFi();     //删除保存的wifi信息
                ESP.restart();              //重启复位esp32
                Serial.println("已重启设备.");//有机会读到这里吗？
            }
        }


        vTaskDelay(1000);
    }
}

