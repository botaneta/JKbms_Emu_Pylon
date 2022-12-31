#include "batrium_can.h"
#include "configuracion.h"

extern Config configuracion;

//Enviar mensajes cada 1000msg

/* Mensaje con la versión del dispositivo  TX FREQ:10sg*/
uint8_t * parseJK_message_0x00(uint8_t * buffer, JK_bms_battery_info * jkbms){
    // hardware version  uint16  i.e  4.1
    buffer[0]=0x01;
    buffer[1]=0x04;
    //firmware version  uint16  i.e 1.29
    buffer[2]=0x1D;
    buffer[3]=0x01;
    // Device Serial number   nº:31122022
    buffer[4]=0x66;
    buffer[5]=0xE2;
    buffer[6]=0xDA;
    buffer[7]=0x01;

    return buffer;

}

/* Mensaje voltaje de celda max, medio, min y sus números   TX FREQ:100mSg*/
uint8_t * parseJK_message_0x01(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint8_t celdaMax=jkbms->cell_number_vmax; //  1..24
    uint8_t celdaMin=jkbms->cell_number_vmin;
    uint16_t maxCellVol=jkbms->cells_voltage[celdaMax - 1].cell_voltage;
    uint16_t avgCellVol=jkbms->cell_Vavrg;
    uint16_t minCellVol=jkbms->cells_voltage[celdaMin - 1].cell_voltage;
    //jk unit 0.001V  batrium unit 0.001V
    buffer[0]=(uint8_t)(minCellVol & 0xFF); //lsb
    buffer[1]=(uint8_t)(minCellVol >> 8);  //msb 
    buffer[2]=(uint8_t)(maxCellVol & 0xFF); 
    buffer[3]=(uint8_t)(maxCellVol >> 8);  
    buffer[4]=(uint8_t)(avgCellVol & 0xFF); 
    buffer[5]=(uint8_t)(avgCellVol >> 8); 
    buffer[6]=celdaMin;
    buffer[7]=celdaMax;     
    return buffer;
}

/* Mensaje temepratura de celda max, medio, min y sus números  TX FREQ:1000mSg*/
uint8_t * parseJK_message_0x02(uint8_t * buffer, JK_bms_battery_info * jkbms){
    int8_t t1=jkbms->battery_status.sensor_temperature_1;
    int8_t t2=jkbms->battery_status.sensor_temperature_2;
    int8_t tmosf=jkbms->battery_status.power_tube_temperature;
    int8_t avgTemp= (t1 + t2)/2;
    if(t1 <= t2){
        uint8_t temp= 40 + t1;  // offset  40ºC
        buffer[0]=(temp);
        temp=40 + t2;
        buffer[1]=temp;
        buffer[2]=avgTemp;
        buffer[3]=0x01;
        buffer[4]=0x02;
    }else{
        uint8_t temp= 40 + t2;
        buffer[0]=(temp);
        temp=40 + t1;
        buffer[1]=temp;
        buffer[2]=avgTemp;
        buffer[3]=0x02;
        buffer[4]=0x01;
    }
    buffer[5]=0x00; // reserved
    buffer[6]=0x00; // reserved
    buffer[7]=0x00; // reserved
    return buffer;
}

/* Mensaje resumen de celdas en bypass/Balanceando    TX FREQ:1000mSg*/
uint8_t * parseJK_message_0x03(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //TODO implementar valores reales
    buffer[0]=0x02;  //number in bypass
    buffer[1]=0x0B;  // number in initial bypass
    buffer[2]=0xF0; // number in final bypass
    buffer[3]=0x00; //reserved
    buffer[4]=0x00; //    "
    buffer[5]=0x00; //    "
    buffer[6]=0x00; //    "
    buffer[7]=0x00; //    "
    return buffer;
}

/* Mensaje de voltaje, intensidad y potencia del shunt   TX FREQ:100mSg*/
uint8_t * parseJK_message_0x04(uint8_t * buffer, JK_bms_battery_info * jkbms){
    int16_t voltaje=jkbms->battery_status.battery_voltage / 10;  //jk unit:0.01v    batrium unit 0.1V
    int16_t corriente=jkbms->battery_status.battery_current /10;  //jk unit:0.01A   batrium unit 0.1V
    int16_t potencia=jkbms->battery_status.battery_voltage * jkbms->battery_status.battery_current;  //jk 0.01V * 0.01A batrium uint 0.010mW
    uint32_t pot=(voltaje * corriente);
    pot/=1000;
    buffer[0]=(uint8_t)(voltaje & 0xFF);
    buffer[1]=(uint8_t)(voltaje >> 8);
    buffer[2]=(uint8_t)(corriente & 0xFF);
    buffer[3]=(uint8_t)(corriente >> 8);
    buffer[4]=(uint8_t)(pot & 0xFF);
    buffer[5]=(uint8_t)(pot >> 8);
    buffer[6]=0x00;
    buffer[7]=0x00;
    return buffer;
}

