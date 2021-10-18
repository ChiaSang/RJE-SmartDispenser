/**
 * RJE-Smart-Dispenser
 *
 * Date: October 07, 2021
 * Author: Chia Sang
 *
 */

#include <Arduino.h>
#include <ArduinoLog.h>
#include <CmdParser.hpp>
#include "Esp.h"

#define TOUCHPIN0 4
#define TOUCH_THRESHOLD 30

CmdParser cmdParser;

boolean deviceState = 0;

int runState = 1;
int deviceMode = 2;
int currentTemperature = 26;
int settingTemperature = 95;

unsigned int getDownInterval = 100;
unsigned int reportInterval = 10000;
unsigned long getDownPreviousMillis = 0;
unsigned long reportPreviousMillis = 0;

// ======================================================================
// Log config
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

// ======================================================================
// Get device state by pressing touch button
void device_switch()
{
  if (TOUCH_THRESHOLD > touchRead(4))
  {
    deviceState = !deviceState;
    Log.verbose("deviceState: %T", deviceState);
  }
}

// ======================================================================
// Print test message
void ticktick()
{
  runState = !runState;
  Log.notice("deviceState: %d" CR, runState);
}

// ======================================================================
// Report parameter state task
void report_state()
{
  // settingTemperature = random(25, 101);
  currentTemperature = random(25, 101);
  String temperature = String(random(50, 100));
  String msg = "properties_changed 2 5 " + temperature + "\r";
  char buffer[msg.length()];
  msg.toCharArray(buffer, msg.length() + 1);
  Serial2.write(buffer);
}

// ======================================================================
// Parse command and assign tasks corresponding to commands
void executeCMD(const char *cmd)
{
  if (cmdParser.parseCmd((char *)cmd) != CMDPARSER_ERROR)
  {
    // const size_t count = cmdParser.getParamCount();
    const char *siid = cmdParser.getCmdParam(1);
    const char *piid = cmdParser.getCmdParam(2);

    Log.notice(F("%s %s %s" CR), cmdParser.getCommand(), siid, piid);

    if (cmdParser.equalCommand("get_properties"))
    {
      if (*siid == '2')
      {
        switch (*piid)
        {

        case '1':
        {
          Serial2.write("result 2 1 0 0\r");
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
          break;
        }

        case '3':
        {

          // char msg[] = "result 2 3 0 ";
          // char c[2];
          // itoa(runState, c, 10);
          // // Serial.println(strcat(msg, strcat(c, "\r")));
          // Serial2.write(strcat(msg, strcat(c, "\r")));

          Serial2.write("result 2 3 0 1\r");
          break;
        }

        case '4':
        {

          // char msg[] = "result 2 4 0 ";
          // char c[2];
          // itoa(deviceMode, c, 10);
          // Serial2.write(strcat(msg, strcat(c, "\r")));

          Serial2.write("result 2 4 0 1\r");
          break;
        }

        case '5':
        {

          // char msg[] = "result 2 5 0 ";
          // char c[4];
          // itoa(currentTemperature, c, 10);
          // Serial2.write(strcat(msg, strcat(c, "\r")));

          Serial2.write("result 2 5 0 45\r");
          break;
        }

        case '6':
        {

          // char msg[] = "result 2 6 0 ";
          // char c[4];
          // itoa(settingTemperature, c, 10);
          // Serial2.write(strcat(msg, strcat(c, "\r")));

          Serial2.write("result 2 6 0 68\r");
          break;
        }
        default:
          break;
        }
      }
    }

    else if (cmdParser.equalCommand("set_properties"))
    {
      const char *val = cmdParser.getCmdParam(3);
      if (*siid == '2')
      {
        switch (*piid)
        {

        case '2':
        {
          runState = *val - '0';
          Log.notice("设备状态: %d" CR, runState);
          Serial2.write("result 2 2 0\r");
          Serial2.write("properties_changed 2 2 1\r");
          break;
        }

        case '4':
        {
          deviceMode = *val - '0';
          Log.notice("设备模式: %d" CR, deviceMode);
          Serial2.write("result 2 4 0\r");
          Serial2.write("properties_changed 2 2 2\r");
          break;
        }

        case '6':
        {
          settingTemperature = atoi(val);
          Log.notice("设定温度: %d" CR, settingTemperature);
          Serial2.write("result 2 6 0\r");
          Serial2.write("properties_changed 2 6 49\r");
          break;
        }

        default:
          break;
        }
      }
    }

    else
    {
      Log.verbose("Command: %s No Action" CR, cmd);
    }
  }
}

