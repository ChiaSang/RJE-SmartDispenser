/**
 * RJE-Smart-Dispenser
 *
 * Date: October 07, 2021
 * Author: Chia Sang
 *
 */
#include <stdlib.h>
#include <Arduino.h>
#include <ArduinoLog.h>
#include <CmdParser.hpp>
#include "Esp.h"

#define TOUCHPIN0 4
#define TOUCH_THRESHOLD 30

CmdParser cmdParser;

QueueHandle_t queueGetting;
QueueHandle_t queueSetting;

TaskHandle_t uart2_receive_handler = NULL;
TaskHandle_t execute_cmd_handler = NULL;
TaskHandle_t report_state_handler = NULL;
TaskHandle_t parserCMD_handler = NULL;
TaskHandle_t uart2_send_handler = NULL;

boolean deviceState = 0;
boolean loopState = true;

int queueSize = 512;
int runState = 1;
int deviceMode = 2;
int heatTemperature = 80;
int keepTemperature = 30;

int currentTemperature = 26;
int settingTemperature = 95;

String inputString = "";

// void InitialDevice();
// void printLogLevel();
// void printTimestamp();
// void printPrefix();

//---------------------------------------------------------
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
    _logOutput->print("notice ");
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
  // printLogLevel(_logOutput, logLevel);
}
//---------------------------------------------------------

