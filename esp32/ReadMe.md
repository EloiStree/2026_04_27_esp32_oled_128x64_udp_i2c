- Adafruit SH110X
  - Adafruit_SH1106G
- Adafruit GFX Library


``` cpp

## ADD
## Adafruit SH110X
## Adafruit GFX Library
 
#include <WiFi.h>
#include <WiFiUdp.h>

// ====================== DISPLAY ======================
// Choose ONE of the following libraries:

// Option 1: Adafruit (recommended for GFX compatibility)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Option 2: ThingPulse (very popular for ESP32)
// #include <SH1106Wire.h>   // from ESP8266_OLED_SSD1306 library by ThingPulse

// Pins (change if needed)
#define SDA_PIN 4    // GP4
#define SCL_PIN 5    // GP5

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR    0x3C

// Adafruit_SH110X
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Or for ThingPulse:
// SH1106Wire display(OLED_ADDR, SDA_PIN, SCL_PIN);

WiFiUDP udp;

// ====================== WIFI ======================
const char* ssid = "EloiStreeWifi2G";
const char* password = "11234566";

#define UDP_PORT 3615

uint8_t data_current[1024];
uint8_t data_previous[1024];

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ====================== DISPLAY INIT ======================
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(800000);  // 800 kHz

  Serial.println("Initializing display...");

  if (!display.begin(OLED_ADDR, true)) {
    Serial.println("SH1106 allocation failed");
    while (1) delay(10);
  }

  display.clearDisplay();
  display.display();
  Serial.println("Display ready");

  // ====================== WIFI ======================
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // ====================== UDP ======================
  udp.begin(UDP_PORT);
  Serial.printf("Listening for 128x64 frames on UDP port %d...\n", UDP_PORT);

  memset(data_previous, 0, sizeof(data_previous));
}

void loop() {
  int packetSize = udp.parsePacket();

  if (packetSize == 1024) {
    udp.read(data_current, 1024);

    // Update display (full frame)
    for (int byte_index = 0; byte_index < 1024; byte_index++) {
      uint8_t byte_new = data_current[byte_index];
      uint8_t byte_old = data_previous[byte_index];

      if (byte_new != byte_old) {  // Optional optimization
        for (int bit_index = 0; bit_index < 8; bit_index++) {
          int i = (byte_index << 3) | bit_index;
          int col = i % SCREEN_WIDTH;
          int row = i / SCREEN_WIDTH;

          bool pixel = (byte_new >> (7 - bit_index)) & 1;
          display.drawPixel(col, row, pixel ? SH110X_WHITE : SH110X_BLACK);
        }
      }
    }

    memcpy(data_previous, data_current, 1024);
    display.display();  // Send buffer to screen
  }
  else if (packetSize > 0) {
    Serial.printf("Bad packet size: %d\n", packetSize);
  }

  // Small delay to prevent watchdog issues
  delay(1);
}

```
