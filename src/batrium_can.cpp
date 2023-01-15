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













/* Live telemetry cell voltage limits data length 6bytes  TX FREQ:100mSg */
uint8_t * parseJK_message_0x00111100(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //Min cell voltage uint16_t unit 1mV
    buffer[0]=(uint8_t)(jkbms->battery_limits.cell_min_voltage & 0xFF);  //lsb
    buffer[1]=(uint8_t)(jkbms->battery_limits.cell_min_voltage >> 8);  //msb
    //max cell voltage
    buffer[2]=(uint8_t)(jkbms->battery_limits.cell_max_voltage & 0xFF);
    buffer[3]=(uint8_t)(jkbms->battery_limits.cell_max_voltage >> 8);
    //min voltage nCell
    buffer[4]=10;
    //max voltage nCell
    buffer[5]=15;
    return buffer;
}

/* Live telemetry cell temperature limits  data length 8bytes  TX FREQ:1000mSg */
uint8_t * parseJK_message_0x00111200(uint8_t * buffer, JK_bms_battery_info * jkbms){
    // min cell temperature 
    buffer[0]=0 + 40; //0ºC + 40 offset
    //max cell temperature
    buffer[1]=70 + 40;
    //min temp nCell
    buffer[2]=5;
    //max temp nCell
    buffer[3]=11;
    //min bypass Temperature
    buffer[4]=0 + 40; //0ºC + 40 offset
    //max bypass Temperature
    buffer[5]=70 + 40;
    //min bypassTemperature nCell
    buffer[6]=6;
    //max bypassTemperature nCell
    buffer[7]=12;
     return buffer;
}

/* Live telemetry Cell bypass limits  data length 7bytes TX FREQ:1000mSg */
uint8_t * parseJK_message_0x00111300(uint8_t * buffer, JK_bms_battery_info * jkbms){
    // min bypass PWM % unit 1% 0-100
    buffer[0]=1;
    // max bypass PWM % unit 1%  0-100
    buffer[1]=100;
    //Min bypass nCell
    buffer[2]=0;
    //Max bypass nCell
    buffer[3]=24;
    //number in initial bypass
    buffer[4]=2;
    //number in final bypass
    buffer[5]=4;
    //number in bypass
    buffer[6]=3;
    return buffer;
}

/* Live telemetry shunt power monitoring data length 8byte  TX FREQ:100mSg */
uint8_t * parseJK_message_0x00111500(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //SOC -10% to 110%  offset 10%  unit 0.5%
    buffer[0]=jkbms->battery_status.battery_soc;
    //Shunt temperature -40 to 125ºC  offset 40ºC unit 1ºC
    buffer[1]=jkbms->battery_status.power_tube_temperature + 40 ;
    //shunt voltage uint16_t unit 0.01V
    buffer[2]=(uint8_t)(jkbms->battery_status.battery_voltage & 0xFF);
    buffer[3]=(uint8_t)(jkbms->battery_status.battery_voltage >> 8);
    //Shunt amperes float unit 1mA
    float current=jkbms->battery_status.battery_current * 1000;
    buffer[4]=(uint8_t)((uint32_t)current & 0xFF); 
    buffer[5]=(uint8_t)((((uint32_t)current) >> 8) & 0xFF);
    buffer[6]=(uint8_t)((((uint32_t)current) >> 16) & 0xFF);
    buffer[7]=(uint8_t)((((uint32_t)current) >> 24) & 0xFF);
    return buffer;
}

/* Control logic Critical Warning state data length 8bytes  TX FREQ:1000mSg */
uint8_t * parseJK_message_0x00140100(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //Critical control mode 0x00=auto, 0x01=Manual_ON, 0x02=Manual_OFF
    buffer[0]=0x00;
    //Critical control flags b0=OnState(relay state), b1=OnCalc(live analysis), b2=input run state, b3=has overdue cell sensor
    buffer[1]=0x00;
    //Critical monitor flags1 b0=has low cell voltage, b1=has high cell voltage, b2=has low cell temperature, b3=has hight cell temp
    //                      b4=has low supply voltage,  b5=has hight supply voltage, b6=has low ambient temperature, b7=has hight ambient temperature
    uint8_t monitor=0x00;
    if(jkbms->battery_alarms.cell_undervoltage) monitor = monitor| 0x01;
    if(jkbms->battery_alarms.cell_overvoltage)monitor = monitor | 0x02;
    if(jkbms->battery_alarms.battery_low_temperature) monitor = monitor | 0x04;
    if(jkbms->battery_alarms.battery_over_temperature) monitor = monitor | 0x08;
    if(jkbms->battery_alarms.discharging_undervoltage) monitor = monitor | 0x10;
    if(jkbms->battery_alarms.charging_overvoltage) monitor = monitor | 0x20;
    buffer[2]=monitor;


    //Critical control flags2 b0=has low shunt voltage, b1=has hight shunt voltage, b2=has low idle voltage, 
    //                          b3=has max charging current, b4=has max discharging current
    uint8_t monitor2=0x00;
    if(jkbms->battery_alarms.charging_overcurrent)monitor2 = monitor2 | 0x08;
    if(jkbms->battery_alarms.discharging_overcurrent)monitor2 = monitor2 | 0x10;
    buffer[3]=monitor2;

    //Warning Control mode  0x00=auto, 0x01=Manual_ON, 0x02=Manual_OFF
    buffer[4]=0x00;

    //Warning Control flags b0=OnState(relay state), b1=OnCalc(live analysis)
    buffer[5]=0x00;

    //Warning control flags1 b0=has low cell voltage, b1=has high cell voltage, b2=has low cell temp,  b3=has hight cell temp
    //                       b4=has low supply voltage, b5=has hight supply voltage, b6=has low ambient temp, b7=has hight ambient temp
    buffer[6]=0x00;

    //Warning control flags2 b=0has low shunt voltage, b1=has hight shunt voltage, b2=has low shunt SOC, b3=has hight shunt SOC
    //                       b4=has max charging current, b5=has max discharging current
    buffer[7]=0x00;
    return buffer;
}

