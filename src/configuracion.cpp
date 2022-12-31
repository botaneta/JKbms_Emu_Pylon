#include <Arduino.h>
#include "configuracion.h"
#include "jk_bms_485.h"

extern JK_bms_battery_info jk_bms_battery_info;

void configuracionInicial(){
    
    configuracion.wifiConfigured=false;
    configuracion.dispositivoCAN=false;
    configuracion.comunicarJKrs485=false;
    configuracion.comunicarCAN=false;
    configuracion.consoleLog=false;
    configuracion.comunicarSerialDebug=false;
    configuracion.comunicarSerialDebug1=false;
    configuracion.comunicarSerialDebug2=false;
    configuracion.comunicarMQTT=false;
    configuracion.comunicarINFLUXDB=false;
    configuracion.comunicarMODBUS=false;

    configuracion.batrium=false;
    configuracion.pylontechHV=false;
    configuracion.habilitarCarga=false;
    configuracion.habilitarDescarga=false;

    configuracion.errorComunicacionJK=false;

    
    configuracion.bateria.soc_max_restart_descarga=25;
    configuracion.bateria.soc_min_stop_descarga=8;
    configuracion.bateria.soc_max_stop_carga=95;
    configuracion.bateria.soc_min_restart_carga=80;
    configuracion.bateria.nivelSOCbajo=25;

    configuracion.bateria.calibracionSOC=false;
    configuracion.bateria.stopCargaPorVoltaje=false;
    configuracion.bateria.stopDescargaPorVoltaje=false;
    configuracion.bateria.rampaCarga_mV=false;
    configuracion.bateria.rampaDescarga_mV=false;
    configuracion.bateria.voltajesCargaDescargaConfigurados=false;

    configuracion.bateria.voltajeMaxCarga=0;
    configuracion.bateria.voltajeMinDescarga=0;
    /* Valores por defecto para celdas LIFEPO4 */
    configuracion.bateria.rampaCarga.norma[0].valor[SOC]=80;
    configuracion.bateria.rampaCarga.norma[0].valor[mV]=3300;
    configuracion.bateria.rampaCarga.norma[0].valor[Amp]=70;
    configuracion.bateria.rampaCarga.norma[1].valor[SOC]=85;
    configuracion.bateria.rampaCarga.norma[1].valor[mV]=3340;
    configuracion.bateria.rampaCarga.norma[1].valor[Amp]=70;
    configuracion.bateria.rampaCarga.norma[2].valor[SOC]=90;
    configuracion.bateria.rampaCarga.norma[2].valor[mV]=3390;
    configuracion.bateria.rampaCarga.norma[2].valor[Amp]=70;
    configuracion.bateria.rampaCarga.norma[3].valor[SOC]=95;
    configuracion.bateria.rampaCarga.norma[3].valor[mV]=3400;
    configuracion.bateria.rampaCarga.norma[3].valor[Amp]=20;
    configuracion.bateria.rampaCarga.norma[4].valor[SOC]=98;
    configuracion.bateria.rampaCarga.norma[4].valor[mV]=3450;
    configuracion.bateria.rampaCarga.norma[4].valor[Amp]=5;

    configuracion.bateria.rampaDescarga.norma[0].valor[SOC]=20;
    configuracion.bateria.rampaDescarga.norma[0].valor[mV]=3100;
    configuracion.bateria.rampaDescarga.norma[0].valor[Amp]=70;
    configuracion.bateria.rampaDescarga.norma[1].valor[SOC]=15;
    configuracion.bateria.rampaDescarga.norma[1].valor[mV]=3050;
    configuracion.bateria.rampaDescarga.norma[1].valor[Amp]=35;
    configuracion.bateria.rampaDescarga.norma[2].valor[SOC]=10;
    configuracion.bateria.rampaDescarga.norma[2].valor[mV]=3000;
    configuracion.bateria.rampaDescarga.norma[2].valor[Amp]=10;
    configuracion.bateria.rampaDescarga.norma[3].valor[SOC]=5;
    configuracion.bateria.rampaDescarga.norma[3].valor[mV]=2800;
    configuracion.bateria.rampaDescarga.norma[3].valor[Amp]=4;
    configuracion.bateria.rampaDescarga.norma[4].valor[SOC]=2;
    configuracion.bateria.rampaDescarga.norma[4].valor[mV]=2680;
    configuracion.bateria.rampaDescarga.norma[4].valor[Amp]=0;


    strcpy(configuracion.hostName, "JKBMS_EMU_PYLON");
    configuracionSalvarEEPROM();
}

void configuracionSalvarEEPROM(){
    EEPROM.put(0, configuracion);
    EEPROM.commit();
}

void configuracionLeerEEPROM(){
    EEPROM.get(0, configuracion);
}



