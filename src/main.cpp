#include <Arduino.h>
#include <CmdBuffer.hpp>
#include "Esp.h"

CmdBuffer<64> cmdBuffer;
TaskHandle_t uart2_getDown_handle = NULL;

// 缓存字符串
String Flag = "\r"; // 缓存字符串

String echo_2_1 = "get_properties 2 1";
String echo_2_2 = "get_properties 2 2";

boolean runState = false;       // 是否string已经完成缓存
boolean stringComplete = false; // 是否string已经完成缓存

uint64_t chipid;

// define function

void initalRJE();

void switch_button()
{
  runState = !runState;
  if (runState)
  {
    Serial.printf("Pressed %d\n", runState);
    Serial2.write("properties_changed 2 2 \"true\"\r");
    Serial2.write("properties_changed 2 1 0 0\r");
  }
  else
  {
    Serial2.write("properties_changed 2 2 \"false\"\r");
    Serial2.write("properties_changed 2 1 0 1\r");
  }
}

void echo_2_1_fault()
{
  // echo device fault info
  Serial.println("echo_2_1_fault");
  Serial2.write("result 2 1 0 0\r");
}

void echo_2_2_on()
{
  // echo device fault info
  Serial.println("echo_2_2_on");
  Serial2.write("result 2 2 0 true\r");
}

void uart2_getDown_task(void *parameters)
{
  for (;;)
  {
    Serial2.write("get_down\r");

    if (Serial2.available())
    {
      String inputString = "";
      while (Serial2.available())
      {
        // 获取新的字符:
        char inChar = (char)Serial2.read();
        Serial.print(inChar);
        // 将它加到inputString中:
        inputString += inChar;
      }
      Serial.println(inputString);
      // Judge wether the echo contains commands
      if (inputString.indexOf(echo_2_1) != -1)
      // if (inputString.equals("get_properties 2 1"))
      {
        echo_2_1_fault();
      }
      if (inputString.indexOf(echo_2_2) != -1)
      {
        echo_2_2_on();
      }
      vTaskDelay(80 / portTICK_PERIOD_MS);
    }
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

  Serial.printf("\n\nDevice Info:\n");
  chipid = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32)); //print High 2 bytes
  Serial.printf("%08X\n", (uint32_t)chipid);                       //print Low 4 bytes.
  Serial.printf("ESP32 Chip model = %s Rev %u\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("ESP32 CpuFreqMHz = %u\n", ESP.getCpuFreqMHz());
  Serial.printf("ESP32 total heap size = %u\n", ESP.getHeapSize());
  Serial.printf("ESP32 available heap = %u\n", ESP.getFreeHeap());
  Serial.printf("ESP32 lowest level of free heap since boot = %u\n", ESP.getMinFreeHeap());
  Serial.printf("ESP32 largest block of heap that can be allocated at once = %u\n", ESP.getMaxAllocHeap());
  Serial.printf("ESP32 total Psram size = %u\n", ESP.getPsramSize());
  Serial.printf("ESP32 available Psram = %u\n", ESP.getFreePsram());
  Serial.printf("ESP32 lowest level of free Psram since boot = %u\n", ESP.getMinFreePsram());
  Serial.printf("ESP32 largest block of Psram that can be allocated at once = %u\n", ESP.getMinFreePsram());
  Serial.printf("ESP32 Cycle Count = %u\n", ESP.getCycleCount());

  // Set Interrupt

  attachInterrupt(digitalPinToInterrupt(0), switch_button, HIGH);
}

void setup()
{
  initalRJE();
  // xTaskCreate(uart2_getDown_task, "uart2_getDown_task", 1024, NULL, 1, &uart2_getDown_handle);
  xTaskCreate(uart2_getDown_task, "uart2_getDown_task", 1024, NULL, 1, &uart2_getDown_handle);
}

void loop()
{
}