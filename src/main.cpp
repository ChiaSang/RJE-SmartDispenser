/**
 * RJE-Smart-Dispenser
 *
 * Date: October 07, 2021
 * Author: Chia Sang
 *
 */

#include <Arduino.h>
#include <ArduinoLog.h>
#include "Esp.h"

#define TOUCHPIN0 4

#define TOUCH_THRESHOLD 20

QueueHandle_t queue;

TaskHandle_t getdown_loop_handler = NULL;
TaskHandle_t execute_cmd_handler = NULL;

String echo_2_1 = "get_properties 2 1";
String echo_2_2 = "get_properties 2 2";

boolean deviceState = false;
boolean loopState = true;

int queueSize = 32;
int runState = 1;
int heatTemperature = 80;
int keepTemperature = 30;

// void InitialDevice();
// void printLogLevel();
// void printTimestamp();
// void printPrefix();

void printTimestamp(Print *_logOutput)
{

  // Division constants
  const unsigned long MSECS_PER_SEC = 1000;
  const unsigned long SECS_PER_MIN = 60;
  const unsigned long SECS_PER_HOUR = 3600;
  const unsigned long SECS_PER_DAY = 86400;

  // Total time
  const unsigned long msecs = millis();
  const unsigned long secs = msecs / MSECS_PER_SEC;

  // Time in components
  const unsigned long MilliSeconds = msecs % MSECS_PER_SEC;
  const unsigned long Seconds = secs % SECS_PER_MIN;
  const unsigned long Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
  const unsigned long Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

  // Time as string
  char timestamp[20];
  sprintf(timestamp, "%02lu:%02lu:%02lu.%03lu ", Hours, Minutes, Seconds, MilliSeconds);
  _logOutput->print(timestamp);
}

void printLogLevel(Print *_logOutput, int logLevel)
{
  /// Show log description based on log level
  switch (logLevel)
  {
  default:
  case 0:
    _logOutput->print("SILENT ");
    break;
  case 1:
    _logOutput->print("FATAL ");
    break;
  case 2:
    _logOutput->print("ERROR ");
    break;
  case 3:
    _logOutput->print("WARNING ");
    break;
  case 4:
    _logOutput->print("INFO ");
    break;
  case 5:
    _logOutput->print("TRACE ");
    break;
  case 6:
    _logOutput->print("VERBOSE ");
    break;
  }
}

void printPrefix(Print *_logOutput, int logLevel)
{
  printTimestamp(_logOutput);
  printLogLevel(_logOutput, logLevel);
}

void device_switch()
{
  if (TOUCH_THRESHOLD == touchRead(T0))
  {
    deviceState = !deviceState;
    Serial.println(deviceState);
  }
}

