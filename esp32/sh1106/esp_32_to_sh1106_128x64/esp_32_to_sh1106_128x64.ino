// ADD
// Adafruit SH110X
// Adafruit GFX Library
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SDA_PIN 4    // GP4
#define SCL_PIN 5    // GP5

#define RX_PIN 16   // GP16
#define TX_PIN 17   // GP17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR    0x3C

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


WiFiUDP udp;

// ====================== WIFI ======================
const char* ssid = "EloiStreeWifi2G";
const char* password = "";

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

bool received_udp_message =false;

int get_four_bytes_little_endian (uint8_t* buffer, int offset) {
  return buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
}

void loop() {
  int packetSize = udp.parsePacket();

  if (received_udp_message==false) {
    // Display ip on screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Waiting for UDP frames...");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
  }

  if (packetSize == 4 || packetSize == 8 || packetSize == 12 || packetSize == 16) {
    
    received_udp_message = true;
    // IID format
    // Little endian four byte integer
    udp.read(data_current, packetSize);
    int integer_index = 0;
    int integer_value = 0;
    if (packetSize == 4) {
      integer_value = get_four_bytes_little_endian(data_current, 0);
    } else if (packetSize == 8) {
      integer_index = get_four_bytes_little_endian(data_current, 0);
      integer_value = get_four_bytes_little_endian(data_current, 4);
    } else if (packetSize == 12) {
      integer_value = get_four_bytes_little_endian(data_current, 0);
    } else if (packetSize == 16) {
      integer_index = get_four_bytes_little_endian(data_current, 0);
      integer_value = get_four_bytes_little_endian(data_current, 4);
    }
  
    switch (integer_value)
    {
      case 0:
          // flush the screen
          display.clearDisplay();
          display.display();
        break;
      case 1:
          // full screen white
          display.fillScreen(SH110X_WHITE);
          display.display();
        break;
      case 42:
          //Display 42 on the screen
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SH110X_WHITE);
          display.setCursor(0, 0);
          display.println("42");
          display.display();
        break;
      case 2501:
          //all pixels random
          for (int i = 0; i < SCREEN_WIDTH; i++) {
            for (int j = 0; j < SCREEN_HEIGHT; j++) {
              display.drawPixel(i, j, random(2) ? SH110X_WHITE : SH110X_BLACK);
            }
          }
          display.display();
        break;
    
      default:
        break;
    }
  }

  if (packetSize <256 && packetSize > 0) {

    // Send the byte to RX_PIN TX_PIN
    // Bluetooth is a pain in the ass to add to Unity and Godot on multiple platform.
    // We can redirect the ESP32 byte received to the Micro:Bit

    while (udp.available()) {
      uint8_t byte_received = udp.read();
      //Serial.printf("Received byte: %02X\n", byte_received);
      //Serial.write(byte_received); // Echo to serial monitor
      Serial1.write(byte_received); // Send to RX_PIN
    }
  }

  if (packetSize == 1024) {
    received_udp_message = true;
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