/* Control logic thermal state data length 4bytes TX FREQ:1000mSg*/
uint8_t * parseJK_message_0x00140200(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //Head control mode  0x00=auto, 0x01=Manual_ON, 0x02=Manual_OFF
    buffer[0]=0x00;
    //Head contol flags b0=OnState(relay state), b1=OnCalc(live analysis), b2=has low cell temperature, b3=has low ambient temp
    buffer[1]= jkbms->battery_alarms.battery_low_temperature? 0x0C : 0x00;

    //Cool control mode  0x00=auto, 0x01=Manual_ON, 0x02=Manual_OFF
    buffer[2]=0x00;

    //Cool control flags  b0=OnState(relay state), b1=OnCalc(live analysis), b2=has high cell temperature, 
    //                    b3=has high ambient temp,  b4=has cells in bypass
    buffer[3]= jkbms->battery_alarms.battery_over_temperature? 0x0C : 0x00;
    return buffer;
}


/* Control Logic - Charging state data length 8byte  TX FREQ:1000mSg */
uint8_t * parseJK_message_0x00140300(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //charge control mode  0x00=auto, 0x01=Manual_ON, 0x02=Manual_OFF
    buffer[0]=0x00; //configuracion.habilitarCarga? 0x01 : 0x02;
    //charge control flags   b0=OnState(relay state), b1=OnCalc(live analysis), b2=low power evoked, b3=has bypass overheated
    //                       b4=has bypass above initial, b5=has bypass above final, b6=has cells in bypass, b7=cells are fully charged                         
    // TODO: implementar todo los bits
    buffer[1]=0x01;
    //Target current uint16_t  unit_jk 1A    unit_batrium 0.1A
    uint16_t current=jkbms->battery_limits.battery_charge_current_limit * 10;
    buffer[2]=(uint8_t)(current & 0xFF);
    buffer[3]=(uint8_t)(current >> 8);
    //Target powerVA uint16_t  unit 1VA
    uint16_t voltage=jkbms->battery_limits.battery_charge_voltage / 100; // 10.01V -> 10V
    uint16_t power=voltage * jkbms->battery_limits.battery_charge_current_limit;
    buffer[4]=(uint8_t)(power & 0xFF);
    buffer[5]=(uint8_t)(power >> 8);
    //Target Voltage uint16_t unit_jk 0.01V  unit_batrium 0.01V
    buffer[6]=(uint8_t)(jkbms->battery_limits.battery_charge_voltage & 0xFF);
    buffer[7]=(uint8_t)(jkbms->battery_limits.battery_charge_voltage >> 8);
    return buffer;
}

/* Control Logic – Discharging state   dataLength 6byte  TX FREQ:1000mSg*/
uint8_t * parseJK_message_0x00140400(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //discharge control mode  0x00=auto, 0x01=Manual_ON, 0x02=Manual_OFF
    buffer[0]=0x00;
    //discharge control flags  b0=OnState(relay state), b1=OnCalc(live analysis), b2=low power evoked
    // TODO: implementar todos los bits
    buffer[1]=0x01;
    //Target current uint16_t  unit_jk 1A    unit_batrium 0.1A
    uint16_t current=jkbms->battery_limits.battery_discharge_current_limit * 10;
    buffer[2]=(uint8_t)(current & 0xFF);
    buffer[3]=(uint8_t)(current >> 8);
    //Target powerVA uint16_t  unit 1VA
    uint16_t voltage=jkbms->battery_limits.battery_discharge_voltage / 100; // 10.01V -> 10V
    uint16_t power=voltage * jkbms->battery_limits.battery_discharge_current_limit;
    buffer[4]=(uint8_t)(power & 0xFF);
    buffer[5]=(uint8_t)(power >> 8);
    return buffer;
}

/* Control Logic */
uint8_t * parseJK_message_0x00140500(uint8_t * buffer, JK_bms_battery_info * jkbms){


    return buffer;
}
