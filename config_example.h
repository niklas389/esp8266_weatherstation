// Network
const char* wifi_ssid           =   "<SSID>";
const char* wifi_password       =   "<Password>";
const char* espName             =   "<esp8266_name>";
const char* friendlyName        =   "<device_name>";

// MQTT Config
const char* mqtt_server         =   "";
const char* mqtt_user           =   "";
const char* mqtt_password       =   "";
int mqtt_port                   =   1883;

// MQTT Topics
const char* topic_data_hassio   =   "test/data";

// Deep Sleep
int min2sleep                   =   10;

// Misc
int battery_measures            =   5;
int battery_full                =   4130;
int battery_offset              =   140;
bool DEBUGGING                  =   false;