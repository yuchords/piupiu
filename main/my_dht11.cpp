#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include <esp_log.h>
#include "esp_timer.h"
#include "esp_rom_sys.h"

const gpio_num_t DHT11_GPIO = GPIO_NUM_19;		// DHT11引脚定义
const static char *TAG = "DHT11";

// 温度 湿度buffer
uint8_t buffer[5];
int64_t phase_duration[3]={0};
int64_t bit_duration_low[40]={0};
int64_t bit_duration_high[40]={0};

// DHT11 初始化引脚，等待1s上电时间
void DHT11_Init()
{
    gpio_config_t cnf={
        .pin_bit_mask =  1ULL<<(int)DHT11_GPIO,
    .mode = GPIO_MODE_OUTPUT_OD,
    .pull_up_en=GPIO_PULLUP_ENABLE,
    //    .pull_down_en = GPIO_PULLDOWN_DISABLE,
      //  .intr_type = GPIO_INTR_DISABLE

    };
    gpio_config(&cnf);
    vTaskDelay(1200/portTICK_PERIOD_MS);
}

/*
timeout单位是us
*/
esp_err_t wait_pin_state(uint32_t timeout, int expected_pin_state)
{
    /*用这段固定时间的代码也可以*/
    esp_rom_delay_us(timeout);
    if(gpio_get_level(DHT11_GPIO)==expected_pin_state)
        return ESP_OK;
    else
        return ESP_FAIL;

    /*建议用这段代码*/
    // int64_t start_time;
    // start_time=esp_timer_get_time();
    // while(esp_timer_get_time()-start_time<=timeout)
    // {
    //     if(gpio_get_level(DHT11_GPIO)==expected_pin_state)
    //         return ESP_OK;
    //     esp_rom_delay_us(1);
    // }
    // return ESP_FAIL;
}

esp_err_t DHT_DataRead(float *temp,float *humi)
{
    int64_t time_since_waiting_start;
    esp_err_t result=ESP_FAIL;
    memset(buffer,0,sizeof(buffer));

    gpio_set_direction(DHT11_GPIO,GPIO_MODE_OUTPUT_OD);
    gpio_set_level(DHT11_GPIO,1);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    gpio_set_level(DHT11_GPIO,0);
    vTaskDelay(25/portTICK_PERIOD_MS);
    gpio_set_level(DHT11_GPIO,1);
    time_since_waiting_start=esp_timer_get_time();
    gpio_set_direction(DHT11_GPIO,GPIO_MODE_INPUT);

    result=wait_pin_state(19,0);  //
    if(result == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Phase A Fail, slave not set LOW.");
        return ESP_FAIL;
    }
    phase_duration[0]=esp_timer_get_time()-time_since_waiting_start;
    time_since_waiting_start=esp_timer_get_time();
    /*等从机拉低总线83us，再拉高87us*/
    result=wait_pin_state(80,1);
        if (result == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Phase B Fail, slave not set HIGH.");
            return ESP_FAIL;
        }
    phase_duration[1]=esp_timer_get_time()-time_since_waiting_start;
    time_since_waiting_start=esp_timer_get_time();
    result = wait_pin_state(80, 0);
        if (result == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Phase C Fail, slave not set LOW to start sending.");
            return ESP_FAIL;
        }
    phase_duration[2]=esp_timer_get_time()-time_since_waiting_start;
    time_since_waiting_start=esp_timer_get_time();

    for(int j=0;j<5;j++)
    {
        for(int i =0;i<8;i++)
        {
            /*数位低电平时间*/
            while(gpio_get_level(DHT11_GPIO)==0)
            {
                esp_rom_delay_us(1);
            }
            bit_duration_low[j*8+i]=esp_timer_get_time()-time_since_waiting_start;
            // bit_duration_high
            /*检测数字高位的时间长度来判断是1或0*/
            time_since_waiting_start=esp_timer_get_time();
            do

            {
                if (gpio_get_level(DHT11_GPIO) == 0)
                {
                    if (esp_timer_get_time() - time_since_waiting_start > 40)
                    {
                        /*数字1*/
                        buffer[j] = buffer[j] | (1U << (7 - i));
                        /*数字0不用处理*/
                    }

                    bit_duration_high[j*8+i]=esp_timer_get_time()-
                    time_since_waiting_start;
                    time_since_waiting_start=esp_timer_get_time();
                    break;
                }
            } while (esp_timer_get_time() - time_since_waiting_start < 74);
        }
    }

    result=wait_pin_state(56,1);
    if (result == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Data is all read.But CAN not set high.");
        return ESP_FAIL;
    }
    memset(phase_duration,0,sizeof(phase_duration));
    memset(bit_duration_low,0,sizeof(bit_duration_low));
    memset(bit_duration_high,0,sizeof(bit_duration_high));
    if(((buffer[0]+buffer[1]+buffer[2]+buffer[3])&0xFF) != buffer[4]) {
        return ESP_FAIL;
    }
    *temp = (float)buffer[2]+(float)buffer[3] / 100.0f;
    *humi = (float)buffer[0]+(float)buffer[1] / 100.0f;


    return ESP_OK;
}






// 主函数
// void app_main(void)
// {
// 	esp_err_t result;
//     uint8_t i,j;
//     DHT11_Init();
//     while(1)
//     {
//         memset(phase_duration,0,sizeof(phase_duration));
//         memset(bit_duration_low,0,sizeof(bit_duration_low));
//         memset(bit_duration_high,0,sizeof(bit_duration_high));
//         result = DataRead();
//         if (result==ESP_OK)
//         {
//             ESP_LOGI(TAG,"Reading data succeed.");
//             if(((buffer[0]+buffer[1]+buffer[2]+buffer[3])&0xFF) != buffer[4])
//                 ESP_LOGE(TAG, "But checksum error.");
//             ESP_LOGI(TAG, "Temperature is:%d.%d, Humidity is:%d.%d", buffer[2], buffer[3], buffer[0],buffer[1]);
//         }
//         ESP_LOGI(TAG,"PhaseA duration is:%lld",phase_duration[0]);
//         ESP_LOGI(TAG,"PhaseB duration is:%lld",phase_duration[1]);
//         ESP_LOGI(TAG,"PhaseC duration is:%lld",phase_duration[2]);
//         ESP_LOGI(TAG,"Bit duration is as follows:");
//         for(j=0;j<5;j++)
//         {
//             for(i=0;i<8;i++)
//             {
//                 printf("%lld,%lld-",bit_duration_low[j*8+i],bit_duration_high[j*8+i]);
//             }
//             printf("\n");
//         }
//     }
// }