#include <Arduino.h>
#include <string>
#include "driver/i2c.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <DNSServer.h>
#include "ESPAsyncWebServer.h"
#include "utilidades.h"
#include "configuracion.h"
#include "esp_log.h"
#include "PortalWeb.h"
#include "SPIFFS.h"
#include "driver/twai.h"  //Two Wire Automovile Interface    CAN BUS
#include <EEPROM.h>
#include "jk_bms_485.h"
#include "tareas.h"
#include <ModbusClientTCPasync.h>
#include <ArduinoJson.h>
#include <AsyncMqttClient.h> 


             
#define ONBOARD_LED_BLUE        GPIO_NUM_2 //2
#define CAN_TX_GPIO             GPIO_NUM_23  //33 pin en prototipo cambiar readme.md de github
#define CAN_RX_GPIO             GPIO_NUM_22  //32

#define VERSION                 "2022_12_16_versionV1.0"



struct METER meter{};
//Estructura de Datos del bms JK
struct JK_bms_battery_info jk_bms_battery_info{};
Config configuracion;


uint8_t bufferStream[512];   
uint8_t bufferReceiver[512];  



DNSServer dnsServer;
AsyncWebServer webserver(80);
AsyncMqttClient mqtt;


//Manejadores de tareas concurrentes
TaskHandle_t  peticionesRS485_handle=NULL;
TaskHandle_t  enviosMODBUS_handle=NULL;
TaskHandle_t  enviosCANBUS_handle=NULL; 
TaskHandle_t  recibirCANBUS_handle=NULL;
TaskHandle_t  capturarSerialPushCola_handle=NULL;
TaskHandle_t  capturarSerial2PushCola_handle=NULL;
TaskHandle_t  parseaSerialToJk_bms_battery_info_handle=NULL;
TaskHandle_t  parpadeo_led_handle=NULL;
TaskHandle_t  reset_configuracion_handle=NULL;
TaskHandle_t  imprimeDatos_taskhandle_handle=NULL;
TaskHandle_t  enviarDatosMqtt_handle=NULL;


//Cola de mensajes entre tareas concurrentes
QueueHandle_t colaPuertoSerie_handle=NULL;
QueueHandle_t colaEscritura_handle=NULL;
QueueHandle_t colaLecturaCAN_handle=NULL;


//Control procesador entre tareas
SemaphoreHandle_t xMutex=NULL;


//SPI

//MODBUS
ModbusClientTCPasync * modbus=NULL;





////////// WATCHDOG FUNCTIONS //////////
const int loopTimeCtl = 0;
hw_timer_t *watchDogTimer = NULL;

void IRAM_ATTR resetModule(){
  Serial.printf("########---------Reboot by Watchdog-------##########\n");
  delay(1000);
  ESP.restart();
}






//CAN BUS
void setupCanbus(){
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  //Filter all other IDs except MSG_ID
 /*
  twai_filter_config_t f_config = {.acceptance_code = 0,
                                             .acceptance_mask = ~(TWAI_STD_ID_MASK << 21),
                                             .single_filter = true};
  */
  //aceptar todo el trafico
  static const twai_filter_config_t f_config={.acceptance_code=0, .acceptance_mask=0xFFFFFFFF, .single_filter=true};
  //Set to NO_ACK mode due to self testing with single module
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NORMAL);
  //configurar
  if(twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK){
    Serial.println("Instalado controlador CANBUS");
    if(twai_start()==ESP_OK){
      Serial.println("Iniciado servicio CANBUS");
      configuracion.dispositivoCAN=true;
    }else{
      Serial.println("No se puede iniciar el servicio CANBUS");
      configuracion.dispositivoCAN=false;
    }
  }else{
    Serial.println("No instalado servicio CANBUS");
    configuracion.dispositivoCAN=false;
  }
  configuracionSalvarEEPROM();
}


