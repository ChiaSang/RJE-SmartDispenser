#include <Arduino.h>
#include "Esp.h"

#define TOUCHPIN0 4

#define TOUCH_THRESHOLD 10

TaskHandle_t uart2_getDown_handle = NULL;
TaskHandle_t cmdParser_handler = NULL;

// 缓存字符串
String Flag = "\r"; // 缓存字符串

String echo_2_1 = "get_properties 2 1";
String echo_2_2 = "get_properties 2 2";

boolean runState = false;
boolean stringComplete = false;
boolean deviceState = false;

uint64_t chipid;

// define function

void initalRJE();

void restore()
{
  // Serial2.write("restore\r");
}

void open_device()
{
  deviceState = !deviceState;
  Serial.println(deviceState);
}

void echo_2_1_fault()
{
  // echo device fault info
  Serial.println("echo_2_1_fault_info");
  Serial2.write("result 2 1 0 0\r");
}

void echo_2_2_on()
{
  // echo device fault info
  if (deviceState)
  {
    Serial.println("echo_2_2_on");
    Serial2.write("result 2 2 0 true\r");
  }
  else
  {
    Serial.println("echo_2_2_off");
    Serial2.write("result 2 2 0 false\r");
  }
}

// void cmdParser(void *inputString)
// {
//   if (inputString.indexOf("get_properties 2 1") != -1)
//   {
//     Serial.println("echo_2_1_fault_info");
//     Serial2.write("result 2 1 0 0\r");
//   }
// }

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
        char inChar = (char)Serial2.read();
        inputString += inChar;
      }

      if (!inputString.equals("down none\r"))
      {
        Serial.println(inputString);
        // xTaskCreate(cmdParser, "cmdParser", 1024, (void *)&inputString, 1, &cmdParser_handler);
      }

      //
      //
      //
      //
      //
      //
      // Judge wether the echo contains commands
      if (inputString.indexOf("get_properties 2 1") != -1)
      // if (inputString.equals("get_properties 2 1"))
      {
        Serial.println("echo_2_1_fault_info");
        Serial2.write("result 2 1 0 0\r");
      }

      if (inputString.indexOf("get_properties 2 2") != -1)
      {
        if (deviceState)
        {
          Serial.println("echo_2_2_on");
          Serial2.write("result 2 2 0 true\r");
        }
        else
        {
          Serial.println("echo_2_2_off");
          Serial2.write("result 2 2 0 false\r");
        }
      }

      if (inputString.indexOf("get_properties 2 3") != -1)
      {
        String temperature = String(random(1, 4));
        String msg = "result 2 3 0 " + temperature + "\r";
        char buffer[msg.length()];
        msg.toCharArray(buffer, msg.length() + 1);
        Serial2.write(buffer);
        Serial.println("echo_2_3_status");
      }

      if (inputString.indexOf("get_properties 2 4") != -1)
      {
        String temperature = String(random(1, 4));
        String msg = "result 2 4 0 " + temperature + "\r";
        char buffer[msg.length()];
        msg.toCharArray(buffer, msg.length() + 1);
        Serial2.write(buffer);
        Serial.println("echo_2_4_status");
      }

      if (inputString.indexOf("get_properties 2 5") != -1)
      {
        String temperature = String(random(0, 100));
        String msg = "result 2 5 0 " + temperature + "\r";
        char buffer[msg.length()];
        msg.toCharArray(buffer, msg.length() + 1);
        Serial2.write(buffer);
        Serial.println("echo_2_5_status");
      }

      if (inputString.indexOf("get_properties 2 6") != -1)
      {
        String temperature = String(random(30, 60));
        String msg = "result 2 6 0 " + temperature + "\r";
        char buffer[msg.length()];
        msg.toCharArray(buffer, msg.length() + 1);
        Serial2.write(buffer);
        Serial.println("echo_2_6_status");
      }

      if (inputString.equals("down set_properties 2 2 true\r"))
      {
        deviceState = true;
        Serial2.write("get_down true\r");
        Serial2.write("properties_changed 2 2 true\r");
      }

      if (inputString.equals("down set_properties 2 2 false\r"))
      {
        deviceState = false;
        Serial2.write("result 2 2 0 false\r");
        Serial2.write("properties_changed 2 2 false\r");
      }

      vTaskDelay(100 / portTICK_PERIOD_MS);
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

  attachInterrupt(digitalPinToInterrupt(0), restore, HIGH);
  touchAttachInterrupt(T0, open_device, TOUCH_THRESHOLD);
}

void setup()
{
  initalRJE();
  xTaskCreatePinnedToCore(uart2_getDown_task, "uart2_getDown_task", 1024, NULL, 1, &uart2_getDown_handle, 0);
}

void loop()
{
}