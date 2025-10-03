#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// lcd setup address may be 0x3f on some backpacks
LiquidCrystal_I2C lcd(0x27, 16, 2);

// mood thresholds
const int THRESH_HAPPY = 6; // >5 nets = happy
const int THRESH_MEH = 1;   // 1-5 nets = meh
const int HUNGRY_SCANS = 3; // no new ssids after 3 scans = hungry

// cute pwnagotchi-style faces
const char* FACE_HAPPY = "^_^ Yay WiFi! ";
const char* FACE_HAPPY2 = "Found:        ";
const char* FACE_MEH =   "-_- So-so...  ";
const char* FACE_MEH2 =  "Found:        ";
const char* FACE_BORED = "o_o No WiFi!  ";
const char* FACE_BORED2 ="Nothing new...";
const char* FACE_HUNGRY =">_< Hungry!!! ";
const char* FACE_HUNGRY2="Need new WiFi!";

// track seen ssids and hungry state
String seenSSIDs[50]; // max 50 ssids adjust if needed
int ssidCount = 0;
int scansWithoutNew = 0;

// show mood on lcd
void showMood(const char* line0, const char* line1tmpl, int value) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  char buf[17];
  snprintf(buf, sizeof(buf), "%s%d", line1tmpl, value);
  lcd.print(buf);
}

// check if ssid is new
bool isNewSSID(String ssid) {
  for (int i = 0; i < ssidCount; i++) {
    if (seenSSIDs[i] == ssid) return false;
  }
  if (ssidCount < 50) {
    seenSSIDs[ssidCount++] = ssid;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  // lcd init
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Mini Pwnagotchi");
  lcd.setCursor(0, 1);
  lcd.print("Booting...");
  delay(1500);

  // wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("\n=== Mini Pwnagotchi ready ===");
}

void loop() {
  Serial.println("Scanning Wi-Fi...");

  // scan
  int n = WiFi.scanNetworks();
  if (n == WIFI_SCAN_FAILED) {
    Serial.println("Scan failed retrying...");
    n = 0;
  } else if (n < 0) {
    n = 0;
  }

  // check for new ssids
  bool foundNew = false;
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (isNewSSID(ssid)) {
      foundNew = true;
    }
    Serial.printf("  [%d] %s (RSSI: %d dBm)\n", i + 1, ssid.c_str(), WiFi.RSSI(i));
  }

  // update hungry counter
  if (foundNew) {
    scansWithoutNew = 0;
  } else {
    scansWithoutNew++;
  }

  // decide mood
  if (scansWithoutNew >= HUNGRY_SCANS) {
    showMood(FACE_HUNGRY, FACE_HUNGRY2, n);
    Serial.printf("Hungry! Found %d networks no new ones.\n", n);
  } else if (n >= THRESH_HAPPY) {
    showMood(FACE_HAPPY, FACE_HAPPY2, n);
    Serial.printf("Happy! Found %d networks.\n", n);
  } else if (n >= THRESH_MEH) {
    showMood(FACE_MEH, FACE_MEH2, n);
    Serial.printf("Meh... Found %d networks.\n", n);
  } else {
    showMood(FACE_BORED, FACE_BORED2, n);
    Serial.println("Bored no networks.");
  }

  delay(10000); // scan every 10s
}
