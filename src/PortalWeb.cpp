
#include "PortalWeb.h"
#include "configuracion.h"
#include "SPIFFS.h"
#include "utilidades.h"
#include <EEPROM.h>
#include <Preferences.h>
#include <ArduinoJson.h> 



AsyncWebServer      * PortalWeb::_server;
Config              * PortalWeb::_config;
JK_bms_battery_info * PortalWeb::_jk_bms;







void PortalWeb::loadConfigFromEEPROM(){
    configuracionLeerEEPROM();
   
}



void PortalWeb::saveConfigIntoEEPROM(){
    configuracionSalvarEEPROM();
}

bool PortalWeb::checksumEEPROM(){
    //MyStruct s;
    //TODO 
    // direccion del checsum en la eeprom a leer y salvar

    return false;
}


uint8_t PortalWeb::create_checksum_EEPROM(){
    uint16_t sizeConfig=sizeof(*_config);
    uint8_t checksum=0;
    uint8_t * buffer = new uint8_t[sizeConfig];
    memcpy(buffer, _config, sizeConfig);

    for(int i=0; i<sizeConfig; i++){
        checksum=(uint8_t)checksum + buffer[i];
    }
    return checksum;
}









void PortalWeb::paginaPrincipal(AsyncWebServerRequest *request){
    request->send(SPIFFS, "/principal.html", "text/html", false, procesarPrincipal);
}





void PortalWeb::handleRoot(AsyncWebServerRequest *request){
    if(_config->comunicarSerialDebug )Serial.printf("PeticiónWeb: %s\n", request->url() );
    if(_config->wifiConfigured==false){  
        Serial.println("Redirigiendo a página configuración wifi");    
        request->redirect("/setupWifi.html");
    }else{
        paginaPrincipal(request);
    }
}

void PortalWeb::handleNotFound(AsyncWebServerRequest *request){ 
    uint8_t tipo=request->method();
    size_t nparam=request->params();
    String peticion="";
    switch(tipo){
        case WebRequestMethod::HTTP_GET: peticion="GET"; break;
        case WebRequestMethod::HTTP_POST: peticion="POST"; break;
        default: peticion="Otro tipo"; break;
    }
    Serial.printf("server.notfound triggered: %s method:%s  numero de parametros:%d\n", request->url().c_str(), peticion, nparam);
    //Serial.println(request->url());       //This gives some insight into whatever was being requested   
    request->redirect("/");
}


void PortalWeb::salvarWiFi(AsyncWebServerRequest *request){
    Serial.println("Obtener argumentos y grabar");
    strcpy(_config->ssid1, request->urlDecode(request->arg("wifiConfigureds")).c_str());
    strcpy(_config->pass1, request->urlDecode(request->arg("password")).c_str());
    _config->wifiConfigured = true;
    saveConfigIntoEEPROM();
    Serial.println("DATA SAVED!!!!, RESTARTING!!!!");
    request->redirect("/");
    EEPROM.end();
    delay(500); // Only to be able to redirect client
    ESP.restart();  
}

void PortalWeb::handleCommand(AsyncWebServerRequest *request){
    String comando=request->urlDecode(request->arg("command"));
    Serial.print("Comando: ");
    Serial.println(comando);
    if(comando != NULL){
       //evaluar comando y hacer algo
    }
    request->redirect("/");
    delay(500); // Only to be able to redirect client
    
}

void PortalWeb::sendSuccess(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  StaticJsonDocument<100> doc;
  doc["success"] = true;
  serializeJson(doc, *response);
  request->send(response);
}

void PortalWeb::sendFailure(AsyncWebServerRequest *request)
{
  request->send(500, "text/plain", "Failed");
}




void PortalWeb::salvarcanbus(AsyncWebServerRequest * request){
    _config->comunicarCAN=false;
    if(request->hasParam("check-enable-can", true)){  //method POST true
        AsyncWebParameter * p=request->getParam("check-enable-can", true);
        if(p->value() =="1"){
            _config->comunicarCAN=true;
        }
    }
    saveConfigIntoEEPROM();
    request->redirect("/ajustes");
}

