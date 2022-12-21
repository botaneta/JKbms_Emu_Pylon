#ifndef _PORTALWEB_
#define _PORTALWEB_

#include <Arduino.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <EEPROM.h>
#include "configuracion.h"
#include "jk_bms_485.h"
#include "driver/twai.h"





class PortalWeb {
  public:
    static const char *_configtag;
    
    static void setupAccessPoint(AsyncWebServer * webserver,  Config  * config, JK_bms_battery_info * jk_bms);
    
    
    static void loadConfigFromEEPROM();
    static void saveConfigIntoEEPROM();
    static bool checksumEEPROM();
    static uint8_t create_checksum_EEPROM();
    static void factoryReset();
    static void sendSuccess(AsyncWebServerRequest *request);
    static void sendFailure(AsyncWebServerRequest *request);
    
    static void handleNotFound(AsyncWebServerRequest *request);
    static void handleRoot(AsyncWebServerRequest *request);
    static void salvarWiFi(AsyncWebServerRequest *request);
    static void handleCommand(AsyncWebServerRequest *request);
    static void portalCautivo(AsyncWebServerRequest *request);
    static void paginaPrincipal(AsyncWebServerRequest *request);
    static void estadoActualBateria(AsyncWebServerRequest *request);

   // static void salvarAjustes(AsyncWebServerRequest *request);
    
    static void salvarIntegracion(AsyncWebServerRequest *request);
    static void salvarrs485(AsyncWebServerRequest *request);
    static void salvarmodbusinversor(AsyncWebServerRequest *request);
    static void salvarcanbus(AsyncWebServerRequest *request);
    static void salvarserialdebug(AsyncWebServerRequest *request);
    static void salvarmqtt(AsyncWebServerRequest *request);
    static void salvarinfluxdb(AsyncWebServerRequest *request);
    static void salvarpylonHV(AsyncWebServerRequest *request);
    static void salvarrampadescarga(AsyncWebServerRequest *request);
    static void salvarrampacarga(AsyncWebServerRequest *request);
    static void salvarlimitesSOC(AsyncWebServerRequest *request);
    static void salvarNivelSocBajo(AsyncWebServerRequest *request);
    static void salvarcalibracionSOC(AsyncWebServerRequest *request);
    
    
    
    static String procesarAjustes(const String &var);
    static String procesarComandos(const String &var);
    static String procesarSetupwifi(const String &var);
    static String procesarPrincipal(const String &var);
    static String procesarReglas(const String &var);
    static String procesarAcercade(const String &var);
    
    
  
    

  private:
    
    static  JK_bms_battery_info * _jk_bms;
    static  Config * _config;
    static  String networks;
    static  AsyncWebServer  *_server;
   
    
    
};
    

    

#endif