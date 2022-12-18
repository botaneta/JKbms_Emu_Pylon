#ifndef _BATRIUM_CAN_
#define _BATRIUM_CAN_
#include "jk_bms_485.h"

/*
Extended canbus identifiers
Default baud rate : 500kB 
Little indian for multibyte fields

Identifier: Base Address + Hex 0x00 message
*/


uint8_t * parseJK_message_0x00(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x01(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x02(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x03(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x04(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x05(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x06(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x07(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x08(uint8_t * buffer, JK_bms_battery_info * jkbms);



// otra versión de protocolo CAN de batrium, mensajes recomendados
uint8_t * parseJK_message_0x00111100(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00111200(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00111300(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00111500(uint8_t * buffer, JK_bms_battery_info * jkbms);

uint8_t * parseJK_message_0x00140100(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140200(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140300(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140400(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140400(uint8_t * buffer, JK_bms_battery_info * jkbms);













#endif