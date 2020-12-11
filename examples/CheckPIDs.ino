#include <Wire.h>
#include <SparkfunOBD2UART.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(A0, A1); // Green A0, White A1

COBD obd;

#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include "TouchScreen.h"


// These are the four touchscreen analog pins
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be any digital pin
#define XP 7   // can be any digital pin

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define MINPRESSURE 10
#define MAXPRESSURE 1000

// The display uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);


void testATcommands()
{
    static const char cmds[][6] = {"AT Z", "AT I", "AT H0", "AT RV", "AT DP", "0100", "010C", "0902"};
    char buf[128];

    for (byte i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        const char *cmd = cmds[i];
        tft.print("Sending ");
        tft.println(cmd);
        if (obd.sendCommand(cmd, buf, sizeof(buf))) {
            char *p = strstr(buf, cmd);
            if (p)
                p += strlen(cmd);
            else
                p = buf;
            while (*p == '\r') p++;
            while (*p) {
                tft.write(*p);
                if (*p == '\r' && *(p + 1) != '\r')
                    tft.write('\n');
                p++;
            }
        } else {
            tft.println("Timed out!");
        }
        delay(1000);
    }
}

void checkPIDs()
{  
  static const char cmds[][6] = {"0100", "0120", "0140", "0160", "0180", "01A0"};
    char buf[128];

    for (byte i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        const char *cmd = cmds[i];
        tft.print("Sending ");
        tft.println(cmd);
        if (obd.sendCommand(cmd, buf, sizeof(buf))) {
            char *p = strstr(buf, cmd);
            if (p)
                p += strlen(cmd);
            else
                p = buf;
            while (*p == '\r') p++;
            while (*p) {
                tft.write(*p);
                if (*p == '\r' && *(p + 1) != '\r')
                    tft.write('\n');
                p++;
            }
        } else {
            tft.println("Timed out!");
        }
        delay(1000);
    }
}

void setup() {
  mySerial.begin(115200);
  while (!mySerial);

  delay(500);
  
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);
  tft.setTextSize(1);

  delay(250);

  tft.println("Connecting...");
  
  for (;;) {
    delay(10000);
    byte version = obd.begin();
    tft.print("Sparkfun OBD-II Adapter ");
    if (version > 0 && version < 254) {
      tft.println("detected");
      tft.print("OBD firmware version ");
      tft.print(version / 10);
      tft.print('.');
      tft.println(version % 10);
      break;
    } else {
      tft.print("not detected... ");
      tft.print(version / 10);
      tft.print('.');
      tft.println(version % 10);
    }
  }

  testATcommands();
  
  tft.println("Initializing...");
  int up = obd.init(PROTO_SAE_VPW);
  tft.print("Sent initialize command. Waiting 5 seconds... ");
  delay(5000);
  tft.print("passed. Stage: ");
  tft.println(up);

  char buf[64];
  if (obd.getVIN(buf, sizeof(buf))) {
      tft.print("VIN:");
      tft.println(buf);
  } else {
      tft.println("No VIN");
  }
  
  uint16_t codes[6];
  byte dtcCount = obd.readDTC(codes, 6);
  if (dtcCount == 0) {
    tft.println("No DTC"); 
  } else {
    tft.print(dtcCount); 
    tft.print(" DTC:");
    for (byte n = 0; n < dtcCount; n++) {
      tft.print(' ');
      tft.print(codes[n], HEX);
    }
  }

  delay(2000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.println("Checking avalible PIDs");
  delay(250);

  int value;
  tft.print('[');
  tft.print(millis());
  tft.print(']');
  tft.print("RPM=");
  if (obd.readPID(PID_RPM, value)) {
    tft.print(value);
  }
  tft.println();

  static const byte pids[]= {0x00, 0x20, 0x40, 0x60, 0x80, 0xA0};
  int values[sizeof(pids)];
  // send a query to OBD adapter for specified OBD-II pid
  //if (obd.readPID(pids, sizeof(pids), values) == sizeof(pids)) {
  obd.readPID(pids, sizeof(pids), values) == sizeof(pids);
    for (byte i = 0; i < sizeof(pids); i++) {
      tft.print((int)pids[i] | 0x100, HEX);
      tft.print('=');
      tft.print(values[i]);
      tft.println();
    }
  /*} else {
    tft.println("Failed to read PIDs!");
  }*/

  tft.println("Done reading PIDs");

  delay(10000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.println("Checking avalible PIDs");
  delay(250);

  checkPIDs();
}

void loop()
{
  delay(60000);
}
