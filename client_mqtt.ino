// Idea: https://github.com/flazer/weather-station
#include <BME280I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

// Import settings
#include "config.h"

ESP8266WiFiMulti WiFiMulti;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define SERIAL_BAUD 115200
#define FORCE_DEEPSLEEP

BME280I2C bme;

// SETUP
void setup() {
    if (DEBUGGING = true) {
        Serial.begin(SERIAL_BAUD);
        splashScreen();
    }

    Wire.begin();
    pinMode(A0, INPUT);

    Serial.println("-----------");
    Serial.print("Searching sensor...");

    int searchTO = 0;
    while (!bme.begin()){
        searchTO++;
        Serial.print(".");

        if (searchTO > 10) {
            Serial.println("");
            Serial.println("Couldn't find BME sensor! >> Deep Sleep");
            searchTO = 0;
            go_DS(5);
        }
        delay(100);
    }

    switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
        Serial.println("Found BME280 sensor!");   
        break;

    case BME280::ChipModel_BMP280:
        Serial.println("Found BMP280 sensor! - Humidity not available.");
        break;
    
    default:
        Serial.println("Found unsupported sensor.");
        break;
    }
    
    startWiFi();
}

void loop() {
    runMQTT();
    sendMQTTdata();
    
    delay(150);
    go_DS(min2sleep);
}

void sendMQTTdata() {
    float temp(NAN), hum(NAN), pres(NAN);
    int bat(NAN), batRAW(NAN);
    BME280::Mode_Forced;

    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_hPa);
    bme.read(pres, temp, hum, tempUnit, presUnit);

    // measure battery level
    int rawMeasure = 0;
    for (int i = 0; i < battery_measures; i++) {
        rawMeasure += analogRead(A0);
        delay(100);
    }

    batRAW = (rawMeasure / battery_measures);
    bat = batRAW  * 4.837;

    // send data
    client.publish(topic_temperature, String(temp).c_str(), true);
    client.publish(topic_humidity, String(hum).c_str(), true);
    client.publish(topic_pressure, String(int(pres)).c_str(), true);
    client.publish(topic_battery, String(bat).c_str(), true);
    client.publish(topic_batteryRAW, String(batRAW).c_str(), true);
}

/**
 * Establish WiFi-Connection
 * If connection times out device will
 * enter deep_sleep and restarts afterwards.
 */
void startWiFi() {
    Serial.println("");
    Serial.println("-------");
    WiFi.mode(WIFI_STA);
    Serial.println("(re)conneting wifi with following credentials:");
    Serial.print("SSID: ");
    Serial.println(wifi_ssid);
    Serial.print("Key: ");
    Serial.println(wifi_password);
    Serial.print("Device Name: ");
    Serial.println(espName);

    WiFi.hostname(espName);
    WiFiMulti.addAP(wifi_ssid, wifi_password);

    int tryCnt = 0;

    Serial.print("Connecting...");
    while (WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
        tryCnt++;

        if (tryCnt > 25) {
            Serial.println("");
            Serial.println("Couldn't connect to WiFi. >> Deep Sleep");
            go_DS(5);
        }
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP adress: ");
    Serial.println(WiFi.localIP());
    delay(100);
}

// Establish MQTT Connection
void runMQTT() {
    Serial.println("");
    Serial.println("-------");
    Serial.println("Starting MQTT-Client with following credentials:");
    Serial.print("Host: ");
    Serial.println(mqtt_server);
    Serial.print("Port: ");
    Serial.println(mqtt_port);
    Serial.print("User: ");
    Serial.println(mqtt_user);
    Serial.print("Password: ");
    Serial.println(mqtt_password);
    Serial.print("ClientID: ");
    Serial.println(espName);

    client.setServer(mqtt_server, mqtt_port);

    while (!client.connected()) {
        Serial.print("Attempting connection...");

        if (client.connect(espName, mqtt_user, mqtt_password)) {
            Serial.println(" >>Success!");
            client.loop();
        } else {
            Serial.println(" >> Failed!");
            Serial.println("Couldn't connect to MQTT-Server. >> Deep Sleep");
            go_DS(5);
        }
    }
}

void go_DS (int minutes) {
    #ifdef FORCE_DEEPSLEEP
        Serial.print("Deep Sleep for ");
        Serial.print(minutes);
        Serial.println(" minutes.");

        ESP.deepSleep(minutes * 60 * 1000000);
        delay(100);
    #endif
}

void splashScreen() {
    for (int i=0; i<=5; i++) Serial.println();
    Serial.println("#######################################");
    Serial.println("ESP8266 Weather Station");
    Serial.print("Device Name: ");
    Serial.println(espName);
    Serial.print("Client Version: ");
    Serial.println(clientVer);
    Serial.println("#######################################");
    for (int i=0; i<2; i++) Serial.println();
}