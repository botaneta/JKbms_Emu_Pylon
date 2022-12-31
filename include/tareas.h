#ifndef _TAREAS_H_
#define _TAREAS_H_

#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "jk_bms_485.h"
#include "utilidades.h"
#include "configuracion.h"
#include "driver/twai.h"  //canbus


//variables globales declaradas en main.cpp

extern METER meter;

extern struct Config  configuracion;

extern uint8_t bufferStream[512];   //cambiar a 512 pruebas reales

extern uint8_t bufferReceiver[512];  

extern AsyncMqttClient mqtt;





//Manejadores de tareas concurrentes

extern TaskHandle_t  peticionesRS485_handle;
extern TaskHandle_t  enviosMODBUS_handle;
extern TaskHandle_t  enviosCANBUS_handle;
extern TaskHandle_t  recibirCANBUS_handle; 
extern TaskHandle_t  capturarSerialPushCola_handle;
extern TaskHandle_t  capturarSerial2PushCola_handle;
extern TaskHandle_t  parseaSerialToJk_bms_battery_info_handle;
extern TaskHandle_t  parpadeo_led_handle;
extern TaskHandle_t  imprimeDatos_taskhandle_handle;
extern TaskHandle_t  reset_configuracion_handle;

extern SemaphoreHandle_t xMutex;


//Cola de mensajes entre tareas concurrentes
extern QueueHandle_t colaPuertoSerie_handle;
extern QueueHandle_t colaEscritura_handle;
extern QueueHandle_t colaLecturaCAN_handle;




//funciones de tareas
void peticionrs485_task(void * parameters);
void capturarSerialToColaLectura_task(void *parameters);
void capturarSerial2ToColaLectura_task(void *parameters);
void parseoColaLecturaToJk_bms_battery_info_task(void * parameters);
void parpadeoLed_task(void * parameters);
void resetConfiguracion_task(void * parameters);
void envioCAN_task(void * parameters);
void recibirCAN_task(void * parameters);
void enviarDatosMqtt_task(void * parameters);

//funciones auxiliares
/** Actualiza los permisos de carga y descarga para el inversor en funcion del soc*/
void ajustarVoltajeCargaDescarga();
void controlCargaDescarga();
void actualizarContadorEnergia();
void ajustarSOC();
void ajustarAmperiosCargaDescarga();
void enviarCANpylonLV();
void enviarCANpylonHV(bool ID_29bits);
void enviarCANpylonHVinfoSystem(bool ID_29bits);
void enviarCANbatrium();
void enviarCANbatrium2();
void mostrarMensajeCAN_pylonHV();
void mostrarMensajeCAN_pylonLV();
void mostrarMensajeCAN_Batrium();









#endif