#ifndef _CONFIGURACION_H_
#define _CONFIGURACION_H_
#include <Arduino.h>
#include <EEPROM.h>

#define SOC 0
#define mV 1
#define Amp 2
#define PASSWORD_CALIBRACION_SOC "Calibración SOC"

     // curvas de vol & soc    0%,   1%,   2%,   3%,   4%,   5%,   7%, 1 0%,   15%,   80%, 85%, 90%, 95%, 96%, 97%, 98%, 99%, 100%
const uint8_t valorPorciento[]={0,   1,    2,    3,     4,    5,    7,   10,    15,    80,   85,  90,  95,  96, 97, 98,  99,  100};     
const uint16_t curvaLFPO[]=   {2500, 2530, 2590, 2660, 2730, 2800, 2870, 3000, 3025,  3350,3375,3400,3410,3420,3506,3554,3602,3650};
const uint16_t curvaLiION[]=  {3000, 3012, 3024, 3036, 3048, 3060, 3084, 3120, 3180,  3960,4020,4080,4140,4152,4164,4176,4188,4200};
const uint16_t curvaLTO[18]={};
enum CELL_SOC                 {SOC_0,SOC_1,SOC_2,SOC_3,SOC_4,SOC_5,SOC_7,SOC_10,SOC_15,
                                                                    SOC_80,SOC_85,SOC_90,SOC_95,SOC_96,SOC_97,SOC_98,SOC_99,SOC_100,
    SOC_COUNT
};

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
    bool consoleLog;
    bool comunicarSerialDebug;
    bool comunicarSerialDebug1;
    bool comunicarSerialDebug2;
    bool comunicarMQTT;
    bool comunicarINFLUXDB;
    bool comunicarMODBUS;
    bool pylontechHV;
    bool batrium;
    bool habilitarCarga;
    bool habilitarDescarga;
    bool errorComunicacionJK;
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