// Parse command and assign tasks corresponding to commands
void parseCMD(String cmd)
{
  // String cmd;
  Log.notice("命令处理" CR);
  // char *cmd;
  for (;;)
  {
    // xQueueReceive(queueGetting, &cmd, portMAX_DELAY);
    char buffer[inputString.length()];
    inputString.toCharArray(buffer, inputString.length());
    Log.notice("接收到的命令为: %s" CR, buffer);
    if (cmdParser.parseCmd(buffer) != CMDPARSER_ERROR)
    {
      // const size_t count = cmdParser.getParamCount();

      // get_properties
      if (cmdParser.equalCommand("get_properties"))
      {
        const char *siid = cmdParser.getCmdParam(2);
        const char *piid = cmdParser.getCmdParam(3);
        if (*siid == '2')
        {
          // Serial.printf("water-dispenser\n");
          switch (*piid)
          {
          case '1':
          { // attribute 1
            // Serial.printf("receive_%s parser get 1\n", cmd.c_str());
            char const *echo_cmd = "result 2 1 0 0\r";
            loopState = true;
            Log.notice("接收处理前" CR);
            Serial.println("接收处理前");
            Serial2.write(echo_cmd);
            Log.notice("接收处理后" CR);
            inputString = "";
            break;
          }
          case '2':
          {
            if (runState)
            {
              Serial2.write("result 2 2 0 1\r");
            }
            else
            {
              Serial2.write("result 2 2 0 0\r");
            }
            loopState = true;
            break;
          }
          case '3':
          {

            char msg[] = "result 2 3 0 ";
            char c[2];
            itoa(runState, c, 10);
            // Serial.println(strcat(msg, strcat(c, "\r")));
            Serial2.write(strcat(msg, strcat(c, "\r")));
            loopState = true;
            break;
          }
          case '4':
          {

            char msg[] = "result 2 4 0 ";
            char c[2];
            itoa(deviceMode, c, 10);
            Serial2.write(strcat(msg, strcat(c, "\r")));
            loopState = true;
            break;
          }
          case '5':
          {

            char msg[] = "result 2 5 0 ";
            char c[4];
            itoa(currentTemperature, c, 10);
            Serial2.write(strcat(msg, strcat(c, "\r")));
            loopState = true;
            break;
          }
          case '6':
          {

            char msg[] = "result 2 6 0 ";
            char c[4];
            itoa(settingTemperature, c, 10);
            Serial2.write(strcat(msg, strcat(c, "\r")));
            loopState = true;
            break;
          }
          default:
            break;
          }
        }
      }

      // set_properties
      else if (cmdParser.equalCommand("set_properties"))
      {
        const char *siid = cmdParser.getCmdParam(2);
        const char *piid = cmdParser.getCmdParam(3);
        const char *val = cmdParser.getCmdParam(4);
        if (*siid == '2')
        {
          switch (*piid)
          {

          case '2':
          {
            runState = *val - '0';
            Serial.printf("设备状态: %d", runState);
            Serial2.write("result 2 2 0\r");
            Serial2.write("properties_changed 2 2 1\r");
            loopState = true;
            break;
          }

          case '4':
          {
            Serial.printf("parser set 4\n");
            break;
          }

          case '6':
          {
            Serial.printf("parser set 6\n");
            break;
          }

          default:
            break;
          }
        }
      }

      else
      {
        Serial.printf(cmdParser.getCommand());
        loopState = true;
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// Get device state by pressing touch button
void device_switch()
{
  if (TOUCH_THRESHOLD > touchRead(4))
  {
    deviceState = !deviceState;
    Log.verbose("deviceState: %T", deviceState);
  }
}

// Print test message
void ticktick()
{
  runState = !runState;
  Serial.printf("deviceState: %d\n", runState);
}

// Report parameter state task
void report_state(void *parameters)
{
  for (;;)
  {
    // settingTemperature = random(25, 101);
    currentTemperature = random(25, 101);
    // String temperature = String(random(50, 100));
    // String msg = "properties_changed 2 5 " + temperature + "\r";
    // char buffer[msg.length()];
    // msg.toCharArray(buffer, msg.length() + 1);
    // Serial2.write(buffer);
    // loopState = true;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Execute command
// void execute_cmd(void *parameters)
// {
//   String cmd;
//   for (;;)
//   {
//     xQueueReceive(queueGetting, &cmd, portMAX_DELAY);
//     // Serial.println(cmd);
//     if (cmd.indexOf("get_properties 2 1") != -1)
//     // if (cmd.equals("get_properties 2 1"))
//     {
//       // Serial.println("echo: " + cmd);
//       Serial.printf("receive_%s", cmd.c_str());
//       Serial2.write("result 2 1 0 0\r");
//       loopState = true;
//     }

//     else if (cmd.indexOf("get_properties 2 2") != -1)
//     {
//       if (deviceState)
//       {
//         Serial.printf("receive_%s", cmd.c_str());
//         Serial.printf("Echo: echo_2_2_on");
//         Serial2.write("result 2 2 0 true\r");
//         loopState = true;
//       }
//       else
//       {
//         Serial.printf("Echo: echo_2_2_off");
//         Serial2.write("result 2 2 0 false\r");
//         loopState = true;
//       }
//     }

//     else if (cmd.indexOf("get_properties 2 3") != -1)
//     {
//       String temperature = String(random(1, 4));
//       String msg = "result 2 3 0 " + temperature + "\r";
//       char buffer[msg.length()];
//       msg.toCharArray(buffer, msg.length() + 1);
//       Serial2.write(buffer);
//       Serial.printf("Echo: echo_2_3_status");
//       loopState = true;
//     }

//     else if (cmd.indexOf("get_properties 2 4") != -1)
//     {
//       String msg = "result 2 4 0 " + (String)runState + "\r";
//       char buffer[msg.length()];
//       msg.toCharArray(buffer, msg.length() + 1);
//       Serial2.write(buffer);
//       Serial.printf("Echo: echo_2_4_status");
//       loopState = true;
//     }

//     else if (cmd.indexOf("get_properties 2 5") != -1)
//     {
//       String temperature = String(random(0, 100));
//       String msg = "result 2 5 0 " + temperature + "\r";
//       char buffer[msg.length()];
//       msg.toCharArray(buffer, msg.length() + 1);
//       Serial2.write(buffer);
//       Serial.printf("Echo: echo_2_5_status");
//       loopState = true;
//     }

//     else if (cmd.indexOf("get_properties 2 6") != -1)
//     {
//       String msg = "result 2 6 0 " + (String)keepTemperature + "\r";
//       char buffer[msg.length()];
//       msg.toCharArray(buffer, msg.length() + 1);
//       Serial2.write(buffer);
//       Serial.printf("Echo: echo_2_6_status");
//       loopState = true;
//     }

//     //------------------------------------------------------
//     // Setting device state
//     else if (cmd.indexOf("down set_properties 2 2 true\r") != -1)
//     {
//       Serial.printf("Echo: 2 2 true");
//       deviceState = true;
//       Serial2.write("result 2 2 0\r");
//       Serial2.write("properties_changed 2 2 true\r");
//       loopState = true;
//       // vTaskResume(uart2_receive_handler);
//     }

//     else if (cmd.indexOf("down set_properties 2 2 false\r") != -1)
//     {
//       Serial.printf("Echo: 2 2 false");
//       deviceState = false;
//       Serial2.write("result 2 2 0\r");
//       Serial2.write("properties_changed 2 2 false\r");
//       loopState = true;
//     }

//     //------------------------------------------------------
//     // Setting mode
//     else if (cmd.equals("down set_properties 2 4 1\r"))
//     {
//       runState = 1;
//       Serial2.write("result 2 4 0\r");
//       Serial2.write("properties_changed 2 4 1\r");
//       loopState = true;
//     }

//     else if (cmd.equals("down set_properties 2 4 2\r"))
//     {
//       runState = 2;
//       Serial2.write("result 2 4 0\r");
//       Serial2.write("properties_changed 2 4 2\r");
//       loopState = true;
//     }

//     else if (cmd.equals("down set_properties 2 4 3\r"))
//     {
//       runState = 3;
//       Serial2.write("result 2 4 0\r");
//       Serial2.write("properties_changed 2 4 3\r");
//       loopState = true;
//     }

//     //------------------------------------------------------
//     // Setting heat temperature
//     else if (cmd.equals("down set_properties 2 6 3\r"))
//     {
//       Serial2.write("result 2 4 0\r");
//       Serial2.write("properties_changed 2 6 3\r");
//       loopState = true;
//     }

//     //------------------------------------------------------
//     // Print unmatched command
//     else
//     {
//       loopState = true;
//     }
//     vTaskDelay(30 / portTICK_PERIOD_MS);
//   }
// }

// Loop getDown command and receive acknowledge message
void uart2_receive(void *parameters)
{
  for (;;)
  {
    String inputString = "";
    if (Serial2.available() > 0)
    {
      while (Serial2.available())
      {
        char inChar = Serial2.read();
        inputString += inChar;
        if (inChar == '\\')
        {
          char buffer[inputString.length()];
          inputString.toCharArray(buffer, inputString.length());
          xQueueSend(queueGetting, &buffer, portMAX_DELAY);
        }
      }
      Log.notice("recv: %s" CR, inputString.c_str());
    }
    // else
    // {
    //   char const *str = "get_down\r";
    //   xQueueSend(queueSetting, &str, portMAX_DELAY);
    // }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void uart2_send(void *parameters)
{
  for (;;)
  {
    char *str;
    xQueueReceive(queueSetting, &str, portMAX_DELAY);
    if (*str)
    {
      Log.notice("transmit: %s" CR, str);
      Serial2.write(str);
    }
    else
    {
      char const *str = "get_down\r";
      xQueueSend(queueSetting, &str, portMAX_DELAY);
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// Initial device configuration
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
  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  queueGetting = xQueueCreate(queueSize, sizeof(String));
  queueSetting = xQueueCreate(queueSize, sizeof(String));

  // Print Logo and device notice
  Serial.println("\n");
  Serial.println("  _____      _ ______ \n");
  Serial.println(" |  __ \\    | |  ____|\n");
  Serial.println(" | |__) |   | | |__   \n");
  Serial.println(" |  _  /_   | |  __|  \n");
  Serial.println(" | | \\ \\ |__| | |____ \n");
  Serial.println(" |_|  \\_\\____/|______|");
  Serial.println("\n");
  //Serial.printf("Device notice:");
  //Serial.printf("Chip Model: %s", ESP.getChipModel());
  //Serial.printf("Revsion: %d", ESP.getChipRevision());
  //Serial.printf("MAC: %lu", ESP.getEfuseMac());
  //Serial.printf("Cores: %d", ESP.getChipCores());
  //Serial.printf("CpuFreqMHz: %u", ESP.getCpuFreqMHz());
  //Serial.printf("Chip ID: %u", chipId);
  //Serial.printf("SDK Version: %s", ESP.getSdkVersion());
  //Serial.printf("Cycle Count: %u", ESP.getCycleCount());
  //Serial.printf("Total Heap Size: %u", ESP.getHeapSize());
  //Serial.printf("Free Heap Size: %u", ESP.getFreeHeap());
  //Serial.printf("Lowest Level Of Free Heap Since Boot: %u", ESP.getMinFreeHeap());
  //Serial.printf("Largest Block Of Heap That Can Be Allocated At Once = %u", ESP.getMaxAllocHeap());
  //Serial.printf("Sketch MD5: %u", ESP.getSketchMD5());
  //Serial.printf("Sketch Size: %u", ESP.getSketchSize());
  //Serial.printf("Sketch Remaining Space: %u", ESP.getFreeSketchSpace());

  attachInterrupt(digitalPinToInterrupt(0), ticktick, HIGH);
  // touchAttachInterrupt(T0, device_switch, TOUCH_THRESHOLD);
}

void getDown()
{
  if (Serial2.available())
  {
    char inChar = Serial2.read();
    inputString += inChar;
    if (inChar == '\\')
    {
      Serial.println(inputString);
      parseCMD(inputString);
      Log.notice("recv: %s" CR, inputString.c_str());
      inputString = "";
    }
  }

  else
  {
    Serial2.write("get_down\r");
    Log.notice("query: %s" CR, "get_down");
  }
}

void setup()
{
  InitialDevice();
  // xTaskCreatePinnedToCore(uart2_send, "uart2_send", 1024, NULL, 1, &uart2_send_handler, 1);
  // xTaskCreatePinnedToCore(uart2_receive, "uart2_receive", 1024, NULL, 1, &uart2_receive_handler, 1);
  // xTaskCreatePinnedToCore(report_state, "report_state", 4096, NULL, 1, &report_state_handler, 1);
  // xTaskCreatePinnedToCore(execute_cmd, "execute_cmd", 4096, NULL, 1, &execute_cmd_handler, 0);
  // xTaskCreatePinnedToCore(parseCMD, "parseCMD", 2048, NULL, 1, &parserCMD_handler, 1);
}

void loop()
{
  getDown();
  delay(10);
}