void setupModbus(){
  IPAddress ip = {192, 168, 0, 190};          // IP address of modbus server
  uint16_t port = 502;                      // port of modbus server
  // Create a ModbusTCP client instance
  ModbusClientTCPasync MB(ip, port);
  modbus=&MB;
  MB.onDataHandler([](ModbusMessage response, uint32_t token){
      Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", 
          response.getServerID(), response.getFunctionCode(), token, response.size());
      for (auto& byte : response) {
        Serial.printf("%02X ", byte);
      }
      Serial.println("");
  });

  MB.onErrorHandler([](Error error, uint32_t token) {
      // ModbusError wraps the error code and provides a readable error message for it
      ModbusError me(error);
      Serial.printf("Error response: %02X - %s token: %d\n", (int)me, (const char *)me, token);
  });
     
 // MB.connect() //conectar otro server
}

//MQTT
void  setupMqtt(){
  IPAddress ipMqtt(configuracion.ipmqtt[0], configuracion.ipmqtt[1], configuracion.ipmqtt[2], configuracion.ipmqtt[3]);
  mqtt.setServer(ipMqtt, configuracion.portmqtt);
  mqtt.setCredentials(configuracion.usermqtt, configuracion.passmqtt);
  mqtt.connect();
}






void setupTareasYColas(){
  colaPuertoSerie_handle=xQueueCreate(2, sizeof(bufferStream));
  if(colaPuertoSerie_handle==NULL){
    Serial.println("ERROR al crear la cola lectura del puerto serie");
    ESP.restart();
  }
  colaLecturaCAN_handle=xQueueCreate(10, sizeof(twai_message_t));
  if(colaLecturaCAN_handle==NULL){
    Serial.println("ERROR al crear la cola lectura de mensaje CAN");
    ESP.restart();
  }

  xTaskCreate(peticionrs485_task, "peticionrs485", 2048, watchDogTimer, 1, &peticionesRS485_handle);
  Serial.println("Creada tarea request jk485");
 // xTaskCreate(capturarSerialToColaLectura, "caputurarSerial", 2048, watchDogTimer, 1, &capturarSerialPushCola_handle);
 // Serial.println("Creada tarea Serial to cola");
  
  xTaskCreate(capturarSerial2ToColaLectura_task, "caputuraRSerial-2", 2048, watchDogTimer, 1, &capturarSerial2PushCola_handle);
  Serial.println("Creada tarea Serial2 to cola");
  
  xTaskCreate(parseoColaLecturaToJk_bms_battery_info_task, "parseaSerial_JK", 3072, watchDogTimer, 2, &parseaSerialToJk_bms_battery_info_handle);
  Serial.println("Creada tarea parseo bytes to JK-INFO");
  
  xTaskCreate(parpadeoLed_task, "parpadeo_led", 1024, watchDogTimer, 1, &parpadeo_led_handle);
  Serial.println("Creada tarea parpadeo led");
  
  xTaskCreate(resetConfiguracion_task, "resetconfiguracion", 2048, watchDogTimer, 1, &reset_configuracion_handle);
  Serial.println("Creada tarea hard-reset");

  xTaskCreate(envioCAN_task, "envio_can", 5120, watchDogTimer, 1, &enviosCANBUS_handle );
  Serial.println("Creada tarea envio datos CANBUS");
  delay(100);

  xTaskCreate(recibirCAN_task, "recibir_can", 5120, watchDogTimer, 1, &recibirCANBUS_handle);
  Serial.println("Creada tarea recibir datos CANBUS");
  delay(100);
  
  xTaskCreate(enviarDatosMqtt_task, "enviar_mqtt", 3024, NULL, 2, &enviarDatosMqtt_handle);
  Serial.println("Creada tarea enviar datos MQTT");
  delay(100);
}




void wifiModeStation() {
  // Set WIFI module to STA mode
  // WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  // Connect
  
  Serial.printf("Conectando a la WIFI: %s pass: %s\n", configuracion.ssid1, configuracion.pass1);
  WiFi.begin(configuracion.ssid1, configuracion.pass1);
  // Wait
  delay(200);
  int contador=0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(200);
    contador++;
    if(contador%10==0)Serial.print('-');
    if(contador>=100){
      configuracion.wifiConfigured=false;

      configuracionSalvarEEPROM();
      EEPROM.end();
      Serial.println("Imposible conectar con red wifi, reiniciando.");
      for(int i=0; i<6; i++){
        Serial.print(".");
        delay(500);
      }
      ESP.restart();
    }
  }//fin test conexión
  // Connected!
  Serial.printf("Conectado a la red WIFI: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  configuracion.wifiConfigured=true;
  configuracionSalvarEEPROM();
}


