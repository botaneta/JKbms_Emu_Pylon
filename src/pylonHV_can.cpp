#include "pylonHV_can.h"
#include "configuracion.h"

extern Config configuracion;




//------Mensajes de respuesta a la petición del host 0x4200:02 00 00 00 00 00 00 00-----------//  


/*Mensaje con información de version de hardware y software*/
uint8_t * parseJK_message_0x7310(uint8_t * buffer, JK_bms_battery_info * jkbms){
    buffer[0]=0x01; //0:null, 1:ver.A, 2:ver.B, other:reserved
    buffer[1]=0x00; //reserve
    buffer[2]=0x10; //hardware version V
    buffer[3]=0x02; //hardware version R
    buffer[4]=0x04; //software version v major
    buffer[5]=0x05; //software version v minor
    buffer[6]=0x34; //software version  
    buffer[7]=0x0C; //software version
    return buffer;
}

/*Mensaje con total celdas, número de módulos, número de celdas por modulo, tensión del conjunto y ah */
uint8_t * parseJK_message_0x7320(uint8_t * buffer, JK_bms_battery_info * jkbms){
    
    buffer[0]=jkbms->cells_number; // lsb
    buffer[1]=0x00; // msb  Número de celdas (uint16)
    buffer[2]=0x01; // número de modulos
    buffer[3]=jkbms->cells_number; // número de celdas por módulo
    uint16_t tension_nominal=TENSION_NOMINAL_CELDA_LIFEPO4 * jkbms->cells_number;
    buffer[4]=(uint8_t)(tension_nominal & 0xFF);// lsb
    buffer[5]=(uint8_t)(tension_nominal >> 8); //msb  Voltaje del conjunto uint:1V
    uint16_t capacidad=(uint16_t)jkbms->battery_cell_capacity;  //perdida de precisión 32bits to 16bits
    buffer[6]=(uint8_t)(capacidad & 0xFF); //lsb
    buffer[7]=(uint8_t)(capacidad >> 8); //msb Capacidad del conjunto uint:1AH
    return buffer;
}

/* Mensaje 1/2 con el nombre del fábricante */
uint8_t * parseJK_message_0x7330(uint8_t * buffer, JK_bms_battery_info * jkbms){
    buffer[0]='P';
    buffer[1]='Y';
    buffer[2]='L';
    buffer[3]='O';
    buffer[4]='N';
    buffer[5]='T';
    buffer[6]='E';
    buffer[7]='C';
    return buffer;
}

/* Mensaje 2/2 con el nombre del fábricante */
uint8_t * parseJK_message_0x7340(uint8_t * buffer, JK_bms_battery_info * jkbms){
    buffer[0]='H';
    buffer[1]=0x00;
    buffer[2]=0x00;
    buffer[3]=0x00;
    buffer[4]=0x00;
    buffer[5]=0x00;
    buffer[6]=0x00;
    buffer[7]=0x00;
    return buffer;
}


//------  Mensajes de respuesta a la petición del host 0x4200:00 00 00 00 00 00 00 00  -----------// 

/* Mensaje con la tensión, intensidad, temperatura, soc y soh actual del conjunto de bateria */
uint8_t * parseJK_message_0x4210(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint16_t voltaje=jkbms->battery_status.battery_voltage; //jk uint 0.01V
    voltaje= voltaje/10;
    buffer[0]=(uint8_t)(voltaje & 0xFF); //lsb
    buffer[1]=(uint8_t)(voltaje >> 8); //msb tension unit: 0.1V
    int16_t intensidad=jkbms->battery_status.battery_current;  // signed jk uint 0.01A
    intensidad=intensidad/10; //jk 0.01  pylon 0.1
    // offset= 30000 =0.0A  unit 0.1A ;29985= -1.5A 30028=2.8A
    intensidad += 30000;
    buffer[2]=(uint8_t)(intensidad & 0xFF); //lsb
    buffer[3]=(uint8_t)(intensidad >> 8); //msb corriente con offset
    int16_t temperatura=jkbms->battery_status.sensor_temperature_1; //signed uint8 uint 1ºC
    // por debajo de cero cual es el menor temperatura
    if(jkbms->battery_status.sensor_temperature_1 <= 0 || jkbms->battery_status.sensor_temperature_2 <=0){
        temperatura= 
            jkbms->battery_status.sensor_temperature_1 >= jkbms->battery_status.sensor_temperature_2 ? 
                        jkbms->battery_status.sensor_temperature_2 : jkbms->battery_status.sensor_temperature_1;
    }else{ //por encima de cero la mayor temperatura
        temperatura=
            jkbms->battery_status.sensor_temperature_1 >= jkbms->battery_status.sensor_temperature_2 ? 
                        jkbms->battery_status.sensor_temperature_1 : jkbms->battery_status.sensor_temperature_2;
    } 
    temperatura= temperatura * 10; // pylon unit 0.1ºC
    temperatura = 1000 + temperatura; // offset -100
    buffer[4]=(uint8_t)(temperatura & 0XFF); //lsb
    buffer[5]=(uint8_t)(temperatura >> 8); //msb temperatura offset = -100
    buffer[6]=jkbms->battery_status.battery_soc;
    buffer[7]=jkbms->battery_status.battery_soh;
    return buffer;
   
}

