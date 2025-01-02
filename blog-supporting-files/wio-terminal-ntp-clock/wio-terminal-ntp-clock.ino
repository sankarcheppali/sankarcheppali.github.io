/**
 adopted from https://wiki.seeedstudio.com/Wio-Terminal-RTC/
 **/
#include "TFT_eSPI.h"
#include "rpcWiFi.h"
#include "RTC_SAMD51.h"
#include "DateTime.h"
#include "SpaceMono_Regular50pt7b.h"
#include "SpaceMono_Regular16pt7b.h"
// Stock font and GFXFF reference handle
#define GFXFF 1

#define CF_SM50 &SpaceMono_Regular50pt7b
#define CF_SM16 &SpaceMono_Regular16pt7b

#define NTP_REFRESH_INTR 60 // mins

const char* ntpServer1 = "time.google.com";
long tzOffset = 19800UL;         // set your timezone offset in seconds
unsigned int localPort = 2390;   // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message
unsigned int uptimeMin = 0;

byte packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming and outgoing packets

const char* ssid = "***";
const char* password = "***";
TFT_eSPI tft;
RTC_SAMD51 rtc;
WiFiUDP udp;
int previousMin=0;

char days[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char months[12][4] = { 
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
};

unsigned long getNTPtime() {

  // module returns a unsigned long time valus as secs since Jan 1, 1970
  // unix time or 0 if a problem encounted

  //only send data when connected
  if (WiFi.status() == WL_CONNECTED) {
    //initializes the UDP state
    //This initializes the transfer buffer
    udp.begin(WiFi.localIP(), localPort);

    sendNTPpacket(ntpServer1);  // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);
    if (udp.parsePacket()) {
      Serial.println("udp packet received");
      Serial.println("");
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, extract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // adjust local time
      unsigned long adjustedTime;
      return adjustedTime = epoch + tzOffset;
    } else {
      // were not able to parse the udp packet successfully
      // clear down the udp connection
      udp.stop();
      return 0;  // zero indicates a failure
    }
    // not calling ntp time frequently, stop releases resources
    udp.stop();
  } else {
    // network not connected
    return 0;
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(const char* address) {
  // set all bytes in the buffer to 0
  for (int i = 0; i < NTP_PACKET_SIZE; ++i) {
    packetBuffer[i] = 0;
  }
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123);  //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void setupLCD() {
  tft.begin();
  digitalWrite(LCD_BACKLIGHT, HIGH);  // turn on the backlight
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);                //Black background
  tft.setFreeFont(&FreeSansOblique12pt7b);  //select Free, Sans, Oblique, 12pt.
}

uint8_t updateTimeInRTCFromNTP() {
  unsigned long devicetime = getNTPtime();
  if (devicetime == 0) {
    Serial.println("Failed to get time from network time server.");
    return -1;
  }
  // adjust time using ntp time
  rtc.adjust(DateTime(devicetime));
  char printBuffer[20];
  DateTime now = rtc.now();
  Serial.println("Updated RTC");
  sprintf(printBuffer, "%02d/%02d/%04d %02d:%02d:%02d",
          now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
  Serial.println(printBuffer);
  return 0;
}


void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  setupLCD();
  int dotCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (dotCounter % 10 == 0) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 110);
    }
    tft.print(".");
    dotCounter++;
    WiFi.begin(ssid, password);
    delay(1000);
  }
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) delay(10);  // stop operating
  }

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(100, 110);
  while(updateTimeInRTCFromNTP()){
    if (dotCounter % 10 == 0) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(100, 110);
    }
    tft.print(".");
    dotCounter++;
    Serial.println("failed to update RTC");
    delay(1000);
  }
  Serial.println("init done");
}


void loop() {
  DateTime currentTime = rtc.now();
  int currentMin = currentTime.minute();
  if(currentMin!=previousMin) {
    previousMin = currentMin;
    tft.setTextDatum(MC_DATUM);
    tft.fillScreen(TFT_BLACK);            // Clear screen
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont(CF_SM50);
    char printBuffer[12];
    sprintf(printBuffer, "%02d:%02d", currentTime.hour(), currentTime.minute());
    tft.drawString(printBuffer, 160, 100, GFXFF);
    tft.setFreeFont(CF_SM16);
    sprintf(printBuffer, "%s %02d %s",days[currentTime.dayOfTheWeek()], currentTime.day(), months[currentTime.month()-1]);
    tft.drawString(printBuffer, 160, 240, GFXFF);
    Serial.println("updated LCD");
    uptimeMin++;
    if(uptimeMin % NTP_REFRESH_INTR == 0){
      updateTimeInRTCFromNTP();
    }
  }
  delay(1000);
}
