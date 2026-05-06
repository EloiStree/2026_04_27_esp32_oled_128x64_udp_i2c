// DONT FORGET TO ADD 
// Adafruit SSD1306
// Adafruit GFX Library
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 4    // GP4
#define SCL_PIN 5    // GP5

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR    0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP udp;

// ====================== WIFI ======================
const char* ssid = "SSD1306_13"; // <- Change with the hotspot you want to create for a student.
const char* ssid_any = "SSD1306"; // <- If you want all the screen to be on the same hotspot
const char* password = "12345678";

#define UDP_PORT 3615

uint8_t data_current[1024];
uint8_t data_previous[1024];

bool received_udp_message = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ====================== DISPLAY INIT ======================
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(800000);  // 800 kHz

  Serial.println("Initializing SSD1306 display...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed");
    while (1) delay(10);
  }

  display.clearDisplay();
  display.display();
  Serial.println("Display ready");

// ====================== WIFI ======================
  Serial.print("Connecting to WiFi...");
  try_reconnection_wifi();
  
  // Serial.println("\nWiFi connected!");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  // === VERY IMPORTANT: Start UDP listener ===
  udp.begin(UDP_PORT);
  // Serial.printf("UDP listener started on port %d\n", UDP_PORT);
}
void try_reconnection_wifi(){
bool connected = false;
  
  while (!connected) {
    // Try connecting to ssid for 5 seconds
    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) {
      showWifiInfo();
      delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    
    WiFi.disconnect();
    delay(500);
    
    // Try connecting to ssid_any for 5 seconds
    WiFi.begin(ssid_any, password);
    startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) {
      showWifiInfo();
      delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    
    WiFi.disconnect();
    delay(500);
  }

}

void showWifiInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println("WiFi NOT Connected");
  
  display.setCursor(0, 12);
  display.print("SSID: ");
  display.println(ssid);
  
  display.setCursor(0, 24);
  display.print("Pass: ");
  display.println(password);
  
  display.setCursor(0, 36);
  display.println("Waiting for connection...");
  
  display.display();
}
int get_four_bytes_little_endian(uint8_t* buffer, int offset) {
  return buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
}
void loop() {
  // === Show WiFi info when not connected ===
  if (WiFi.status() != WL_CONNECTED) {
      showWifiInfo();
      delay(100);
      try_reconnection_wifi();
      return;  
  }

  int packetSize = udp.parsePacket();
  // ====================== Once Connected ======================
  if (received_udp_message == false) {
    // Display IP when connected but no UDP yet
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Waiting for UDP frames...");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.print("Port: ");
    display.println(UDP_PORT);
    display.display();
  }


  if (packetSize == 4 || packetSize == 8 || packetSize == 12 || packetSize == 16) 
  {
    
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
      case 0:   // flush the screen
          display.clearDisplay();
          display.display();
        break;
        
      case 1:   // full screen white
          display.fillScreen(SSD1306_WHITE);
          display.display();
        break;
        
      case 42:  // Display 42 on the screen
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(0, 0);
          display.println("42");
          display.display();
        break;
        
      case 2501: // all pixels random
          for (int i = 0; i < SCREEN_WIDTH; i++) {
            for (int j = 0; j < SCREEN_HEIGHT; j++) {
              display.drawPixel(i, j, random(2) ? SSD1306_WHITE : SSD1306_BLACK);
            }
          }
          display.display();
        break;
    
      default:
        break;
    }
  }

  
  // Forward small packets to Serial1 (Micro:Bit)
  if (packetSize < 256 && packetSize > 0) {
    while (udp.available()) {
      uint8_t byte_received = udp.read();
      Serial1.write(byte_received);
    }
  }

  if (packetSize == 1024) {
    if (received_udp_message == false)
      display.clearDisplay();
    received_udp_message = true;
    udp.read(data_current, 1024);
    // Update display (full frame)
    for (int byte_index = 0; byte_index < 1024; byte_index++) {
      uint8_t byte_new = data_current[byte_index];
      uint8_t byte_old = data_previous[byte_index];

      if (byte_new != byte_old) {
        for (int bit_index = 0; bit_index < 8; bit_index++) {
          int i = (byte_index << 3) | bit_index;
          int col = i % SCREEN_WIDTH;
          int row = i / SCREEN_WIDTH;

          bool pixel = (byte_new >> (7 - bit_index)) & 1;
          display.drawPixel(col, row, pixel ? SSD1306_WHITE : SSD1306_BLACK);
        }
      }
    }
    memcpy(data_previous, data_current, 1024);
    display.display();
  }
  delay(1); // prevent watchdog
}