/* Mensaje de tensión maxima desconexión de carga y minima de descarga, intensidad máxima de carga y descarga */
uint8_t * parseJK_message_0x4220(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint16_t voltaje=jkbms->battery_limits.battery_charge_voltage; 
    voltaje=voltaje/10;  //jk unit 0.01V   pylon 0.1V
    buffer[0]=(uint8_t)(voltaje & 0xFF); //lsb
    buffer[1]=(uint8_t)(voltaje >> 8); //msb  voltaje corte en carga
    voltaje=jkbms->battery_limits.battery_discharge_voltage;
    voltaje=voltaje/10u;
    buffer[2]=(uint8_t)(voltaje & 0xFF); //lsb
    buffer[3]=(uint8_t)(voltaje >> 8); //msb  voltaje corte en descarga
    uint16_t intensidad=jkbms->battery_limits.battery_charge_current_limit;
    

    intensidad=30000+intensidad*10u; //jk unit 1A pylon 0.1A offset:-3000A
    buffer[4]=(uint8_t)(intensidad & 0xFF); //lsb
    buffer[5]=(uint8_t)(intensidad >> 8); //msb intensiadad maxima de carga
    intensidad=jkbms->battery_limits.battery_discharge_current_limit;
    intensidad=30000+intensidad*10u; //jk uint 1A pylon 0.1A offset:-3000A
    buffer[6]=(uint8_t)(intensidad & 0xFF); //lsb
    buffer[7]=(uint8_t)(intensidad >> 8); //msb intensiadad maxima de descarga        
    return buffer;
}

/* Mensaje de tensión mayor y menor de celda y sus números de celda*/
uint8_t * parseJK_message_0x4230(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint8_t celdaMax=jkbms->cell_number_vmax;
    uint8_t celdaMin=jkbms->cell_number_vmin;
    uint16_t voltaje=jkbms->cells_voltage[celdaMax - 1].cell_voltage;
    //jk unit 0.001V  pylon unit 0.001V
    buffer[0]=(uint8_t)(voltaje & 0xFF); //lsb
    buffer[1]=(uint8_t)(voltaje >> 8);  //msb valor de tensión del celda más alto
    voltaje=jkbms->cells_voltage[celdaMin - 1].cell_voltage;
    buffer[2]=(uint8_t)(voltaje & 0xFF); //lsb
    buffer[3]=(uint8_t)(voltaje >> 8);  //msb valor de tensión del celda más bajo
    buffer[4]=celdaMax; //lsb
    buffer[5]=0x00; //msb  número de celda tensión más alta
    buffer[6]=celdaMin;
    buffer[7]=0x00; //msb  número de celda tensión más baja
    return buffer;

}

/* Mensaje con el valor máximo y minimo de temperatura de celda y sus números*/
uint8_t * parseJK_message_0x4240(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //para adaptar la jk se establece t1 y t2 como max y min, sus números seran 1 y 2 respectivamente
    uint16_t temperatura=jkbms->battery_status.sensor_temperature_1;
    temperatura= 1000 + temperatura *10; // jk unit 1ºC  pylon 0.1ºC offset -100ºC
    buffer[0]=(uint8_t)(temperatura & 0xFF); //lsb
    buffer[1]=(uint8_t)(temperatura >> 8); //msb  temp sensor 1
    temperatura=jkbms->battery_status.sensor_temperature_2;
    temperatura= 1000 + temperatura*10;
    buffer[2]=(uint8_t)(temperatura & 0xFF); //lsb
    buffer[3]=(uint8_t)(temperatura >> 8); //msb  temp sensor 2
    buffer[4]=0x01; //lsb
    buffer[5]=0x00; //msb  sensor 1
    buffer[6]=0x02; //lsb
    buffer[7]=0x00; //msb  sensor 2
    return buffer;
}

