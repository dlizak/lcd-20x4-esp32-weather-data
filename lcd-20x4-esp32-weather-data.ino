#include <WiFi.h>
#include <WebServer.h>
#include <ESPDateTime.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <string_asukiaaa.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

byte CharTemperature[8] = {0b00100, 0b11010, 0b01010, 0b11010, 0b01010, 0b11111, 0b11111, 0b01110};
byte CharHumidity[8] = {0b00100, 0b01010, 0b01010, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000};
byte CharPressure[8] = {0b00100, 0b00100, 0b00100, 0b10101, 0b01110, 0b00100, 0b10001, 0b01110};
byte CharInside[8] = {0b00000, 0b00001, 0b10010, 0b10100, 0b11000, 0b11110, 0b00000, 0b00000};
byte CharOutside[8] = {0b00000, 0b01111, 0b00011, 0b00101, 0b01001, 0b10000, 0b00000, 0b00000};

char *months[] = {"STY", "LUT", "MAR", "KWI", "MAJ", "CZE", "LIP", "SIE", "WRZ", "PAZ", "LIS", "GRU"};
char *weekDays[] = {"niedz", "pon", "wt", "sr", "czw", "pt", "sob"};

unsigned long ms = millis();
unsigned long msLocalData = millis();
unsigned long msWeatherData = millis();

long dataUpdateIndex = 0;
String temperature = "";
String humidity = "";
String pressure = "";
String aqi = "";
String localTemperature = "";
String localHumidity = "";

Adafruit_BME280 bme; // I2C

WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(100);

  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  delay(100);

  server.on("/display", updateWeatherData);
  server.begin();
  Serial.println("HTTP server started");

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, CharTemperature);
  lcd.createChar(1, CharHumidity);
  lcd.createChar(2, CharPressure);
  lcd.createChar(3, CharInside);
  lcd.createChar(4, CharOutside);
  lcd.setCursor(0,1);
  lcd.print("    Startuje...     ");

  setupDateTime();
}

void loop() {
  server.handleClient();
  updateLocalData();
  updateDisplay();
  
  if (millis() - ms > 5000) {
    ms = millis();
    if (!DateTime.isTimeValid()) {
      Serial.println("Failed to get time from server, retry.");
      DateTime.begin();
    }
  }
}

void setupDateTime() {
  DateTime.setServer("192.168.1.1");
  DateTime.setTimeZone(1);
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
}

void updateLocalData() {
  if (millis() - msLocalData > 10000) {
    msLocalData = millis();
    localTemperature = String(bme.readTemperature());
    localHumidity = String(bme.readHumidity());
  }
}

void updateWeatherData() {
  msWeatherData = millis();
  temperature = server.arg("BME280_temperature");
  humidity = server.arg("BME280_humidity");
  pressure = server.arg("BME280_pressure");
  aqi = server.arg("aqi");
  server.send(200, "text/html");
}

void updateDisplay() {
  time_t t = DateTime.now();
  DateTimeParts timeParts = DateTime.getParts();
  const char* dateFormat = t % 2 == 0 ? "%H %M" : "%H:%M";
  String date_str = string_asukiaaa::padEnd((String)weekDays[timeParts.getWeekDay()] + ", " + (String)timeParts.getMonthDay() + " " + (String)months[timeParts.getMonth()], 12, ' ');
  String temperature_str = string_asukiaaa::padEnd(" " + (temperature != "" ? roundAndStrip(temperature) + (char)223 + "C" : "n/a"), 9, ' ');
  String localTemperature_str = string_asukiaaa::padEnd(" " + roundAndStrip(localTemperature) + (char)223 + "C", 9, ' ');
  String humidity_str = string_asukiaaa::padEnd(" " + (humidity != "" ? roundAndStrip(humidity) + "%" : "n/a"), 7, ' ');
  String localHumidity_str = string_asukiaaa::padEnd(" " + roundAndStrip(localHumidity) + "%", 7, ' ');
  String aqi_str = string_asukiaaa::padEnd("AQI " + (aqi != "" ? aqi : "n/a"), 10, ' ');
  String pressure_str = string_asukiaaa::padEnd((pressure != "" ? roundAndStrip(pressure) + " hPa" : "n/a"), 10, ' ');
  const boolean communicationError = millis() - msWeatherData > 5 * 60 * 1000;

  lcd.setCursor(0,0);
  lcd.print(" " + DateFormatter::format(dateFormat, t) + "  " + date_str);

  if (communicationError) {
    lcd.setCursor(0,1);
    lcd.print("      Problem       ");
    lcd.setCursor(0,2);
    lcd.print("   z komunikacja    ");
  } else {
    if (temperature != "" && humidity != "" & aqi != "") {
      lcd.setCursor(0,1);
      lcd.print(aqi_str);
      lcd.print(pressure_str);
      lcd.setCursor(0,2);
      lcd.write(byte(4));
      lcd.print(" ");
      lcd.write(byte(0));
      lcd.print(temperature_str);
      lcd.write(byte(1));
      lcd.print(humidity_str);
    } else {
      lcd.setCursor(0,1);
      lcd.print("   Czekam na dane   ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
    }
  }

  lcd.setCursor(0,3);
  lcd.write(byte(3));
  lcd.print(" ");
  lcd.write(byte(0));
  lcd.print(localTemperature_str);
  lcd.write(byte(1));
  lcd.print(localHumidity_str);
}

String roundAndStrip(String value) {
  String tempValue = String(round(value.toFloat() * 10) / 10);
  return tempValue.substring(0, tempValue.length() - 1);
}
