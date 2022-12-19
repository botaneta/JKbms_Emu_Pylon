#ifndef _UTILIDADES_
#define _UTILIDADES_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "jk_bms_485.h"
#include "configuracion.h"

String bufferToString(String text, uint8_t * buffer);
String toStringJkBms(JK_bms_battery_info * jkbms);
uint8_t parseByteLetras(char * textoHexadecimal);
void imprimeJKBMSinfo(const struct JK_bms_battery_info &jkbms); 
void parseaTextoTobytes(char * ptr_cadena, uint8_t * bufferdatos);
void parseJK_JSON(DynamicJsonDocument &doc, JK_bms_battery_info * jkbms, Config * config);






#endif