/* Mensaje con el estado básico, ciclos, errores, alarmas y protección*/
uint8_t * parseJK_message_0x4250(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint8_t basicStatus=0x00;
    
    //b0..b2 status: 0=sleep(reposo), 1=charge, 2=discharge, 3=idle(inactiva,paro de descarga), 4..7 reserve
    //b3 0=null  1=force charge request  
    //b4 0=null  1=balance charge request
    //b5..b7 reserve
    
    //Testear corriente de bateria (+) carga, (-) descarga, (0) reposo
    if(jkbms->battery_status.battery_current > 0){
        basicStatus=0x01;
    }else if(jkbms->battery_status.battery_current < 0){
        basicStatus=0x02;
    }else{
        basicStatus=0x00;
    }
    //con cualquier alarma, excepto aviso baja capacidad b0, se envía estado inactivo
    if(jkbms->battery_alarms.alarm_data > 1) basicStatus= 0x03;
    //force charge == JK-emergency mosfet
    if(jkbms->battery_mosfet.emergencia) basicStatus = basicStatus | 0x08;
    buffer[0]=basicStatus;

    uint16_t cyclos=jkbms->battery_status.battery_cycles;
    buffer[1]=(uint8_t)(cyclos & 0xFF);  //lsb
    buffer[2]=(uint8_t)(cyclos >> 8);   //msb
    uint8_t errorStatus=0x00;
    //b7 other_error
    //b6 fault cell
    if(jkbms->battery_alarms.cell_overvoltage || jkbms->battery_alarms.cell_undervoltage)errorStatus= errorStatus | 0x40;
    //b5 relay_check_errror
    //b4 input_transposition_errror
    //b3 input voltage error
    if(jkbms->battery_alarms.charging_overvoltage) errorStatus=errorStatus | 0x08;
    //b2 internal_communication_errror
    if(configuracion.errorComunicacionJK) errorStatus=errorStatus | 0x04;
    //b1 temperature_sensor_errror
    //b0 voltage_sensor_errror
    buffer[3]=errorStatus;

    uint16_t alarmStatus=0x0000;
    //b15..b14 reserved
    //b13 fan alarm
    //b12 terminal temperature alarm (jk mosfet temp > 70ºC)
    if(jkbms->battery_status.power_tube_temperature > 70) alarmStatus = alarmStatus | 0x1000;
    
    //b11 module high voltage alarm
    //b10 module low voltage alarm
    //b9 discharge over current alarm
    if(jkbms->battery_alarms.discharging_overcurrent)alarmStatus = alarmStatus | 0x0200;
    //b8 charge over current alarm
    if(jkbms->battery_alarms.charging_overcurrent)alarmStatus = alarmStatus | 0x0100;
    
    //b7 discharge cell high temperature alarm
    if(jkbms->battery_alarms.battery_over_temperature)alarmStatus = alarmStatus | 0x0080;
    //b6 discharge cell low temperature alarm
    if(jkbms->battery_alarms.battery_low_temperature)alarmStatus = alarmStatus | 0x0040;
    //b5 charge cell high temperature alarm
    if(jkbms->battery_alarms.battery_over_temperature)alarmStatus = alarmStatus | 0x0020;
    //b4 charge cell low temperature alarm
    if(jkbms->battery_alarms.battery_low_temperature)alarmStatus = alarmStatus | 0x0010;
    
    //b3 charge system high voltage alarm
    if(jkbms->battery_alarms.charging_overvoltage)alarmStatus = alarmStatus | 0x0008; 
    //b2 discharge system low voltage alarm
    if(jkbms->battery_alarms.discharging_undervoltage)alarmStatus = alarmStatus | 0x0004;
    //b1 single cell high voltage alarm
    if(jkbms->battery_alarms.cell_overvoltage)alarmStatus = alarmStatus | 0x0002;
    //b0 single cell low voltage alarm
    if(jkbms->battery_alarms.cell_undervoltage)alarmStatus = alarmStatus | 0x0001;
   
    buffer[4]=(uint8_t)(alarmStatus & 0xFF);  //lsb
    buffer[5]=(uint8_t)(alarmStatus >> 8);   //msb

    uint16_t protecctionStatus=0x0000;
    //b15..b13 reserved
    //b12 Battery cell secondary undervoltage protection
    //b11 Battery module overvoltage protection
    //b10 Battery module undervoltage protection
    //b9  Discharge overcurrent protection
    //b8  Charging overcurrent protection
    //b7  Discharge cell over temperature protection
    //b6  Discharge cell under temperature protection
    //b5  Charging cell over temperature protection
    //b4  Charging cell under temperature protection
    //b3  Charging system over voltage protection
    //b2  Discharging system under voltage protection
    //b1  Single cell over voltage protection
    //b0  Single cell under voltage protection



    buffer[6]=(uint8_t)(protecctionStatus & 0xFF);  //lsb
    buffer[7]=(uint8_t)(protecctionStatus >> 8);   //msb

    return buffer;    
}

