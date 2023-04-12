#include "pylon_can.h"






/**
 * @brief Envio de bits de proteccion, alarmas y número de módulos de la batería
 * 
 * @param jk_bms_battery_info 
 * @return uint8_t* trama 8 bytes
 */
uint8_t * parseJK_message_0x359(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info){ 

    uint8_t data=0x00;
    //byte 0 protection table1
    if(jk_bms_battery_info->battery_alarms.discharging_overcurrent) data = data | 0x80;
    if(jk_bms_battery_info->battery_alarms.battery_low_temperature) data = data | 0x10;
    if(jk_bms_battery_info->battery_alarms.battery_over_temperature || 
        jk_bms_battery_info->battery_alarms.overtemperature_alarm_battery_box ||
        jk_bms_battery_info->battery_alarms.power_tube_overtemperature) data = data | 0x08;
    if(jk_bms_battery_info->battery_alarms.cell_undervoltage)       data = data | 0x04;
    if(jk_bms_battery_info->battery_alarms.cell_overvoltage)        data = data | 0x02;
    buffer[0]=data;
    
    //byte 1 protection table2
    data=0x00;
    if(configuracion.errorComunicacionJK) data = data | 0x08; // system error bit3
    if(jk_bms_battery_info->battery_alarms.charging_overcurrent) data = data | 0x01;
    buffer[1]=data;

    //bye 2 alarm table3
    data=0x00;
    if(jk_bms_battery_info->battery_alarms.discharging_overcurrent)data = data | 0x80;
    if(jk_bms_battery_info->battery_alarms.battery_low_temperature)data = data | 0x10;
    if(jk_bms_battery_info->battery_alarms.battery_over_temperature || 
        jk_bms_battery_info->battery_alarms.overtemperature_alarm_battery_box ||
        jk_bms_battery_info->battery_alarms.power_tube_overtemperature) data = data | 0x08;
    if(jk_bms_battery_info->battery_alarms.cell_undervoltage) data = data | 0x04;
    if(jk_bms_battery_info->battery_alarms.cell_overvoltage) data = data | 0x02;
    buffer[2]=data;

    //byte 3 alarm table4
    data=0x00;
    if(configuracion.errorComunicacionJK) data = data | 0x08; // system error bit3
    if(jk_bms_battery_info->battery_alarms.charging_overcurrent )data = data | 0x01;
    buffer[3]=data;
    
    //byte4 numero de modulos
    buffer[4]=0x01;
    //byte5 "P"
    buffer[5]=0x50;
    //byte5 "N"
    buffer[6]=0X4E;
    buffer[7]=0x00;
    return buffer;
}

/**
 * @brief Envío de tensión de carga  y límites de corriente de carga descarga
 * 
 * @param jk_bms_battery_info 
 * @return uint8_t* trama de 8 bytes
 */
uint8_t * parseJK_message_0x351(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info){ 
    // byte0 es el byte menos significativo y el byte1 el más significativo
    // tensión maxima de carga 
    uint16_t voltage=jk_bms_battery_info->battery_limits.battery_charge_voltage;
    voltage=voltage/10;  //jk unit 0.01V  pylon-can unit0.1V
    buffer[0]= (uint8_t)(voltage & 0xFF);
    buffer[1]=(uint8_t) (voltage >> 8 ); //desplazar byte superior

    //byte2-3 charge current limit signed 
    //jk unit 1A   pylon unit 0.1A signed
    int16_t current=jk_bms_battery_info->battery_limits.battery_charge_current_limit*10;
    buffer[2]= (uint8_t)(current & 0xFF);
    buffer[3]=(uint8_t) (current >> 8 ); //desplazar byte superior

    //byte4-5 discharge current limit signed 
    //jk unit 1A   pylon unit 0.1A signed
    current=jk_bms_battery_info->battery_limits.battery_discharge_current_limit*-10;
    buffer[4]= (uint8_t)(current & 0xFF);
    buffer[5]=(uint8_t) (current >> 8 ); //desplazar byte superior

    buffer[6]=0x00;
    buffer[7]=0x00;
    return buffer;
}

/**
 * @brief Envío de estado actual de carga (SOC) y salud (SOH)
 * 
 * @param jk_bms_battery_info 
 * @return uint8_t* trama 8 bytes
 */
uint8_t * parseJK_message_0x355(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info){ 
   
    uint16_t data=0;;
    data=jk_bms_battery_info->battery_status.battery_soc;
    buffer[0]= (uint8_t)data & 0x00FF;
    buffer[1]=(uint8_t) data >> 8 ; //desplazar byte superior
    data=jk_bms_battery_info->battery_status.battery_soh;
    buffer[2]=(uint8_t)data & 0x00FF;
    buffer[3]=(uint8_t)data >> 8 ; //desplazar byte superior
    buffer[4]=0x00;
    buffer[5]=0x00;
    buffer[6]=0x00;
    buffer[7]=0x00;
    return buffer;
}

/**
 * @brief Envío de tensión, corriente y temperatura de la batería
 * 
 * @param jk_bms_battery_info 
 * @return uint8_t* trama 8bytes
 */
uint8_t * parseJK_message_0x356(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info){ 
   
    int16_t data=jk_bms_battery_info->battery_status.battery_voltage;  //parseo uint to int????
    buffer[0]=(uint8_t) (data & 0xFF);
    buffer[1]=(uint8_t) (data >> 8);
    //monitorizacion en ingeteam valor negativo es carga de bateria
    //monitorizacion jk valor positivo es carga de bateria (valor jk 0.01A   pylon 0.1A)
    data=(int16_t)(jk_bms_battery_info->battery_status.battery_current / 10);  
    buffer[2]=(uint8_t)(data & 0xFF);
    buffer[3]=(uint8_t)(data >> 8);
    // TODO tº mosfet  
    // t1-bat   t2-bat  -40ºC 0 100ºC   unit 1ºC
    // pylon unit 0.1ºC
    data=jk_bms_battery_info->battery_status.sensor_temperature_1;
    data += jk_bms_battery_info->battery_status.sensor_temperature_2;
    data = data/2;
    data = data *  10;
    buffer[4]= (uint8_t)(data & 0xFF);
    buffer[5]=(uint8_t) (data >> 8 ); //desplazar byte superior
    buffer[6]=0x00;
    buffer[7]=0x00;
    return buffer;
}



/**
 * @brief Envio de banderas para carga y descarga como forzar la carga
 * 
 * @param jk_bms_battery_info 
 * @return uint8_t* trama 2bytes
 */
uint8_t * parseJK_message_0x35C(uint8_t * buffer2, JK_bms_battery_info *jk_bms_battery_info){
    
    uint8_t chargeEnable=0x80;
    uint8_t dischargeEnable=0x40;
    uint8_t data=0x00;
    if(configuracion.habilitarCarga) data = data | chargeEnable;
    if(configuracion.habilitarDescarga) data = data | dischargeEnable;
    if(configuracion.errorComunicacionJK) data = 0x00; // si no hay comunicación se para carga/descarga
    buffer2[0]=data; 
    buffer2[1]=0x00;   
    return buffer2;
 }


/** Envio de mensaje con el nombre de la Marca*/
uint8_t * parseJK_message_0x35E(uint8_t * buffer, JK_bms_battery_info *jk_bms_battery_info){ 
    buffer[0]='P';    
    buffer[1]='Y';
    buffer[2]='L';
    buffer[3]='O';
    buffer[4]='N';
    buffer[5]='e';
    buffer[6]='m';
    buffer[7]='u';
    return buffer;

}
