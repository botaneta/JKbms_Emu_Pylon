#ifndef _PYLON_HV_CAN_
#define _PYLON_HV_CAN_

#include "jk_bms_485.h"

#define TENSION_NOMINAL_CELDA_LIFEPO4  3.2

uint8_t * parseJK_message_0x7310(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x7320(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x7330(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x7340(uint8_t * buffer, JK_bms_battery_info * jkbms);



uint8_t * parseJK_message_0x4210(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4220(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4230(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4240(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4250(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4260(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4270(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4280(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x4290(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x42A0(uint8_t * buffer, JK_bms_battery_info * jkbms);

#endif