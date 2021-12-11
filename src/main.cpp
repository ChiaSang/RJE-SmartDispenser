/**
 * RJE-Smart-Dispenser
 *
 * Date: October 07, 2021
 * Author: Chia Sang
 *
 */

#include <Arduino.h>
#include <HardwareSerial.h>
#include <ArduinoLog.h>
#include <CommandParser.h>
#include "Esp.h"

#define TOUCHPIN0 4
#define TOUCH_THRESHOLD 30

HardwareSerial rSerial(1);
typedef CommandParser<> MyCommandParser;
MyCommandParser parser;

byte openRJE[] = {0xA5, 0x05, 0x16, 0x01, 0xC1, 0xFB};
byte closeRJE[] = {0xA5, 0x05, 0x16, 0x01, 0xC1, 0xFB};

byte keepwarmon[] = {0xA5, 0x05, 0x17, 0x01, 0xC2, 0xFB};
byte keepwarmoff[] = {0xA5, 0x05, 0x17, 0x01, 0xC2, 0xFB};

byte heaton[] = {0xA5, 0x05, 0x18, 0x01, 0xC3, 0xFB};
byte heatoff[] = {0xA5, 0x05, 0x18, 0x01, 0xC3, 0xFB};

byte heatwateron[] = {0xA5, 0x05, 0x19, 0x01, 0xC4, 0xFB};
byte heatwateroff[] = {0xA5, 0x05, 0x19, 0x01, 0xC4, 0xFB};

byte wateron[] = {0xA5, 0x05, 0x1A, 0x01, 0xC5, 0xFB};
byte wateroff[] = {0xA5, 0x05, 0x1A, 0x01, 0xC5, 0xFB};

byte temp50[] = {0xA5, 0x05, 0x1B, 0x32, 0xF7, 0xFB};
byte temp55[] = {0xA5, 0x05, 0x1B, 0x37, 0xFC, 0xFB};
byte temp60[] = {0xA5, 0x05, 0x1B, 0x3C, 0x01, 0xFB};
byte temp65[] = {0xA5, 0x05, 0x1B, 0x41, 0x06, 0xFB};
byte temp70[] = {0xA5, 0x05, 0x1B, 0x46, 0x0B, 0xFB};
byte temp75[] = {0xA5, 0x05, 0x1B, 0x4B, 0x10, 0xFB};
byte temp80[] = {0xA5, 0x05, 0x1B, 0x50, 0x15, 0xFB};
byte temp85[] = {0xA5, 0x05, 0x1B, 0x55, 0x1A, 0xFB};
byte temp90[] = {0xA5, 0x05, 0x1B, 0x5A, 0x1F, 0xFB};
byte temp95[] = {0xA5, 0x05, 0x1B, 0x5F, 0x24, 0xFB};
byte temp100[] = {0xA5, 0x05, 0x1B, 0x64, 0x29, 0xFB};

int currentTemperature = 0;
int settingTemperature = 50;
int deviceSW = 0;
int warmingSW = 0;
int heatingSW = 0;
int hRelaySW = 0;
int wRelaySW = 0;

unsigned int getDownInterval = 100;
unsigned int rInterval = 200;
unsigned int reportInterval = 5000;
unsigned long getDownPreviousMillis = 0;
unsigned long reportPreviousMillis = 0;
unsigned long rPreviousMillis = 0;

// constants won't change. They're used here to set pin numbers:
const int BUTTON_PIN = 0;         // the number of the pushbutton pin
const int LONG_PRESS_TIME = 5000; // 1000 milliseconds

// Variables will change:
int lastState = LOW; // the previous state from the input pin
int currentState;    // the current reading from the input pin
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

char rSerialCMD[32] = "";
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

void longPressAction()
{
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) // button is pressed
    pressedTime = millis();
  else if (lastState == LOW && currentState == HIGH)
  { // button is released
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration > LONG_PRESS_TIME)
    {
      Serial2.print("restore\r");
      Log.warning("restore config!" CR);
    }
  }

  // save the the last state
  lastState = currentState;
}

// ======================================================================
// Get device state by pressing touch button
void device_switch()
{
  if (TOUCH_THRESHOLD > touchRead(4))
  {
  }
}

// ======================================================================
// Print test message
void ticktick()
{
}

// ======================================================================
// Report task
void report_state()
{
  currentTemperature = random(50, 100);
  String report_msg = "properties_changed 2 8 " + String(currentTemperature) + "\r";
  Serial2.print(report_msg);
  Log.notice("send: %s" CR, report_msg.c_str());
}

