#ifndef _PYLON_CAN_
#define _PYLON_CAN_

//Use standard frame, communication rate: 500kbps, data transmission cycle: 1s.
//Inverter reply every second: 0x305: 00-00-00-00-00-00-00-00

// frame CAN_BUS
// CAN_ID (11bits)  |    byte0  |   byte1   |   byte2   |   byte3   |   byte4   |   byte5   |   byte6   |   byte7   |
//--------------------------------------------------------------------------------------------------------------------
//  0x359           |protection |protection |Alarm      |Alarm      |Modules nº |    "P"    |    "N"    |    -      |
//                  | tabla1    |table2     |table3     |tabla4     | uint8_t   |   0x50    |    0x4E   |           |
//--------------------------------------------------------------------------------------------------------------------
//  0x351           |Battery charge voltage | Charger current limit |Discharger current limt|           |           |
//                  |unit 0.1V  uint16_t    |0.1A int16 (2´s comple)|0.1A int16 (2´s comple)|           |           |
//--------------------------------------------------------------------------------------------------------------------
//  0x355           |SOC unit1%  uint16_t   | SOH unit1%  uint16_t  |           |           |           |           |
//--------------------------------------------------------------------------------------------------------------------           
//  0x356           |Voltage average system |Total current system   |Average cell temp      |           |           |
//                  |unit:0.01V 16bitSigned |unit:0.1A  16bitSigned |unit:0.1ºC 16bitSigned |           |           |
//--------------------------------------------------------------------------------------------------------------------
//  0x35C           |Request flag|          | 
//                  |table5      |          | 
//--------------------------------------------------------------------------------------------------------------------            
//  0x35E           |Manufacturer Name      |           |           |           |           |           |           |
//                  |"PYLON" (ASCII)        |           |           |           |           |           |           |
//--------------------------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------------------------
//  TABLES          |   bit7    |   bit6    |   bit5    |   bit4    |  bit3     |  bit2     |   bit1    |   bit0    |
//--------------------------------------------------------------------------------------------------------------------
//   Table1         |Discarger  |           |           |Cell  under|Cell  over |Cell/System|Cell/System|           |
//                  |over_current|          |           |temperature|temperature|under Voltg|over voltg |           |
//--------------------------------------------------------------------------------------------------------------------
//   Table2         |           |           |           |           |System error|          |           |Charge over|
//                  |           |           |           |           |            |          |           | current   |
//--------------------------------------------------------------------------------------------------------------------
//   Table3         |Discharge high|        |           |Cell low   |Cell  high |Cell/System|Cell/System|           |
//                  |current       |        |           |temperature|temperature|low voltage|high voltag|           |
//--------------------------------------------------------------------------------------------------------------------
//   Table4         |           |           |           |           |Internal   |           |           |Charge     |
//                  |           |           |           |           |communication|         |           |high       |
//                  |           |           |           |           |fail        |          |           |current    |
//--------------------------------------------------------------------------------------------------------------------
//  Table5          |Charge     |Discharge  |Request forc|Request forc|Request full|        |           |           |
//                  |enable     | enable    |charge I    |charge II   | charge **  |        |           |           |    
//--------------------------------------------------------------------------------------------------------------------
// NOTE:  littel endian  -----uint16 and int16 data byte0 lowWord byte1 highWord------ 

#include <Arduino.h>
#include <jk_bms_485.h>
#include "configuracion.h"



uint8_t * parseJK_message_0x359(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info);

uint8_t * parseJK_message_0x351(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info);

uint8_t * parseJK_message_0x355(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info);

uint8_t * parseJK_message_0x356(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info);

uint8_t * parseJK_message_0x35C(uint8_t * buffer2, JK_bms_battery_info *jk_bms_battery_info);

uint8_t * parseJK_message_0x35E(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info);


extern Config configuracion;




#endif