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
    configuracion.comunicarMQTT=false;
    configuracion.comunicarINFLUXDB=false;
    configuracion.comunicarMODBUS=false;

    configuracion.batrium=false;
    configuracion.pylontechHV=false;
    configuracion.habilitarCarga=false;
    configuracion.habilitarDescarga=false;

    configuracion.errorComunicacionJK=false;

    configuracion.bateria.cargado_total=false;
    configuracion.bateria.intensidad_carga.soc_80=100;
    configuracion.bateria.intensidad_carga.soc_85=50;
    configuracion.bateria.intensidad_carga.soc_90=15;
    configuracion.bateria.intensidad_carga.soc_95=2;
    configuracion.bateria.intensidad_descarga.soc_20=50;
    configuracion.bateria.intensidad_descarga.soc_15=25;
    configuracion.bateria.intensidad_descarga.soc_10=5;
    configuracion.bateria.intensidad_descarga.soc_05=0;
    configuracion.bateria.soc_max_restart_descarga=25;
    configuracion.bateria.soc_min_stop_descarga=8;
    configuracion.bateria.soc_max_stop_carga=99;
    configuracion.bateria.soc_min_restart_carga=80;


    for(int i=0; i < CELL_SOC::SOC_COUNT; i++){
        configuracion.bateria.calibracion.voltageCell[i]=curvaLFPO[i];
    }
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



