
#include "SPI.h"
#include "TFT_eSPI.h"
#include "SpaceMono_Regular50pt7b.h"
#include "SpaceMono_Regular40pt7b.h"
#include "SpaceMono_Regular16pt7b.h"
#include "SpaceMono_Regular10pt7b.h"
// Stock font and GFXFF reference handle
#define GFXFF 1

#define CF_SM50 &SpaceMono_Regular50pt7b
#define CF_SM40 &SpaceMono_Regular40pt7b
#define CF_SM16 &SpaceMono_Regular16pt7b
#define CF_SM10 &SpaceMono_Regular10pt7b


// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

void setup(void) {
    Serial.begin(250000);
    tft.begin();
    tft.setRotation(1);
}

void loop() {
    // Set text datum to middle centre (MC_DATUM)
    tft.setTextDatum(MC_DATUM);
    tft.fillScreen(TFT_BLACK);            // Clear screen
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont(CF_SM50);
    tft.drawString("12:55", 160, 100, GFXFF);
    tft.setFreeFont(CF_SM16);
    tft.drawString("Tue 31 Dec", 160, 240, GFXFF);
    delay(10000);
}