// ======================================================================
// Loop getDown command and receive acknowledge message
void getDown()
{
  String inputString = "";
  if (Serial2.available())
  {
    while (Serial2.available())
    {
      char inChar = (char)Serial2.read();
      if (inChar != '\\')
      {
        inputString += inChar;
      }
    }
    // cmd = &inputString[0];
    inputString.trim();
    String gcmd = inputString.substring(5);
    if (gcmd != "none")
    {
      // parseCMD(gcmd);
      executeCMD(inputString.substring(5).c_str());
      Log.verbose("recv_p: %s" CR, inputString.substring(5).c_str());
    }
    else
    {
      Log.verbose("recv: %s" CR, inputString.c_str());
    }
    // char buffer[inputString.length()];
    // inputString.toCharArray(buffer, inputString.length());
  }
  else
  {
    Serial2.write("get_down\r");
    Log.verbose("query: %s" CR, "get_down");
  }
}

// ======================================================================
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

  // queueGetting = xQueueCreate(queueSize, sizeof(String));
  // queueSetting = xQueueCreate(queueSize, sizeof(String));

  // Print Logo and device notice
  Serial.println("\n");
  Serial.println("  _____      _ ______ \n");
  Serial.println(" |  __ \\    | |  ____|\n");
  Serial.println(" | |__) |   | | |__   \n");
  Serial.println(" |  _  /_   | |  __|  \n");
  Serial.println(" | | \\ \\ |__| | |____ \n");
  Serial.println(" |_|  \\_\\____/|______|");
  Serial.println("\n");
  // Serial.printf("Device notice:");
  // Serial.printf("Chip Model: %s", ESP.getChipModel());
  // Serial.printf("Revsion: %d", ESP.getChipRevision());
  // Serial.printf("MAC: %lu", ESP.getEfuseMac());
  // Serial.printf("Cores: %d", ESP.getChipCores());
  // Serial.printf("CpuFreqMHz: %u", ESP.getCpuFreqMHz());
  // Serial.printf("Chip ID: %u", chipId);
  // Serial.printf("SDK Version: %s", ESP.getSdkVersion());
  // Serial.printf("Cycle Count: %u", ESP.getCycleCount());
  // Serial.printf("Total Heap Size: %u", ESP.getHeapSize());
  // Serial.printf("Free Heap Size: %u", ESP.getFreeHeap());
  // Serial.printf("Lowest Level Of Free Heap Since Boot: %u", ESP.getMinFreeHeap());
  // Serial.printf("Largest Block Of Heap That Can Be Allocated At Once = %u", ESP.getMaxAllocHeap());
  // Serial.printf("Sketch MD5: %u", ESP.getSketchMD5());
  // Serial.printf("Sketch Size: %u", ESP.getSketchSize());
  // Serial.printf("Sketch Remaining Space: %u", ESP.getFreeSketchSpace());

  Log.notice("Device notice:" CR);
  Log.notice("Chip Model: %s" CR, ESP.getChipModel());
  Log.notice("Revsion: %d" CR, ESP.getChipRevision());
  Log.notice("MAC: %lu" CR, ESP.getEfuseMac());
  Log.notice("Cores: %d" CR, ESP.getChipCores());
  Log.notice("CpuFreqMHz: %u" CR, ESP.getCpuFreqMHz());
  Log.notice("Chip ID: %u" CR, chipId);
  Log.notice("SDK Version: %s" CR, ESP.getSdkVersion());
  Log.notice("Cycle Count: %u" CR, ESP.getCycleCount());
  Log.notice("Total Heap Size: %u" CR, ESP.getHeapSize());
  Log.notice("Free Heap Size: %u" CR, ESP.getFreeHeap());
  Log.notice("Lowest Level Of Free Heap Since Boot: %u" CR, ESP.getMinFreeHeap());
  Log.notice("Largest Block Of Heap That Can Be Allocated At Once = %u" CR, ESP.getMaxAllocHeap());
  Log.notice("Sketch MD5: %u" CR, ESP.getSketchMD5());
  Log.notice("Sketch Size: %u" CR, ESP.getSketchSize());
  Log.notice("Sketch Remaining Space: %u" CR, ESP.getFreeSketchSpace());

  attachInterrupt(digitalPinToInterrupt(0), ticktick, HIGH);
  // touchAttachInterrupt(T0, device_switch, TOUCH_THRESHOLD);
}

void setup()
{
  InitialDevice();
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - getDownPreviousMillis >= getDownInterval)
  {
    getDownPreviousMillis = currentMillis;
    getDown();
  }

  if (currentMillis - reportPreviousMillis >= reportInterval)
  {
    reportPreviousMillis = currentMillis;
    report_state();
  }
  else if (currentMillis - getDownPreviousMillis <= 0)
  {
    getDownPreviousMillis = currentMillis;
  }
  else if (currentMillis - reportPreviousMillis <= 0)
  {
    reportPreviousMillis = currentMillis;
  }
}