/**Mensaje con el valor de mayor tensión, menor tensión y el número de módulo al que corresponde*/
uint8_t * parseJK_message_0x4260(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //mayor tensión uint 0.001V
    uint16_t voltage=jkbms->battery_status.battery_voltage; 
    voltage*=10;  //jk 0.01V  pylon 0.001V
    buffer[0]=(uint8_t)(voltage & 0xFF); //lsb
    buffer[1]=(uint8_t)(voltage >> 8); //msb
    //menor tension
    buffer[2]=buffer[0];
    buffer[3]=buffer[1];
    //número módulo mayor tensión
    buffer[4]=0x01;
    buffer[5]=0x00;
    //número módulo menor tensión
    buffer[6]=0x01;
    buffer[7]=0x00;

    //TODO probar otros valores de módulos    
    return buffer;
}

/**Mensaje con el valor de mayor temperatura, menor temperatura y el número de módulo al que corresponde*/
uint8_t * parseJK_message_0x4270(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //offeset -100ºC  resolution pylon  0.1ºC   resolution jk 1ºC signed
    uint16_t sensor1=1000 + jkbms->battery_status.sensor_temperature_1 * 10;
    uint16_t sensor2=1000 + jkbms->battery_status.sensor_temperature_2 * 10;
    if(sensor1 >=sensor2){
        // módulo mayor temp
        buffer[0]=(uint8_t) (sensor1 & 0xFF); //lsb
        buffer[1]=(uint8_t) (sensor1 >> 8); //msb
        // módulo menor temp
        buffer[2]=(uint8_t) (sensor2 & 0xFF); //lsb
        buffer[3]=(uint8_t) (sensor2 >> 8); //msb
        //número de modulo con mayor y menor temp
        buffer[4]=0x01;  //sensor 1
        buffer[5]=0x00;
        buffer[6]=0x02;  //sensor 2
        buffer[7]=0x00;

    }else{
        // módulo mayor temp
        buffer[0]=(uint8_t) (sensor2 & 0xFF); //lsb
        buffer[1]=(uint8_t) (sensor2 >> 8); //msb
        // módulo menor temp
        buffer[2]=(uint8_t) (sensor1 & 0xFF); //lsb
        buffer[3]=(uint8_t) (sensor1 >> 8); //msb
        //número de modulo con mayor y menor temp
        buffer[4]=0x02;
        buffer[5]=0x00;
        buffer[6]=0x01;
        buffer[7]=0x00;
    }
    return buffer;
}

/**Mensaje con permisos de carga y descarga*/
uint8_t * parseJK_message_0x4280(uint8_t * buffer, JK_bms_battery_info * jkbms){
    uint8_t prohibir=0xAA;
    uint8_t permitir=0x00;
    uint8_t carga= configuracion.habilitarCarga? permitir : prohibir;
    uint8_t descarga= configuracion.habilitarDescarga? permitir : prohibir;
    if(configuracion.errorComunicacionJK){
        carga=prohibir;
        descarga=prohibir;
    }
    buffer[0]=carga;
    buffer[1]=descarga;
    buffer[2]=0x00; //reserve
    buffer[3]=0x00; //reserve
    buffer[4]=0x00; //reserve
    buffer[5]=0x00; //reserve
    buffer[6]=0x00; //reserve
    buffer[7]=0x00; //reserve
    return buffer;
}

/**Mensaje extensión de la lista de errores del sistema*/
uint8_t * parseJK_message_0x4290(uint8_t * buffer, JK_bms_battery_info * jkbms){ 
    //b0  shutdown circuit error
    //b1  BMIC error
    //b2  internal bus error
    //b3  self-test error
    //b4  chip error
    //b5..b7 reserve
    uint8_t errorList1=0x00;
    if(configuracion.errorComunicacionJK) errorList1= errorList1 | 0x04;
    buffer[0]=errorList1;
    buffer[1]=0x00;
    buffer[2]=0x00;
    buffer[3]=0x00;
    buffer[4]=0x00;
    buffer[5]=0x00;
    buffer[6]=0x00;
    buffer[7]=0x00;
    return buffer;
}

/**Mensaje con las temperatura max y min de terminal y sus número de  módulo*/
uint8_t * parseJK_message_0x42A0(uint8_t * buffer, JK_bms_battery_info * jkbms){
    //para adaptar la jk se establece la temperatura mosfet como max y min, sus números seran 1 y 2 
    uint16_t temperatura=jkbms->battery_status.power_tube_temperature;
    temperatura= 1000 + temperatura *10; // jk unit 1ºC  pylon 0.1ºC offset -100ºC
    buffer[0]=(uint8_t)(temperatura & 0xFF); //lsb
    buffer[1]=(uint8_t)(temperatura >> 8); //msb  temp sensor 1
    buffer[2]=(uint8_t)(temperatura & 0xFF); //lsb
    buffer[3]=(uint8_t)(temperatura >> 8); //msb  temp sensor 2
    buffer[4]=0x01; //lsb
    buffer[5]=0x00; //msb  sensor 1
    buffer[6]=0x02; //lsb
    buffer[7]=0x00; //msb  sensor 2
    return buffer;
}