void PortalWeb::salvarmodbusinversor(AsyncWebServerRequest * request){
     _config->comunicarMODBUS=false;
     
    if(request->hasParam("ipmodbus", true)  &&  request->hasParam("portmodbus", true)){
        String iptexto=request->getParam("ipmodbus", true)->value();
        uint8_t ipmodbus[4]={};
        sscanf(iptexto.c_str(), "%hu.%hu.%hu.%hu", 
                        &_config->ipmodbus[0], &_config->ipmodbus[1], 
                        &_config->ipmodbus[2], &_config->ipmodbus[3]);       
        _config->portmodbus=request->getParam("portmodbus", true)->value().toInt();
    }
    if(request->hasParam("check-enable-modbus", true)){
        AsyncWebParameter * p=request->getParam("check-enable-modbus", true);
        if(p->value() =="1"){
            _config->comunicarMODBUS=true;
            
        }
    }   
    saveConfigIntoEEPROM();
    request->redirect("/ajustes");
}

void PortalWeb::salvarinfluxdb(AsyncWebServerRequest * request){
    _config->comunicarINFLUXDB=false;
    if(request->hasParam("check-enable-influx", true)){
        if(request->getParam("check-enable-influx", true)->value() =="1"){
            _config->comunicarINFLUXDB=true;
        }
    }
    if(request->hasParam("ipinfluxdb", true) && request->hasParam("portinfluxdb", true)){
        String iptexto=request->getParam("ipinfluxdb", true)->value();
        uint8_t ipmodbus[4]={};
        sscanf(iptexto.c_str(), "%hu.%hu.%hu.%hu", 
                        &_config->ipmqtt[0], &_config->ipmqtt[1], 
                        &_config->ipmqtt[2], &_config->ipmqtt[3]);       
        _config->portmqtt=request->getParam("portmqtt", true)->value().toInt();
    }
    if(request->hasParam("databaseinfluxdb", true)){
        String text= request->getParam("databaseinfluxdb", true)->value();
        text.toCharArray(_config->databaseinfluxdb, sizeof(_config->databaseinfluxdb)/sizeof(char));
    }
    if(request->hasParam("userinfluxdb", true)){
        String text= request->getParam("userinfluxdb", true)->value();
        text.toCharArray(_config->userinfluxdb, sizeof(_config->userinfluxdb)/sizeof(char));
    }
    if(request->hasParam("passinfluxdb", true)){
        String text= request->getParam("passinfluxdb", true)->value();
        text.toCharArray(_config->passinfludb, sizeof(_config->passinfludb)/sizeof(char));
    }
    saveConfigIntoEEPROM();
    request->redirect("/ajustes");
}

void PortalWeb::salvarmqtt(AsyncWebServerRequest * request){
    _config->comunicarMQTT=false;
    if(request->hasParam("check-enable-mqtt", true)){
        if(request->getParam("check-enable-mqtt", true)->value() =="1"){
            _config->comunicarMQTT=true;
        }
    }
    if(request->hasParam("ipmqtt", true) && request->hasParam("portmqtt", true)){
        String iptexto=request->getParam("ipmqtt", true)->value();
        uint8_t ipmodbus[4]={};
        sscanf(iptexto.c_str(), "%hu.%hu.%hu.%hu", 
                        &_config->ipmqtt[0], &_config->ipmqtt[1], 
                        &_config->ipmqtt[2], &_config->ipmqtt[3]);       
        _config->portmqtt=request->getParam("portmqtt", true)->value().toInt();
    }
    if(request->hasParam("topicmqtt", true)){
        strcpy(_config->topicmqtt, request->getParam("topicmqtt", true)->value().c_str());
    }
    if(request->hasParam("usermqtt", true)){
        strcpy(_config->usermqtt, request->getParam("usermqtt", true)->value().c_str());
    }
    if(request->hasParam("passmqtt", true)){
        strcpy(_config->passmqtt, request->getParam("passmqtt", true)->value().c_str());
    }
    saveConfigIntoEEPROM();
    request->redirect("/ajustes");   
}

void PortalWeb::salvarrs485(AsyncWebServerRequest * request){
    _config->comunicarJKrs485=false;
    if(request->hasParam("check-enable-jkrs485", true)){
        
        if(request->getParam("check-enable-jkrs485", true)->value() =="1"){
            _config->comunicarJKrs485=true;
           
        }
    }
    
    saveConfigIntoEEPROM();
    //request->send(304, "text/plain", "respuesta de salvar rs485");
    request->redirect("/ajustes");
}

