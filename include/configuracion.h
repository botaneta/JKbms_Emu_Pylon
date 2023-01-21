#ifndef _CONFIGURACION_H_
#define _CONFIGURACION_H_
#include <Arduino.h>
#include <EEPROM.h>

#define SOC 0
#define mV 1
#define Amp 2

enum CANBUS { NO_CONFIGURADO=0, PYLON_LV=1, PYLON_HV=3, RESERVADO1=5, RESERVADO2=7 };

struct METER{
    uint32_t miliamperioSegundo;
};


struct Norma{
     uint16_t valor[3];
 };


struct RampaCarga{
    Norma norma[5];
};

struct RampaDescarga{
    Norma norma[5];
};


struct CalibracionSOC{
    uint16_t voltageCell[18];

};

struct Bateria{
    bool cargado_total;
    bool calibracionSOC;
    bool rampaCarga_mV;
    bool rampaDescarga_mV;
    bool stopCargaPorVoltaje;
    bool stopDescargaPorVoltaje;
    bool voltajesCargaDescargaConfigurados;
    struct CalibracionSOC calibracion;
	
    struct RampaCarga rampaCarga;
    struct RampaDescarga rampaDescarga;
    uint16_t voltajeMaxCarga;
    uint16_t voltajeMinDescarga;
    uint16_t voltajeStopCarga;
    uint16_t voltajeStopDescarga;
    uint8_t soc_max_stop_carga;
    uint8_t soc_min_restart_carga;
    uint8_t soc_min_stop_descarga;
    uint8_t soc_max_restart_descarga;
    uint8_t nivelSOCbajo;
};


struct Config{
    bool wifiConfigured;
    bool dispositivoCAN;
    bool comunicarJKrs485;
    bool comunicarCAN;
    bool comunicarSerialDebug;
    bool comunicarSerialDebug1;
    bool comunicarSerialDebug2;
    bool comunicarMQTT;
    bool comunicarINFLUXDB;
    bool comunicarMODBUS;
    bool pararDescargaPorHorario;
    bool pararCargaPorHorario;
    bool pararDescargaPorMQTT;
    bool pararCargaPorMQTT;
    bool habilitarCarga;
    bool habilitarDescarga;
    bool errorComunicacionJK;
    CANBUS protocoloCanBus;
    Bateria bateria;
    
    uint8_t tiempoconsultaJK;
    uint8_t tiempoenvioCAN;
    uint8_t ipmodbus[4];
    uint16_t portmodbus;
    uint8_t ipinfluxdb[4];
    uint16_t portinfluxdb;
    char databaseinfluxdb[64];
    char userinfluxdb[32];
    char passinfludb[32];
    uint8_t ipmqtt[4];
    uint16_t portmqtt;
    char topicmqtt[64];
    char usermqtt[32];
    char passmqtt[32];
    char hostName[30]="JKBMS_EMU_PYLONTECH";
    char ssid1[31+1];
    char pass1[63+1];

    char version[32];  //sirve de palabra de control por el primer inicio en el esp32 
        
};

extern Config configuracion;

void configuracionLeerEEPROM();
void configuracionSalvarEEPROM();

/* Establece valores por defecto, sin configuración */
void configuracionInicial();

/* Solo borra la configuración de WIFI */
void configuracionResetWifi();

/* calcula la intesidad en función del soc */
uint16_t configuracionIntesidadCargaSOC();

/* calcula la intesidad en función del soc */
uint16_t configuracionIntesidadDescargaSOC();

#endif