// ======================================================================
// command parse function
void cmd_ok(MyCommandParser::Argument *args, char *response)
{
  Log.notice("recv: ok" CR);
}

void cmd_none(MyCommandParser::Argument *args, char *response)
{
  Log.verbose("recv: none" CR);
}

void cmd_error(MyCommandParser::Argument *args, char *response)
{
  Log.notice("recv: error" CR);
}

void cmd_get(MyCommandParser::Argument *args, char *response)
{
  Log.notice("recv: get %d %d" CR, (int32_t)args[0].asInt64, (int32_t)args[1].asInt64);

  if ((int32_t)args[0].asInt64 == 2)
  {
    switch ((int32_t)args[1].asInt64)
    {

    case 1:
    {
      Serial2.print("result 2 1 0 0\r");
      break;
    }

    case 2:
    {
      String prefix_reply_msg = "result 2 2 0 ";
      String reply = prefix_reply_msg + String(deviceSW) + "\r";
      Serial2.print(reply);
      break;
    }

    case 3:
    {
      String prefix_reply_msg = "result 2 3 0 ";
      String reply = prefix_reply_msg + String(warmingSW) + "\r";
      Serial2.print(reply);
      break;
    }

    case 4:
    {
      String prefix_reply_msg = "result 2 4 0 ";
      String reply = prefix_reply_msg + String(heatingSW) + "\r";
      Serial2.print(reply);
      break;
    }

    case 5:
    {
      String prefix_reply_msg = "result 2 5 0 ";
      String reply = prefix_reply_msg + String(hRelaySW) + "\r";
      Serial2.print(reply);
      break;
    }

    case 6:
    {
      String prefix_reply_msg = "result 2 6 0 ";
      String reply = prefix_reply_msg + String(wRelaySW) + "\r";
      Serial2.print(reply);
      break;
    }

    case 7:
    {
      String prefix_reply_msg = "result 2 7 0 ";
      String reply = prefix_reply_msg + String(settingTemperature) + "\r";
      Serial2.print(reply);
      break;
    }

    case 8:
    {
      String prefix_reply_msg = "result 2 8 0 ";
      String reply = prefix_reply_msg + String(currentTemperature) + "\r";
      Serial2.print(reply);
      break;
    }

    default:
      break;
    }
  }
}

