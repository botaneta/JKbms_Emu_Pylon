
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
   
    if(request->hasParam("rampadescarga", true)){
        if(request->getParam("rampadescarga" , true)->value().equals("rampaMV")){
            _config->bateria.rampaDescarga_mV=true;
        }else{
            _config->bateria.rampaDescarga_mV=false;
        }
    }

    if(request->hasParam("regla1SOC", true)){
        _config->bateria.rampaDescarga.norma[0].valor[SOC]=request->getParam("regla1SOC", true)->value().toInt();
    }
    if(request->hasParam("regla1mV", true)){
        _config->bateria.rampaDescarga.norma[0].valor[mV]=request->getParam("regla1mV", true)->value().toInt();
    }
    if(request->hasParam("regla1Amp", true)){
        _config->bateria.rampaDescarga.norma[0].valor[Amp]=request->getParam("regla1Amp", true)->value().toInt();
    }


    if(request->hasParam("regla2SOC", true)){
        _config->bateria.rampaDescarga.norma[1].valor[SOC]=request->getParam("regla2SOC", true)->value().toInt();
    }
    if(request->hasParam("regla2mV", true)){
        _config->bateria.rampaDescarga.norma[1].valor[mV]=request->getParam("regla2mV", true)->value().toInt();
    }
    if(request->hasParam("regla2Amp", true)){
        _config->bateria.rampaDescarga.norma[1].valor[Amp]=request->getParam("regla2Amp", true)->value().toInt();
    }


    if(request->hasParam("regla3SOC", true)){
        _config->bateria.rampaDescarga.norma[2].valor[SOC]=request->getParam("regla3SOC", true)->value().toInt();
    }
    if(request->hasParam("regla3mV", true)){
        _config->bateria.rampaDescarga.norma[2].valor[mV]=request->getParam("regla3mV", true)->value().toInt();
    }
    if(request->hasParam("regla3Amp", true)){
        _config->bateria.rampaDescarga.norma[2].valor[Amp]=request->getParam("regla3Amp", true)->value().toInt();
    }


    if(request->hasParam("regla4SOC", true)){
        _config->bateria.rampaDescarga.norma[3].valor[SOC]=request->getParam("regla4SOC", true)->value().toInt();
    }
    if(request->hasParam("regla4mV", true)){
        _config->bateria.rampaDescarga.norma[3].valor[mV]=request->getParam("regla4mV", true)->value().toInt();
    }
    if(request->hasParam("regla4Amp", true)){
        _config->bateria.rampaDescarga.norma[3].valor[Amp]=request->getParam("regla4Amp", true)->value().toInt();
    }


    if(request->hasParam("regla5SOC", true)){
        _config->bateria.rampaDescarga.norma[4].valor[SOC]=request->getParam("regla5SOC", true)->value().toInt();
    }
    if(request->hasParam("regla5mV", true)){
        _config->bateria.rampaDescarga.norma[4].valor[mV]=request->getParam("regla5mV", true)->value().toInt();
    }
    if(request->hasParam("regla5Amp", true)){
        _config->bateria.rampaDescarga.norma[4].valor[Amp]=request->getParam("regla5Amp", true)->value().toInt();
    }
    saveConfigIntoEEPROM();
    request->redirect("/reglas");
}

void PortalWeb::salvarrampacarga(AsyncWebServerRequest *request){
   

    if(request->hasParam("rampacarga", true)){
        if(request->getParam("rampacarga" , true)->value().equals("rampaMV")){
            _config->bateria.rampaCarga_mV=true;
        }else{
            _config->bateria.rampaCarga_mV=false;
        }
    }


    if(request->hasParam("regla1SOC", true)){
        _config->bateria.rampaCarga.norma[0].valor[SOC]=request->getParam("regla1SOC", true)->value().toInt();
    }
    if(request->hasParam("regla1mV", true)){
        _config->bateria.rampaCarga.norma[0].valor[mV]=request->getParam("regla1mV", true)->value().toInt();
    }
    if(request->hasParam("regla1Amp", true)){
        _config->bateria.rampaCarga.norma[0].valor[Amp]=request->getParam("regla1Amp", true)->value().toInt();
    }


    if(request->hasParam("regla2SOC", true)){
        _config->bateria.rampaCarga.norma[1].valor[SOC]=request->getParam("regla2SOC", true)->value().toInt();
    }
    if(request->hasParam("regla2mV", true)){
        _config->bateria.rampaCarga.norma[1].valor[mV]=request->getParam("regla2mV", true)->value().toInt();
    }
    if(request->hasParam("regla2Amp", true)){
        _config->bateria.rampaCarga.norma[1].valor[Amp]=request->getParam("regla2Amp", true)->value().toInt();
    }


    if(request->hasParam("regla3SOC", true)){
        _config->bateria.rampaCarga.norma[2].valor[SOC]=request->getParam("regla3SOC", true)->value().toInt();
    }
    if(request->hasParam("regla3mV", true)){
        _config->bateria.rampaCarga.norma[2].valor[mV]=request->getParam("regla3mV", true)->value().toInt();
    }
    if(request->hasParam("regla3Amp", true)){
        _config->bateria.rampaCarga.norma[2].valor[Amp]=request->getParam("regla3Amp", true)->value().toInt();
    }


    if(request->hasParam("regla4SOC", true)){
        _config->bateria.rampaCarga.norma[3].valor[SOC]=request->getParam("regla4SOC", true)->value().toInt();
    }
    if(request->hasParam("regla4mV", true)){
        _config->bateria.rampaCarga.norma[3].valor[mV]=request->getParam("regla4mV", true)->value().toInt();
    }
    if(request->hasParam("regla4Amp", true)){
        _config->bateria.rampaCarga.norma[3].valor[Amp]=request->getParam("regla4Amp", true)->value().toInt();
    }


    if(request->hasParam("regla5SOC", true)){
        _config->bateria.rampaCarga.norma[4].valor[SOC]=request->getParam("regla5SOC", true)->value().toInt();
    }
    if(request->hasParam("regla5mV", true)){
        _config->bateria.rampaCarga.norma[4].valor[mV]=request->getParam("regla5mV", true)->value().toInt();
    }
    if(request->hasParam("regla5Amp", true)){
        _config->bateria.rampaCarga.norma[4].valor[Amp]=request->getParam("regla5Amp", true)->value().toInt();
    }

    saveConfigIntoEEPROM();
    request->redirect("/reglas");
}