/* Mensaje de SOC SOH Ah_restante y  Ah_nominal  TX FREQ:1000mSg*/
uint8_t * parseJK_message_0x05(uint8_t * buffer, JK_bms_battery_info * jkbms){
    int16_t soc=jkbms->battery_status.battery_soc * 100; //jk unit 1%   batrium unit 0.01%
    int16_t soh=jkbms->battery_status.battery_soc * 100;
    uint16_t ah_nominal=jkbms->battery_cell_capacity * 100; //? 0.01ah  0.1ah
    uint16_t ah_restantes=(ah_nominal * jkbms->battery_status.battery_soc)/100;
    buffer[0]=(uint8_t)(soc & 0xFF);
    buffer[1]=(uint8_t)(soc >> 8);
    buffer[2]=(uint8_t)(soh & 0xFF);
    buffer[3]=(uint8_t)(soh >> 8);
    buffer[4]=(uint8_t)(ah_restantes & 0xFF);
    buffer[5]=(uint8_t)(ah_restantes >> 8);
    buffer[6]=(uint8_t)(ah_nominal & 0xFF);
    buffer[7]=(uint8_t)(ah_nominal >> 8);
    return buffer;
}

/* Mensaje con los límites de tensión y corriente para la carga y descarga TX FREQ:100mSg*/
uint8_t * parseJK_message_0x06(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint16_t voltaje_carga=jkbms->battery_limits.battery_charge_voltage; //jk 0.01V  batrium 0.01V
    uint16_t voltaje_descarga=jkbms->battery_limits.battery_discharge_voltage;
    uint16_t corriente_carga=jkbms->battery_limits.battery_charge_current_limit * 10; //jk 1A    batrium 0.1A
    uint16_t corriente_descarga=jkbms->battery_limits.battery_discharge_current_limit * 10;
    buffer[0]=(uint8_t)(voltaje_carga & 0xFF);
    buffer[1]=(uint8_t)(voltaje_carga >> 8);
    buffer[2]=(uint8_t)(corriente_carga & 0xFF);
    buffer[3]=(uint8_t)(corriente_carga >> 8);
    buffer[4]=(uint8_t)(voltaje_descarga & 0xFF);
    buffer[5]=(uint8_t)(voltaje_descarga >> 8);
    buffer[6]=(uint8_t)(corriente_descarga & 0xFF);
    buffer[7]=(uint8_t)(corriente_descarga >> 8);
    return buffer;
}

/* Mensaje con las baderas de control  TX FREQ:100mSg*/
uint8_t * parseJK_message_0x07(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint8_t control_critico=0x00;
    uint8_t control_carga=0x00;
    uint8_t control_descarga=0x00;
    uint8_t control_calor=0x00;
    uint8_t control_frio=0x00;
    uint8_t control_balanceoCeldas=0x00;
    control_critico=jkbms->battery_mosfet.emergencia? 0x01 : 0x00; //b0 state relay;  //b1 transition of state
    control_critico=jkbms->battery_mosfet.emergencia? control_critico | 0x04 : control_critico & 0x03; //b2 Precharge

    control_carga=jkbms->battery_mosfet.charge? 0x01 : 0x00; //b0 state relay;  //b1 transition of state
    control_carga=configuracion.habilitarCarga? control_carga & 0x03 : control_carga | 0x04; //b2 limited power evoked

    control_descarga=jkbms->battery_mosfet.discharge? 0x01 : 0x00;  //b0 state relay;  //b1 transition of state
    control_descarga=configuracion.habilitarDescarga? control_descarga & 0x03 : control_descarga | 0x04; //b2 limited power evoked

    //control_calor  //b0 state relay;  //b1 transition of state
    //control_frio   //b0 state relay;  //b1 transition of state

    control_balanceoCeldas=jkbms->battery_mosfet.balance? 0x01:0x00; //b0 state relay;  //b1 transition of state
    control_balanceoCeldas=jkbms->active_balance? control_balanceoCeldas | 0x02 : control_balanceoCeldas & 0x01;
    
    buffer[0]=control_critico;
    buffer[1]=control_carga;
    buffer[2]=control_descarga;
    buffer[3]=control_calor;
    buffer[4]=control_frio;
    buffer[5]=control_balanceoCeldas;
    buffer[6]=0x00;  //reserved
    buffer[7]=0x00;  //reserved
    return buffer;
}














uint8_t * parseJK_message_0x00111100(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00111200(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00111300(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00111500(uint8_t * buffer, JK_bms_battery_info * jkbms);

uint8_t * parseJK_message_0x00140100(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140200(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140300(uint8_t * buffer, JK_bms_battery_info * jkbms);
uint8_t * parseJK_message_0x00140400(uint8_t * buffer, JK_bms_battery_info * jkbms);