void PortalWeb::salvarserialdebug(AsyncWebServerRequest * request){
    _config->comunicarSerialDebug=false;
    _config->comunicarSerialDebug1=false;
    _config->comunicarSerialDebug2=false;
    if(request->hasParam("check-enable-serial", true)){
        if(request->getParam("check-enable-serial", true)->value() =="1"){
            _config->comunicarSerialDebug=true;
        }
    }
    if(request->hasParam("check-enable-serial1", true)){
        if(request->getParam("check-enable-serial1", true)->value() =="1"){
            _config->comunicarSerialDebug1=true;
        }
    }
    if(request->hasParam("check-enable-serial2", true)){
        if(request->getParam("check-enable-serial2", true)->value() =="1"){
            _config->comunicarSerialDebug2=true;
        }
    }
    saveConfigIntoEEPROM();
    request->redirect("/ajustes");
}

void PortalWeb::salvarpylonHV(AsyncWebServerRequest *request){
    _config->pylontechHV=false;
    if(request->hasParam("check-enable-pylonHV", true)){
        if(request->getParam("check-enable-pylonHV", true)->value() =="1"){
            _config->pylontechHV=true;
        }
    }
    saveConfigIntoEEPROM();
    request->redirect("/ajustes");
}

void PortalWeb::salvarrampadescarga(AsyncWebServerRequest *request){
    if(request->hasParam("corriente20soc", true)){
        _config->bateria.intensidad_descarga.soc_20=request->getParam("corriente20soc", true)->value().toInt();
    }
    if(request->hasParam("corriente15soc", true)){
        _config->bateria.intensidad_descarga.soc_15=request->getParam("corriente15soc", true)->value().toInt();
    }
    if(request->hasParam("corriente10soc", true)){
        _config->bateria.intensidad_descarga.soc_10=request->getParam("corriente10soc", true)->value().toInt();
    }
    if(request->hasParam("corriente05soc", true)){
        _config->bateria.intensidad_descarga.soc_05=request->getParam("corriente05soc", true)->value().toInt();
    }
    saveConfigIntoEEPROM();
    request->redirect("/reglas");
}

void PortalWeb::salvarrampacarga(AsyncWebServerRequest *request){
    if(request->hasParam("corriente80soc", true)){
        _config->bateria.intensidad_carga.soc_80=request->getParam("corriente80soc", true)->value().toInt();
    }
    if(request->hasParam("corriente85soc", true)){
        _config->bateria.intensidad_carga.soc_85=request->getParam("corriente85soc", true)->value().toInt();
    }
    if(request->hasParam("corriente90soc", true)){
        _config->bateria.intensidad_carga.soc_90=request->getParam("corriente90soc", true)->value().toInt();
    }
    if(request->hasParam("corriente95soc", true)){
        _config->bateria.intensidad_carga.soc_95=request->getParam("corriente95soc", true)->value().toInt();
    }
    saveConfigIntoEEPROM();
    request->redirect("/reglas");
}

void PortalWeb::salvarlimitesSOC(AsyncWebServerRequest *request){
    if(request->hasParam("stopcargasoc", true)){
        _config->bateria.soc_max_stop_carga=request->getParam("stopcargasoc", true)->value().toInt();
    }
    if(request->hasParam("restartcargasoc", true)){
        _config->bateria.soc_min_restart_carga=request->getParam("restartcargasoc", true)->value().toInt();
    }
    if(request->hasParam("restartdescargasoc", true)){
        _config->bateria.soc_max_restart_descarga=request->getParam("restartdescargasoc", true)->value().toInt();
    }
    if(request->hasParam("stopdescargasoc", true)){
        _config->bateria.soc_min_stop_descarga=request->getParam("stopdescargasoc", true)->value().toInt();
    }
    saveConfigIntoEEPROM();
    request->redirect("/reglas");
}

