// Network
const char* wifi_ssid           =   "<SSID>";
const char* wifi_password       =   "<Password>";
const char* espName             =   "<esp8266_name>";

// MQTT Config
const char* mqtt_server         =   "";
const char* mqtt_user           =   "";
const char* mqtt_password       =   "";
int mqtt_port                   =   1883;

// MQTT Topics
const char* topic_humidity      =   "ws/test/humidity";
const char* topic_temperature   =   "ws/test/temperature";
const char* topic_pressure      =   "ws/test/pressure";
const char* topic_battery       =   "ws/test/battery";
const char* topic_batteryRAW    =   "ws/test/batteryRAW";

// Deep Sleep
int min2sleep                   =   10;

// Misc
String clientVer                =   "1.2";
int battery_measures            =   5;
bool DEBUGGING                  =   true;