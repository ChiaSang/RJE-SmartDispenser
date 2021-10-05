#include <Arduino.h>
#include "Esp.h"

TaskHandle_t uart2_getDown_handle = NULL;
TaskHandle_t uart2_rev_handle = NULL;

String rev = "";
unsigned int timecnt;
String inputString = ""; // 缓存字符串

String echo_msg = "get_properties 2 1";

boolean stringComplete = false; // 是否string已经完成缓存

uint64_t chipid;

// define function

void initalRJE();

void test()
{
  Serial.println("Pressed");
}

void uart2_getDown_task(void *parameters)
{
  for (;;)
  {
    // Serial.printf("[%lu]:run get_down\n", millis());
    Serial2.write("get_down\r");
    vTaskDelay(180 / portTICK_PERIOD_MS);
  }
}

void uart2_rev_task(void *parameters)
{
  for (;;)
  {
    Serial2.write("get_down\r");
    if (Serial2.available())
    {
      while (Serial2.available())
      {
        // 获取新的字符:
        char inChar = (char)Serial2.read();
        // 将它加到inputString中:
        inputString += inChar;
      }
    }
    // Serial.println(inputString);
    // vTaskDelay(5 / portTICK_PERIOD_MS);

    // Judge wether the echo contains commands
    if (inputString.indexOf(echo_msg) != -1)
    {
      Serial2.write("result 2 1 0 0\r");
      // Serial.println("Yes");
    }
    vTaskDelay(80 / portTICK_PERIOD_MS);
  }
}
void initalRJE()
{

  // Set Serial Config

  Serial.begin(115200);
  Serial2.begin(115200);

  // print Logo

  Serial.printf("\n");
  Serial.printf("  _____      _ ______ \n");
  Serial.printf(" |  __ \\    | |  ____|\n");
  Serial.printf(" | |__) |   | | |__   \n");
  Serial.printf(" |  _  /_   | |  __|  \n");
  Serial.printf(" | | \\ \\ |__| | |____ \n");
  Serial.printf(" |_|  \\_\\____/|______|");

  // print chip info

  Serial.printf("\n\nDevice Info\n");
  chipid = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32)); //print High 2 bytes
  Serial.printf("%08X\n", (uint32_t)chipid);                       //print Low 4 bytes.
  Serial.printf("total heap size = %u\n", ESP.getHeapSize());
  Serial.printf("available heap = %u\n", ESP.getFreeHeap());
  Serial.printf("lowest level of free heap since boot = %u\n", ESP.getMinFreeHeap());
  Serial.printf("largest block of heap that can be allocated at once = %u\n", ESP.getMaxAllocHeap());
  Serial.printf("total Psram size = %u\n", ESP.getPsramSize());
  Serial.printf("available Psram = %u\n", ESP.getFreePsram());
  Serial.printf("lowest level of free Psram since boot = %u\n", ESP.getMinFreePsram());
  Serial.printf("largest block of Psram that can be allocated at once = %u\n", ESP.getMinFreePsram());
  Serial.printf("get Chip Revision = %u\n", ESP.getChipRevision());
  Serial.printf("getCpuFreqMHz = %u\n", ESP.getCpuFreqMHz());
  Serial.printf("get Cycle Count = %u\n", ESP.getCycleCount());

  // Set Interrupt

  attachInterrupt(digitalPinToInterrupt(0), test, HIGH);
}

void setup()
{
  initalRJE();
  // xTaskCreate(uart2_getDown_task, "uart2_getDown_task", 1024, NULL, 1, &uart2_getDown_handle);
  xTaskCreate(uart2_rev_task, "uart2_rev_task", 1024, NULL, 1, &uart2_rev_handle);
}

void loop()
{
}