void PortalWeb::salvarcalibracionSOC(AsyncWebServerRequest *request){
    if(request->hasParam("passwordcalibracion", true)){
        String password(request->getParam("passwordcalibracion",true)->value());
        Serial.println(password);
        if(password.equals(PASSWORD_CALIBRACION_SOC)){
            Serial.println("Pasword OK");
            if(request->hasParam("voltagecell00soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_0] =
                request->getParam("voltagecell00soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell01soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_1] =
                request->getParam("voltagecell01soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell02soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_2] =
                request->getParam("voltagecell02soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell03soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_3] =
                request->getParam("voltagecell03soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell04soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_4] =
                request->getParam("voltagecell04soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell05soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_5] =
                request->getParam("voltagecell05soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell07soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_7] =
                request->getParam("voltagecell07soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell10soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_10] =
                request->getParam("voltagecell10soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell15soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_15] =
                request->getParam("voltagecell15soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell80soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_80] =
                request->getParam("voltagecell80soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell85soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_85] =
                request->getParam("voltagecell85soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell90soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_90] =
                request->getParam("voltagecell90soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell95soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_95] =
                request->getParam("voltagecell95soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell96soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_96] =
                request->getParam("voltagecell96soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell97soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_97] =
                request->getParam("voltagecell97soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell98soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_98] =
                request->getParam("voltagecell98soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell99soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_99] =
                request->getParam("voltagecell99soc", true)->value().toInt();
            }
            if(request->hasParam("voltagecell100soc",true)){
                configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_100] =
                request->getParam("voltagecell100soc", true)->value().toInt();
            }
        }
    }
    configuracionSalvarEEPROM();
    request->redirect("/reglas");

}


String PortalWeb::procesarPrincipal(const String &var){
    if(var == "OPCION_MENU") return "home";  
    return "";
}


void PortalWeb::estadoActualBateria(AsyncWebServerRequest *request){
    AsyncResponseStream *response=  request->beginResponseStream("application/json"); //modificar size por defecto  1460
    DynamicJsonDocument docjson(4096);
    parseJK_JSON(docjson, &jk_bms_battery_info, &configuracion);
    String textojson="";
    int size=serializeJson(docjson, textojson);
    if(_config->comunicarSerialDebug){
        Serial.println(textojson);
        Serial.println();
    }                    
    response->print(textojson);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
    
}



String PortalWeb::procesarAjustes(const String &var){
    if(var == "OPCION_MENU") return "ajustes";
    
    // canbus
    if(var == "CHECK_CAN"  && _config->comunicarCAN==true) return "checked";

    //pylontech high voltage
    if(var == "CHECK_PYLON_HV" && _config->pylontechHV==true) return "checked";

    //serialdebug
    if(var == "CHECK_SERIAL"  && _config->comunicarSerialDebug == true) return "checked";
    if(var == "CHECK_SERIAL1"  && _config->comunicarSerialDebug1 == true) return "checked";
    if(var == "CHECK_SERIAL2"  && _config->comunicarSerialDebug2 == true) return "checked";
    
    //rs485  
    if(var == "CHECK_JKRS485"  && _config->comunicarJKrs485 == true) return "checked";

    //modbus
    if(var == "CHECK_MODBUS"  && _config->comunicarMODBUS == true) return "checked";
    if(var == "IP_MODBUS"){
        char texto[32];
        int textsize=sprintf(texto, "%d.%d.%d.%d",
        _config->ipmodbus[0], _config->ipmodbus[1], _config->ipmodbus[2], _config->ipmodbus[3]);
        return String(texto, textsize);
    }
    if(var == "PORT_MODBUS") return String(_config->portmodbus);
    
    
    //influxdb
    if(var == "CHECK_INFLUXDB"  && _config->comunicarINFLUXDB == true)return "checked";
    if(var == "IP_INFLUXDB"){
        char texto[32];
        int textsize=sprintf(texto, "%d.%d.%d.%d",
        _config->ipinfluxdb[0], _config->ipinfluxdb[1], _config->ipinfluxdb[2], _config->ipinfluxdb[3]);
        return String(texto, textsize);
    }
    if(var == "PORT_INFLUXDB") return String(_config->portinfluxdb);
    if(var == "DATABASE_INFLUXDB") return String(_config->databaseinfluxdb);
    if(var == "USER_INFLUXDB") return String(_config->userinfluxdb);
    if(var == "PASS_INFLUXDB") return String(_config->passinfludb);

    //mqtt
    if(var == "CHECK_MQTT"  && _config->comunicarMQTT == true) return "checked";
    if(var == "IP_MQTT"){
        char texto[32];
        int textsize=sprintf(texto, "%d.%d.%d.%d",
        _config->ipmqtt[0], _config->ipmqtt[1], _config->ipmqtt[2], _config->ipmqtt[3]);
        return String(texto, textsize);
    }
    if(var == "PORT_MQTT") return String(_config->portmqtt);
    if(var == "TOPIC_MQTT") return String(_config->topicmqtt);
    if(var == "USER_MQTT") return String(_config->usermqtt);
    if(var == "PASS_MQTT") return String(_config->passmqtt);

    return "";
}

String PortalWeb::procesarAcercade(const String &var){
    if(var == "OPCION_MENU") return "acercade";
    return "";
}



String PortalWeb::procesarReglas(const String &var){
    if(var == "OPCION_MENU") return "reglas";
    if(var == "STOP_CARGA_SOC") return      String(_config->bateria.soc_max_stop_carga);
    if(var == "RESTART_CARGA_SOC") return   String(_config->bateria.soc_min_restart_carga);
    if(var == "RESTART_DESCARGA_SOC")return String(_config->bateria.soc_max_restart_descarga);
    if(var == "STOP_DESCARGA_SOC")return    String(_config->bateria.soc_min_stop_descarga);
    if(var.startsWith("CORRIENTE")){
        if(var == "CORRIENTE_MAX_CARGA") return String(_jk_bms->battery_limits.battery_charge_current_limit);
        if(var == "CORRIENTE_80SOC") return     String(_config->bateria.intensidad_carga.soc_80);
        if(var == "CORRIENTE_85SOC") return     String(_config->bateria.intensidad_carga.soc_85);
        if(var == "CORRIENTE_90SOC") return     String(_config->bateria.intensidad_carga.soc_90);
        if(var == "CORRIENTE_95SOC") return     String(_config->bateria.intensidad_carga.soc_95);
        if(var == "CORRIENTE_MAX_DESCARGA") return String(_jk_bms->battery_limits.battery_discharge_current_limit);
        if(var == "CORRIENTE_20SOC") return     String(_config->bateria.intensidad_descarga.soc_20);
        if(var == "CORRIENTE_15SOC") return     String(_config->bateria.intensidad_descarga.soc_15);
        if(var == "CORRIENTE_10SOC") return     String(_config->bateria.intensidad_descarga.soc_10);
        if(var == "CORRIENTE_05SOC") return     String(_config->bateria.intensidad_descarga.soc_05);
    }
    if(var == "TENSION_MAX_CARGA") return   String(_jk_bms->battery_limits.battery_charge_voltage/100);
    if(var == "TENSION_MIN_DESCARGA") return String(_jk_bms->battery_limits.battery_discharge_voltage/100);
    if(var.startsWith("VOL_CELL_")){
        if(var == "VOL_CELL_00SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_0]);
        if(var == "VOL_CELL_01SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_1]);
        if(var == "VOL_CELL_02SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_2]);
        if(var == "VOL_CELL_03SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_3]);
        if(var == "VOL_CELL_04SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_4]);
        if(var == "VOL_CELL_05SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_5]);
        if(var == "VOL_CELL_07SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_7]);
        if(var == "VOL_CELL_10SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_10]);
        if(var == "VOL_CELL_15SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_15]);
        if(var == "VOL_CELL_80SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_80]);
        if(var == "VOL_CELL_85SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_85]);
        if(var == "VOL_CELL_90SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_90]);
        if(var == "VOL_CELL_95SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_95]);
        if(var == "VOL_CELL_96SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_96]);
        if(var == "VOL_CELL_97SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_97]);
        if(var == "VOL_CELL_98SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_98]);
        if(var == "VOL_CELL_99SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_99]);
        if(var == "VOL_CELL_100SOC")return String(_config->bateria.calibracion.voltageCell[CELL_SOC::SOC_100]);

    }


    return "";
}




String PortalWeb::procesarComandos(const String &var){
    return "";
}


String PortalWeb::procesarSetupwifi(const String &var){
    String texto="";

    if(var == "SSID"){
        texto="";
        String scanNetworks[15];
        int32_t rssiNetworks[15];
        for (int i = 0; i < 15; ++i) {
            if(WiFi.SSID(i) == "") { break; }
            scanNetworks[i] = WiFi.SSID(i);
            rssiNetworks[i] = (int8_t)WiFi.RSSI(i);
            int quality=0;
            if (rssiNetworks[i] <= -100) {
                 quality = 0;
             } else if (rssiNetworks[i] >= -50) {
                 quality = 100;
             } else {
                 quality = 2 * (rssiNetworks[i] + 100);
             }
            log_printf("SSID %i - %s (%d%%, %d dBm)\n", i, scanNetworks[i].c_str(), quality, rssiNetworks[i]);
            char tmp[80];
            sprintf(tmp,"%s (%d dBm)", scanNetworks[i].c_str(), rssiNetworks[i]);
            texto += (" <option value='" + scanNetworks[i] + "'>" + String(tmp) + "</option>");      
        }
        return texto;
    }


    return texto;
}




	


void PortalWeb::setupAccessPoint(AsyncWebServer *webserver,  Config * config, JK_bms_battery_info *jk_bms){
    _server=webserver;
    _config=config;
    _jk_bms=jk_bms;
	
    _server->onNotFound(handleNotFound);
    _server->on("/", HTTP_GET, handleRoot);
    _server->on("/index.html", HTTP_GET, handleRoot);
    _server->on("/comando", HTTP_GET, handleCommand);
    _server->on("/savedatawifi", HTTP_GET, salvarWiFi);
    _server->on("/generate_204", handleRoot);
    _server->on("/principal", HTTP_GET, paginaPrincipal);

    _server->on("/estadoactualbateria.json", HTTP_GET, estadoActualBateria);
    
    _server->on("/setupWifi.html", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        request->send(SPIFFS, "/setupWifi.html", "text/html", false, procesarSetupwifi);
    });
    
    _server->on("/ajustes", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        request->send(SPIFFS, "/principal.html", "text/html", false, procesarAjustes);
    });

    _server->on("/reglas", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        request->send(SPIFFS, "/principal.html", "text/html", false, procesarReglas);
    });

    _server->on("/acercade", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        request->send(SPIFFS, "/principal.html", "text/html", false, procesarAcercade);
    });

    /////////////////////////////////GET//////////////////////////////////////////////
    _server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/favicon.ico", "image/x-icon");
        response->addHeader("Cache-Control", "max-age=86400, must-revalidate");
        request->send(response);
    });

    _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/style.css", "text/css");
        response->addHeader("Content-Type", "text/css");
        response->addHeader("Cache-Control", "max-age=0, must-revalidate"); //"max-age=86400
        request->send(response);
    });

    _server->on("/principal.js", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/principal.js", "text/javascript");
        response->addHeader("Content-Type", "text/javascript");
        response->addHeader("Cache-Control", "max-age=0, must-revalidate");   //"max-age=86400
        request->send(response);
    });

    _server->on("/logoBlack68.png", HTTP_GET, [](AsyncWebServerRequest *request) { // GET
        //checkAuth(request);
        request->send(SPIFFS, "/logoBlack68.png", "image/png");
    });

    _server->on("/wait.png", HTTP_GET, [](AsyncWebServerRequest * request){
        request->send(SPIFFS, "/wait.png", "image/png");
    });

    /////////////////////////////POST//////////////////////////////////////
    _server->on("/salvarmodbusinversor", HTTP_POST, salvarmodbusinversor);
    _server->on("/salvarrs485", HTTP_POST, salvarrs485);
    _server->on("/salvarinfluxdb", HTTP_POST, salvarinfluxdb);
    _server->on("/salvarmqtt", HTTP_POST, salvarmqtt);
    _server->on("/salvarcanbus", HTTP_POST, salvarcanbus);
    _server->on("/salvarserialdebug", HTTP_POST, salvarserialdebug);
    _server->on("/salvarpylonHV", HTTP_POST, salvarpylonHV);
    _server->on("/salvarrampadescarga",HTTP_POST, salvarrampadescarga);
    _server->on("/salvarrampacarga", HTTP_POST, salvarrampacarga);
    _server->on("/salvarlimitessoc", HTTP_POST, salvarlimitesSOC);
    _server->on("/salvarcalibracionsoc", HTTP_POST, salvarcalibracionSOC);

}
