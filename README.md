# lcd-20x4-esp32-weather-data

## Usage:
### Fibaro HC3
- import QuickApp
- set requred variable:
  - *sensorIp* - using IP of your local Luftdaten sensor
- optional variables:
  - *language* [en|pl|de] (default: en)
  - *interval* (default: 145)
  - *lcdIp* - ip of your ESP32 with LCD

### ESP32 with LCD and BME280 sensor:
- edit variables in .ino file with your WiFi SSID and password, enter NTP server address
- upload sketch to ESP32