void execute_cmd(void *parameters)
{
  String cmd;
  for (;;)
  {
    xQueueReceive(queue, &cmd, portMAX_DELAY);
    // Serial.println(cmd);
    if (cmd.indexOf("get_properties 2 1") != -1)
    // if (cmd.equals("get_properties 2 1"))
    {
      // Serial.println("echo: " + cmd);
      Log.info("Rev: %s" CR, cmd.c_str());
      Serial2.write("result 2 1 0 0\r");
      loopState = true;
    }

    else if (cmd.indexOf("get_properties 2 2") != -1)
    {
      if (deviceState)
      {
        Log.info("Rev: %s" CR, cmd.c_str());
        Log.info("Echo: echo_2_2_on" CR);
        Serial2.write("result 2 2 0 true\r");
        loopState = true;
      }
      else
      {
        Log.info("Echo: echo_2_2_off" CR);
        Serial2.write("result 2 2 0 false\r");
        loopState = true;
      }
    }

    else if (cmd.indexOf("get_properties 2 3") != -1)
    {
      String temperature = String(random(1, 4));
      String msg = "result 2 3 0 " + temperature + "\r";
      char buffer[msg.length()];
      msg.toCharArray(buffer, msg.length() + 1);
      Serial2.write(buffer);
      Log.info("Echo: echo_2_3_status" CR);
      loopState = true;
    }

    else if (cmd.indexOf("get_properties 2 4") != -1)
    {
      String msg = "result 2 4 0 " + (String)runState + "\r";
      char buffer[msg.length()];
      msg.toCharArray(buffer, msg.length() + 1);
      Serial2.write(buffer);
      Log.info("Echo: echo_2_4_status" CR);
      loopState = true;
    }

    else if (cmd.indexOf("get_properties 2 5") != -1)
    {
      String temperature = String(random(0, 100));
      String msg = "result 2 5 0 " + temperature + "\r";
      char buffer[msg.length()];
      msg.toCharArray(buffer, msg.length() + 1);
      Serial2.write(buffer);
      Log.info("Echo: echo_2_5_status" CR);
      loopState = true;
    }

    else if (cmd.indexOf("get_properties 2 6") != -1)
    {
      String msg = "result 2 6 0 " + (String)keepTemperature + "\r";
      char buffer[msg.length()];
      msg.toCharArray(buffer, msg.length() + 1);
      Serial2.write(buffer);
      Log.info("Echo: echo_2_6_status" CR);
      loopState = true;
    }

    //------------------------------------------------------
    // Setting device state
    else if (cmd.indexOf("down set_properties 2 2 true\r") != -1)
    {
      Log.info("Echo: 2 2 true" CR);
      deviceState = true;
      Serial2.write("result 2 2 0\r");
      Serial2.write("properties_changed 2 2 true\r");
      loopState = true;
      // vTaskResume(getdown_loop_handler);
    }

    else if (cmd.indexOf("down set_properties 2 2 false\r") != -1)
    {
      Log.info("Echo: 2 2 false" CR);
      deviceState = false;
      Serial2.write("result 2 2 0\r");
      Serial2.write("properties_changed 2 2 false\r");
      loopState = true;
    }

    //------------------------------------------------------
    // Setting mode
    else if (cmd.equals("down set_properties 2 4 1\r"))
    {
      runState = 1;
      Serial2.write("result 2 4 0\r");
      Serial2.write("properties_changed 2 4 1\r");
      loopState = true;
    }

    else if (cmd.equals("down set_properties 2 4 2\r"))
    {
      runState = 2;
      Serial2.write("result 2 4 0\r");
      Serial2.write("properties_changed 2 4 2\r");
      loopState = true;
    }

    else if (cmd.equals("down set_properties 2 4 3\r"))
    {
      runState = 3;
      Serial2.write("result 2 4 0\r");
      Serial2.write("properties_changed 2 4 3\r");
      loopState = true;
    }

    //------------------------------------------------------
    // Setting heat temperature
    else if (cmd.equals("down set_properties 2 6 3\r"))
    {
      Serial2.write("result 2 4 0\r");
      Serial2.write("properties_changed 2 6 3\r");
      loopState = true;
    }

    //------------------------------------------------------
    // Ack command
    // else if (cmd.equals("ok\r"))
    // {
    //   Serial.println("Ack command");
    //   loopState = true;
    // }

    //------------------------------------------------------
    // Print unmatched command
    else
    {
      loopState = true;
    }
    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

void getdown_loop(void *parameters)
{
  for (;;)
  {
    String inputString = "";
    if (Serial2.available() > 0)
    {
      while (Serial2.available())
      {
        char inChar = (char)Serial2.read();
        inputString += inChar;
      }

      if (!inputString.equals("down none\r"))
      {
        // loopState = false;
        // xQueueSend(queue, &inputString, portMAX_DELAY);
        if (inputString.equals("ok\r"))
        {
          Log.info("Rev: %s" CR, inputString.c_str());
          loopState = true;
        }
        else if (inputString.equals("error\r"))
        {
          Log.warning("Rev: %s" CR, inputString.c_str());
          loopState = true;
        }
        else
        {
          loopState = false;
          xQueueSend(queue, &inputString, portMAX_DELAY);
        }
      }
    }
    else
    {
      Serial2.write("get_down\r");
    }
    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

void InitialDevice()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.begin(115200);
  Serial2.begin(115200);

  Log.setPrefix(printPrefix); // set prefix similar to NLog
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.begin(LOG_LEVEL_INFO, &Serial);

  queue = xQueueCreate(queueSize, sizeof(String));

  // print Logo

  Serial.println("\n");
  Serial.println("  _____      _ ______ \n");
  Serial.println(" |  __ \\    | |  ____|\n");
  Serial.println(" | |__) |   | | |__   \n");
  Serial.println(" |  _  /_   | |  __|  \n");
  Serial.println(" | | \\ \\ |__| | |____ \n");
  Serial.println(" |_|  \\_\\____/|______|");
  Serial.println("\n");
  Log.verbose("\n\nDevice Info:\n");
  Log.verbose("ESP32 MAC: %X" CR, ESP.getEfuseMac());
  Serial.printf("ESP32 Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("ESP32 Revsion: %d\n", ESP.getChipRevision());
  Serial.printf("ESP32 Cores: %d\n", ESP.getChipCores());
  Serial.printf("ESP32 CpuFreqMHz: %u\n", ESP.getCpuFreqMHz());
  Serial.printf("ESP32 Chip ID: %u\n", chipId);
  Serial.printf("ESP32 SDK Version: %s\n", ESP.getSdkVersion());
  Serial.printf("ESP32 Cycle Count: %u\n", ESP.getCycleCount());
  Serial.printf("ESP32 Total Heap Size: %u\n", ESP.getHeapSize());
  Serial.printf("ESP32 Free Heap Size: %u\n", ESP.getFreeHeap());
  Serial.printf("ESP32 Lowest Level Of Free Heap Since Boot: %u\n", ESP.getMinFreeHeap());
  Serial.printf("ESP32 Largest Block Of Heap That Can Be Allocated At Once = %u\n", ESP.getMaxAllocHeap());
  Serial.printf("ESP32 Sketch MD5: %u\n", ESP.getSketchMD5());
  Serial.printf("ESP32 Sketch Size: %u\n", ESP.getSketchSize());
  Serial.printf("ESP32 Sketch Remaining Space: %u\n", ESP.getFreeSketchSpace());

  // attachInterrupt(digitalPinToInterrupt(0), restore, HIGH);
  touchAttachInterrupt(T0, device_switch, TOUCH_THRESHOLD);
}

void setup()
{
  InitialDevice();
  // Serial2.write("restore\r");
  xTaskCreatePinnedToCore(getdown_loop, "getdown_loop", 2048, NULL, 1, &getdown_loop_handler, 1);
  xTaskCreatePinnedToCore(execute_cmd, "execute_cmd", 2048, NULL, 1, &execute_cmd_handler, 0);
  // xTaskCreate(execute_cmd, "execute_cmd", 2048, NULL, 1, NULL);
  // xTaskCreate(getdown_loop, "getdown_loop", 2048, NULL, 1, NULL);
}

void loop()
{
  if (loopState)
  {
    vTaskResume(getdown_loop_handler);
  }
  else
  {
    vTaskSuspend(getdown_loop_handler);
  }
}
