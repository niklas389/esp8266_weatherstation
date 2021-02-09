// Idea: https://github.com/flazer/weather-station
#include <ArduinoJson.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

#define FORCE_DEEPSLEEP

// Import settings
#include "config.h"

ESP8266WiFiMulti WiFiMulti;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
BME280I2C bme;

const String sw_version = "2021.6.0";

// SETUP
void setup() {
    if (DEBUGGING == true) {
        Serial.begin(115200);
        splashScreen();
    }

    Wire.begin();
    pinMode(A0, INPUT);

    Serial.println("-----------");
    Serial.print("Searching sensor...");

    int sensorSearchCnt = 0;
        while(!bme.begin()) {
            sensorSearchCnt++;
            Serial.print(".");

            if (sensorSearchCnt > 4) {
                Serial.println();
                Serial.println("    Couldn't find sensor.");
                Serial.println("    Retrying after DS.");
                sensorSearchCnt = 0;
                go_DS(5);
            }
            delay(100);
        }

    Serial.println();

    switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
        Serial.println("Found sensor - Model: BME280");
        break;

    case BME280::ChipModel_BMP280:
        Serial.println("Found sensor - Model: BMP280");
        Serial.println("    >> Humidity not available.");    
        break;

    default:
        Serial.println("Found unsupported sensor.");
        break;
    }

    startWiFi();
}

void loop() {
    runMQTT();
    sendMQTT_v2();

    // delay(150);
    go_DS(min2sleep);
}

void sendMQTT_v2() {
    float temp(NAN), hum(NAN), pres(NAN);
    BME280::Mode_Forced;

    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_hPa);
    bme.read(pres, temp, hum, tempUnit, presUnit);

    char* tempUnitSymb;
    if (tempUnit == 0) { tempUnitSymb = "°C"; } else { tempUnitSymb = "°F"; }

    StaticJsonDocument<200> doc;

    JsonObject obj_0 = doc.createNestedObject("device");
    obj_0["friendlyName"] = friendlyName;
    obj_0["sensorType"] = "BME280";
    obj_0["version"] = sw_version;
    obj_0["debugging"] = DEBUGGING;
    // obj_0["battery"] = batPer;

    JsonObject obj_1 = doc.createNestedObject("values");
    obj_1["temp"] = round(temp*10)/10;
    obj_1["tempUnit"] = tempUnitSymb;
    obj_1["hum"] = round(hum*10)/10;
    obj_1["pres"] = round(pres);
    obj_1["presUnit"] = "hPa";

    // Start a new line
    Serial.println();
    // Generate the prettified JSON and send it to the Serial port.
    serializeJsonPretty(doc, Serial);

    char buffer[256];
    serializeJson(doc, buffer);
    Serial.println();

    if (client.publish(topic_data_hassio, buffer) == true) {
        Serial.println("Sent message");
    } else {
        Serial.println("Error sending message");
    }
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

void go_DS(int minutes) {
#ifdef FORCE_DEEPSLEEP
    Serial.print("Deep Sleep for ");
    Serial.print(minutes);
    Serial.println(" minutes.");

    ESP.deepSleep(minutes * 60 * 1000000);
#endif
}

void splashScreen() {
    for (int i = 0; i <= 5; i++)
        Serial.println();
    Serial.println("#######################################");
    Serial.println("ESP8266 Weather Station");
    Serial.println("Device:");
    Serial.print("Device Name: ");
    Serial.println(espName);
    Serial.print("Client Version: ");
    Serial.println(sw_version);
    Serial.println("Config:");
    Serial.print("Debugging enabled: ");
    Serial.println(DEBUGGING);
    Serial.print("Sensor Model: ");
    Serial.println(bme.chipModel());
    Serial.println("#######################################");
    for (int i = 0; i < 2; i++)
        Serial.println();
}