void cmd_set(MyCommandParser::Argument *args, char *response)
{
  Log.notice("recv: set %d %d %d" CR, (int32_t)args[0].asInt64, (int32_t)args[1].asInt64, (int32_t)args[2].asInt64);

  if ((int32_t)args[0].asInt64 == 2)
  {
    switch ((int32_t)args[1].asInt64)
    {

    case 2:
    {
      deviceSW = (int32_t)args[2].asInt64;
      Log.notice("开关: %d" CR, deviceSW);
      Serial2.print("result 2 2 0\r");
      String prefix_reply_msg = "properties_changed 2 2 ";
      String reply = prefix_reply_msg + String(deviceSW) + "\r";
      Serial2.print(reply);

      if (deviceSW == 1)
      {
        rSerial.write(openRJE, sizeof(openRJE));
      }
      else
      {
        rSerial.write(closeRJE, sizeof(closeRJE));
      }

      break;
    }

    case 3:
    {
      warmingSW = (int32_t)args[2].asInt64;
      Log.notice("保温开关: %d" CR, warmingSW);
      Serial2.print("result 2 3 0\r");
      String prefix_reply_msg = "properties_changed 2 3 ";
      String reply = prefix_reply_msg + String(warmingSW) + "\r";
      Serial2.print(reply);

      if (warmingSW == 1)
      {
        rSerial.write(keepwarmon, sizeof(keepwarmon));
      }
      else
      {
        rSerial.write(keepwarmoff, sizeof(keepwarmoff));
      }

      break;
    }

    case 4:
    {
      heatingSW = (int32_t)args[2].asInt64;
      Log.notice("加热开关: %d" CR, heatingSW);
      Serial2.print("result 2 4 0\r");
      String prefix_reply_msg = "properties_changed 2 4 ";
      String reply = prefix_reply_msg + String(heatingSW) + "\r";
      Serial2.print(reply);

      if (heatingSW == 1)
      {
        rSerial.write(heaton, sizeof(heatwateron));
      }
      else
      {
        rSerial.write(heatoff, sizeof(heatwateroff));
      }

      break;
    }

    case 5:
    {
      hRelaySW = (int32_t)args[2].asInt64;
      Log.notice("热加水: %d" CR, hRelaySW);
      Serial2.print("result 2 5 0\r");
      String prefix_reply_msg = "properties_changed 2 5 ";
      String reply = prefix_reply_msg + String(hRelaySW) + "\r";
      Serial2.print(reply);

      if (hRelaySW == 1)
      {
        rSerial.write(heatwateron, sizeof(heatwateron));
      }
      else
      {
        rSerial.write(heatwateroff, sizeof(heatwateroff));
      }

      break;
    }

    case 6:
    {
      wRelaySW = (int32_t)args[2].asInt64;
      Log.notice("冷加水: %d" CR, wRelaySW);
      Serial2.print("result 2 6 0\r");
      String prefix_reply_msg = "properties_changed 2 6 ";
      String reply = prefix_reply_msg + String(wRelaySW) + "\r";
      Serial2.print(reply);

      if (wRelaySW == 1)
      {
        rSerial.write(wateron, sizeof(wateron));
      }
      else
      {
        rSerial.write(wateroff, sizeof(wateroff));
      }

      break;
    }

    case 7:
    {
      settingTemperature = (int32_t)args[2].asInt64;
      Log.notice("设定温度: %d" CR, settingTemperature);
      Serial2.print("result 2 7 0\r");
      String prefix_reply_msg = "properties_changed 2 7 ";
      String reply = prefix_reply_msg + String(settingTemperature) + "\r";
      Serial2.print(reply);

      if ((int32_t)args[2].asInt64 == 50)
      {
        rSerial.write(temp50, sizeof(temp50));
      }
      else if ((int32_t)args[2].asInt64 == 55)
      {
        rSerial.write(temp55, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 60)
      {
        rSerial.write(temp60, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 65)
      {
        rSerial.write(temp65, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 70)
      {
        rSerial.write(temp70, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 75)
      {
        rSerial.write(temp75, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 80)
      {
        rSerial.write(temp80, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 85)
      {
        rSerial.write(temp85, sizeof(temp55));
      }
      else if ((int32_t)args[2].asInt64 == 90)
      {
        rSerial.write(temp90, sizeof(temp100));
      }
      else if ((int32_t)args[2].asInt64 == 95)
      {
        rSerial.write(temp95, sizeof(temp100));
      }
      else if ((int32_t)args[2].asInt64 == 100)
      {
        rSerial.write(temp100, sizeof(temp100));
      }

      break;
    }

    default:
      break;
    }
  }
}

void cmd_hex(MyCommandParser::Argument *args, char *response)
{
  Log.notice("recv: cmd_hex: %s" CR, args[1].asString);
}

// ======================================================================
// Loop getDown command and receive acknowledge message
void getDown()
{
  if (Serial2.available())
  {
    char line[32];
    size_t lineLength = Serial2.readBytesUntil('\r', line, 31);
    line[lineLength] = '\0';
    char cmd[32];

    if (strlen(line) > 10)
    {
      // 截取down后到命令，重组命令字符串数组
      int cmdlen = strlen(line) - 5;
      strncpy(cmd, line + 5, cmdlen);
      cmd[cmdlen] = '\0';
      // Log.notice("slice: %d - total: %d cpy: %s" CR, cmdlen, strlen(line), cmd);
      char response[MyCommandParser::MAX_RESPONSE_SIZE];
      parser.processCommand(cmd, response);
    }

    if (strlen(line) <= 10)
    {
      char response[MyCommandParser::MAX_RESPONSE_SIZE];
      parser.processCommand(line, response);
    }
  }

  else
  {
    Serial2.print("get_down\r");
    Log.verbose("query: %s" CR, "get_down");
  }
}

// ======================================================================
// parse HEX Command from rSerial
void parseHEXCommand()
{
  char tem[4] = "";
  char ctem[4] = "";
  while (rSerial.available())
  {
    char hex_cmd[2];
    unsigned int hex_num = rSerial.read();
    // sprintf(hex_cmd, "%02X%c", hex_num, ' ');
    sprintf(hex_cmd, "%02X", hex_num);
    strcat(rSerialCMD, hex_cmd);
  }
  strncpy(tem, rSerialCMD + 14, 2);
  strncpy(ctem, rSerialCMD + 16, 2);
  sscanf(tem, "%x", &settingTemperature);
  sscanf(ctem, "%x", &currentTemperature);

  if (rSerialCMD[0] == 'A' && strlen(rSerialCMD) == 24)
  {
    // Log.notice("recv: (hex)%s(%d)" CR, rSerialCMD, strlen(rSerialCMD));
    Log.notice("recv: (hex)%s(%d) | sw(%c) wsw(%c) hsw(%c) sw1(%c) sw2(%c) tem(%d) ctem(%d) err(%c)" CR, rSerialCMD, strlen(rSerialCMD), rSerialCMD[5], rSerialCMD[7], rSerialCMD[9], rSerialCMD[11], rSerialCMD[13], settingTemperature, currentTemperature, rSerialCMD[19]);

    deviceSW = rSerialCMD[5] - '0';
    warmingSW = rSerialCMD[7] - '0';
    heatingSW = rSerialCMD[9] - '0';
    hRelaySW = rSerialCMD[11] - '0';
    wRelaySW = rSerialCMD[13] - '0';

    String report_msg = "properties_changed 2 8 " + String(currentTemperature) + "\r";
    Serial2.print(report_msg);

    memset(rSerialCMD, 0, 32);
  }
  if (rSerialCMD[0] == '5' && strlen(rSerialCMD) == 12)
  {
    Log.notice("recv: (hex)%s(%d)" CR, rSerialCMD, strlen(rSerialCMD));

    String report_msg = "properties_changed 2 8 " + String(currentTemperature) + "\r";
    Serial2.print(report_msg);
    // Log.notice("send: %s" CR, report_msg.c_str());

    memset(rSerialCMD, 0, 32);
  }
  else
  {
    memset(rSerialCMD, 0, 32);
    memset(tem, 0, 4);
    memset(ctem, 0, 4);
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
  rSerial.begin(9600, SERIAL_8N1, 5, 18);

  parser.registerCommand("ok", "", &cmd_ok);
  parser.registerCommand("none", "", &cmd_none);
  parser.registerCommand("error", "", &cmd_error);
  parser.registerCommand("get_properties", "ii", &cmd_get);
  parser.registerCommand("set_properties", "iii", &cmd_set);

  Log.setPrefix(printPrefix); // set prefix similar to NLog
  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Print Logo and device notice
  Serial.println("\n");
  Serial.println("######        # #######");
  Serial.println("#     #       # #      ");
  Serial.println("#     #       # #      ");
  Serial.println("######        # #####  ");
  Serial.println("#   #   #     # #      ");
  Serial.println("#    #  #     # #      ");
  Serial.println("#     #  #####  #######");
  Serial.println("\n");
  Log.notice("Device notice:" CR);
  Log.notice("Chip Model: %s" CR, ESP.getChipModel());
  Log.notice("Revsion: %d" CR, ESP.getChipRevision());
  Log.notice("MAC: %lu" CR, ESP.getEfuseMac());
  Log.notice("Cores: %d" CR, ESP.getChipCores());
  Log.notice("CpuFreqMHz: %u" CR, ESP.getCpuFreqMHz());
  Log.notice("Chip ID: %u" CR, chipId);
  Log.notice("SDK Version: %s" CR, ESP.getSdkVersion());
  // Log.notice("Cycle Count: %u" CR, ESP.getCycleCount());
  // Log.notice("Total Heap Size: %u" CR, ESP.getHeapSize());
  // Log.notice("Free Heap Size: %u" CR, ESP.getFreeHeap());
  // Log.notice("Lowest Level Of Free Heap Since Boot: %u" CR, ESP.getMinFreeHeap());
  // Log.notice("Largest Block Of Heap That Can Be Allocated At Once = %u" CR, ESP.getMaxAllocHeap());
  // Log.notice("Sketch MD5: %u" CR, ESP.getSketchMD5());
  // Log.notice("Sketch Size: %u" CR, ESP.getSketchSize());
  // Log.notice("Sketch Remaining Space: %u" CR, ESP.getFreeSketchSpace());

  // attachInterrupt(digitalPinToInterrupt(0), ticktick, HIGH);
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

  // if (currentMillis - reportPreviousMillis >= reportInterval)
  // {
  //   reportPreviousMillis = currentMillis;
  //   report_state();
  // }

  if (currentMillis - rPreviousMillis >= rInterval)
  {
    rPreviousMillis = currentMillis;
    parseHEXCommand();
  }

  else if (currentMillis - getDownPreviousMillis <= 0)
  {
    getDownPreviousMillis = currentMillis;
  }

  // else if (currentMillis - reportPreviousMillis <= 0)
  // {
  //   reportPreviousMillis = currentMillis;
  // }

  else if (currentMillis - rPreviousMillis <= 0)
  {
    rPreviousMillis = currentMillis;
  }
  longPressAction();
}