void wifiModeAp(){
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  WiFi.softAP(configuracion.hostName);
  IPAddress myIP = WiFi.softAPIP();
  delay(250);
  Serial.printf("Configurado punto de acceso con IP: %s\n", myIP.toString().c_str());
  Serial.printf("Configurado punto de acceso con SSID: %s\n", configuracion.hostName);
  
  if(dnsServer.start(53, "*", myIP)){
    Serial.println("Configurado DNS-Server");
  }else{
    Serial.println("FALLO configuración DNS-Server");
  }
  
  configuracion.wifiConfigured=false;
  
  configuracionSalvarEEPROM();
  
}

void setupPines(){
  //configuracion de pines
  gpio_reset_pin(ONBOARD_LED_BLUE);
  gpio_set_direction(ONBOARD_LED_BLUE, GPIO_MODE_INPUT_OUTPUT);  //si es solo salida al leer el pin devuelve cero siempre
  gpio_set_level(ONBOARD_LED_BLUE, 0);
  
  gpio_reset_pin(GPIO_NUM_0);
  gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);  //boton boot placa devkit

  //pinMode(ONBOARD_LED, OUTPUT);

}




void setup(){
  // Inicialización puertos serie
  Serial.begin(115200);
  Serial2.begin(115200); 

   // Inicialización EEPROM
  if (!EEPROM.begin(sizeof(configuracion))){
    log_printf("[MAIN.CPP]Fallo inicializando EEPROM\nRestarting...\n");
    delay(1000);
    ESP.restart();
  }

  configuracionLeerEEPROM();
  
  if(strcmp(VERSION, configuracion.version) != 0){
    configuracionInicial();
    strcpy(configuracion.version, VERSION); //establecer versión como palabra de control
    configuracionSalvarEEPROM();
    Serial.println("Inicializando el sistema a valores por defecto....");
    Serial.println(String(configuracion.version));
    delay(3000);
    ESP.restart();
  }

  //////////////////// WATCHDOG //////////////////
  // pinMode(loopTimeCtl, INPUT_PULLUP);
  delay(1000);
  watchDogTimer = timerBegin(0, 240, true); // Timer 0, div 240, 80
  timerAttachInterrupt(watchDogTimer, &resetModule, true);
  timerAlarmWrite(watchDogTimer, 30000000, false); // Set time in us (30 seconds)
  timerAlarmEnable(watchDogTimer); // Enable interrupt
  /////////////////////////////////////////////////
  Serial.println("Configurado watchdog");

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    delay(3000);
    ESP.restart();
  }
  Serial.println("Configurado sistema SPIFFS");
  // hardware pines i/o
  setupPines();
  Serial.println("Configurado pines I/O");  
  
  PortalWeb::setupAccessPoint(&webserver, &configuracion, &jk_bms_battery_info);
  Serial.println("Configurado servidor portalweb");


  if (configuracion.wifiConfigured==false){ 
      Serial.println("Establecer Punto de acceso para configurar wifi local");
      wifiModeAp();
  }else{ 
      Serial.println("Configurado acceso a WIFI local");
      wifiModeStation();
  }

  webserver.begin();
  Serial.println("Iniciado servidor web"); 
  delay(250);
  
  

  Serial.println("Iniciando CANBUS...");
  setupCanbus();
  //Creacion de semaforo exclusión y tareas
  xMutex=xSemaphoreCreateMutex();
  if( xMutex != NULL ){
    Serial.println("Creado semaforo exclusión");
    setupTareasYColas();
    delay(300); 
  }
  
  //MQTT
  if (configuracion.wifiConfigured==true){ 
    setupMqtt();
  }
  
}

void loop(){
  
  //////// Watchdog ////////////
  timerWrite(watchDogTimer, 0); // reset timer (feed watchdog)
  /////////////////////////////////////
 
  if(configuracion.wifiConfigured==false){
      dnsServer.processNextRequest();
  }
 //Serial.println("HACER ALGO");  
  delay(200);

}