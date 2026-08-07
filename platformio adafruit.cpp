[env:esp32dev]
platform = espressif32@6.7.0
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps = adafruit/Adafruit IO Arduino@^4.3.7

lib_ignore=
    WIFININA
    WiFiNINA_-_AdaFruit_Fork

lib_ldf_mode= chain+