void PortalWeb::salvarlimitesVoltajes(AsyncWebServerRequest *request){
    _config->bateria.voltajesCargaDescargaConfigurados=false;
    if(request->hasParam("checkcustomtension", true)){
        if(request->getParam("checkcustomtension",true)->value().equals("1")){
            _config->bateria.voltajesCargaDescargaConfigurados=true;
        }
    }
    if(request->hasParam("tensionMaximaCarga",true)){
        uint16_t voltaje=request->getParam("tensionMaximaCarga",true)->value().toFloat()*100;        
        _config->bateria.voltajeMaxCarga=voltaje;
        if(voltaje==0)_config->bateria.voltajesCargaDescargaConfigurados=false;
    }
    if(request->hasParam("tensionMinimaDescarga",true)){
        uint16_t voltaje=request->getParam("tensionMinimaDescarga",true)->value().toFloat()*100;
        _config->bateria.voltajeMinDescarga=voltaje;
        if(voltaje==0)_config->bateria.voltajesCargaDescargaConfigurados=false;
    }
    if(_config->bateria.voltajesCargaDescargaConfigurados){
        _jk_bms->battery_limits.battery_charge_voltage=_config->bateria.voltajeMaxCarga;
        _jk_bms->battery_limits.battery_discharge_voltage=_config->bateria.voltajeMinDescarga;
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

     _config->bateria.stopCargaPorVoltaje=false;
    if(request->hasParam("checkstopcargatension", true)){
        if(request->getParam("checkstopcargatension",true)->value().equals("1"))_config->bateria.stopCargaPorVoltaje=true;    
    }
    if(request->hasParam("stopcargatension", true)){
        _config->bateria.voltajeStopCarga=request->getParam("stopcargatension", true)->value().toFloat()*100;
    }
    _config->bateria.stopDescargaPorVoltaje=false;
    if(request->hasParam("checkstopdescargatension", true)){
        if(request->getParam("checkstopdescargatension", true)->value().equals("1")) _config->bateria.stopDescargaPorVoltaje=true;
    }
    if(request->hasParam("stopdescargatension", true)){
        _config->bateria.voltajeStopDescarga=request->getParam("stopdescargatension", true)->value().toFloat()*100;
    }

    saveConfigIntoEEPROM();
    request->redirect("/reglas");
}

void PortalWeb::salvarNivelSocBajo(AsyncWebServerRequest *request){
    if(request->hasParam("nivelsocbajo",true)){
        uint8_t nivelSOC=(uint8_t)request->getParam("nivelsocbajo", true)->value().toInt();
        configuracion.bateria.nivelSOCbajo=nivelSOC;
        configuracionSalvarEEPROM();
    }
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
    if(var == "TENSION_MAX_CARGA") return   String(_jk_bms->battery_limits.battery_charge_voltage/100.0);
    if(var == "TENSION_MIN_DESCARGA") return String(_jk_bms->battery_limits.battery_discharge_voltage/100.0);
    if(var == "CORRIENTE_MAX_CARGA") return String(_jk_bms->battery_limits.battery_charge_current_limit);
    if(var == "CORRIENTE_MAX_DESCARGA")return String(_jk_bms->battery_limits.battery_discharge_current_limit);
    if(var == "NIVEL_SOC_BAJO")return String(_config->bateria.nivelSOCbajo); 
    if(var == "CHECK_CUSTOM_TENSION") return String(_config->bateria.voltajesCargaDescargaConfigurados? "checked": "");
    if(var == "CHECK_STOP_CARGA_TENSION")return _config->bateria.stopCargaPorVoltaje? "checked" : "";
    if(var == "CHECK_STOP_DESCARGA_TENSION")return _config->bateria.stopDescargaPorVoltaje? "checked" : "";
    if(var == "STOP_CARGA_TENSION")return String(_config->bateria.voltajeStopCarga/100.0);
    if(var == "STOP_DESCARGA_TENSION")return String(_config->bateria.voltajeStopDescarga/100.0);
    if(var == "C_RADIO_SOC") return _config->bateria.rampaCarga_mV? "" : "checked";
    if(var == "C_RADIO_MV") return _config->bateria.rampaCarga_mV? "checked" : "";
    if(var == "D_RADIO_SOC") return _config->bateria.rampaDescarga_mV? "" : "checked";
    if(var == "D_RADIO_MV") return _config->bateria.rampaDescarga_mV? "checked" : "";
    
    if(var.startsWith("C_REGLA")){
        if(var=="C_REGLA1_SOC")return String(_config->bateria.rampaCarga.norma[0].valor[SOC]);
        if(var=="C_REGLA1_MV") return String(_config->bateria.rampaCarga.norma[0].valor[mV]);
        if(var=="C_REGLA1_AMP")return String(_config->bateria.rampaCarga.norma[0].valor[Amp]);
        if(var=="C_REGLA2_SOC")return String(_config->bateria.rampaCarga.norma[1].valor[SOC]);
        if(var=="C_REGLA2_MV") return String(_config->bateria.rampaCarga.norma[1].valor[mV]);
        if(var=="C_REGLA2_AMP")return String(_config->bateria.rampaCarga.norma[1].valor[Amp]);
        if(var=="C_REGLA3_SOC")return String(_config->bateria.rampaCarga.norma[2].valor[SOC]);
        if(var=="C_REGLA3_MV") return String(_config->bateria.rampaCarga.norma[2].valor[mV]);
        if(var=="C_REGLA3_AMP")return String(_config->bateria.rampaCarga.norma[2].valor[Amp]);
        if(var=="C_REGLA4_SOC")return String(_config->bateria.rampaCarga.norma[3].valor[SOC]);
        if(var=="C_REGLA4_MV") return String(_config->bateria.rampaCarga.norma[3].valor[mV]);
        if(var=="C_REGLA4_AMP")return String(_config->bateria.rampaCarga.norma[3].valor[Amp]);
        if(var=="C_REGLA5_SOC")return String(_config->bateria.rampaCarga.norma[4].valor[SOC]);
        if(var=="C_REGLA5_MV") return String(_config->bateria.rampaCarga.norma[4].valor[mV]);
        if(var=="C_REGLA5_AMP")return String(_config->bateria.rampaCarga.norma[4].valor[Amp]);
    }

    if(var.startsWith("D_REGLA")){
        if(var=="D_REGLA1_SOC")return String(_config->bateria.rampaDescarga.norma[0].valor[SOC]);
        if(var=="D_REGLA1_MV") return String(_config->bateria.rampaDescarga.norma[0].valor[mV]);
        if(var=="D_REGLA1_AMP")return String(_config->bateria.rampaDescarga.norma[0].valor[Amp]);
        if(var=="D_REGLA2_SOC")return String(_config->bateria.rampaDescarga.norma[1].valor[SOC]);
        if(var=="D_REGLA2_MV") return String(_config->bateria.rampaDescarga.norma[1].valor[mV]);
        if(var=="D_REGLA2_AMP")return String(_config->bateria.rampaDescarga.norma[1].valor[Amp]);
        if(var=="D_REGLA3_SOC")return String(_config->bateria.rampaDescarga.norma[2].valor[SOC]);
        if(var=="D_REGLA3_MV") return String(_config->bateria.rampaDescarga.norma[2].valor[mV]);
        if(var=="D_REGLA3_AMP")return String(_config->bateria.rampaDescarga.norma[2].valor[Amp]);
        if(var=="D_REGLA4_SOC")return String(_config->bateria.rampaDescarga.norma[3].valor[SOC]);
        if(var=="D_REGLA4_MV") return String(_config->bateria.rampaDescarga.norma[3].valor[mV]);
        if(var=="D_REGLA4_AMP")return String(_config->bateria.rampaDescarga.norma[3].valor[Amp]);
        if(var=="D_REGLA5_SOC")return String(_config->bateria.rampaDescarga.norma[4].valor[SOC]);
        if(var=="D_REGLA5_MV") return String(_config->bateria.rampaDescarga.norma[4].valor[mV]);
        if(var=="D_REGLA5_AMP")return String(_config->bateria.rampaDescarga.norma[4].valor[Amp]);
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
    _server->on("/salvarlimitesvoltajes", HTTP_POST, salvarlimitesVoltajes);
    _server->on("/salvarnivelsocbajo", HTTP_POST, salvarNivelSocBajo);

}
