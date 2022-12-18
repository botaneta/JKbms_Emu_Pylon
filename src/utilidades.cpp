#include <Arduino.h>
#include "jk_bms_485.h"
#include <StreamString.h>
#include <string.h>
#include <sstream>







void imprimeJKBMSinfo(const struct JK_bms_battery_info &jkbms){
    Serial.println("\n\n");
    Serial.println("######################   INFO   ##########################");
    Serial.printf("Voltage:%.2fV\t Current:%.2fA\t SOC:%d%\n", 
                    (float_t)jkbms.battery_status.battery_voltage/100, 
                    (float_t)jkbms.battery_status.battery_current/100,
                    jkbms.battery_status.battery_soc);
    Serial.println("----------------------------------------------------------");
    Serial.printf("Temp: %dºC\tT1: %dºC\tT2: %dºC\n ", jkbms.battery_status.power_tube_temperature,
                        jkbms.battery_status.sensor_temperature_1, 
                        jkbms.battery_status.sensor_temperature_2
                        );
    Serial.println("----------------------------------------------------------");
    Serial.printf("Capacity: %dAh\t Cycles count: %d   ID:%08x\n",
                        jkbms.battery_status.battery_cycle_capacity,
                        jkbms.battery_status.battery_cycles,
                        jkbms.ID
                        );
    Serial.println("###################  STATUS CELLS  ########################");
    float voltagemedio=0;
    for(int i=0; i<jkbms.cells_number; i++){
        voltagemedio+=jkbms.cells_voltage[i].cell_voltage;
    }
    voltagemedio=voltagemedio/jkbms.cells_number;
    Serial.printf("NºCells:%d, voltage_averange:%.2fV\n ",
                    jkbms.cells_number, voltagemedio/1000);
    
    for(int i=0; i<jkbms.cells_number; i++){
        if(i%4==0 && i/4>=1){Serial.println();}

        Serial.printf("cell_%d  %.3fVol  \t", 
                jkbms.cells_voltage[i].cell_number,
                (float_t)jkbms.cells_voltage[i].cell_voltage/1000);
    }
    Serial.println();
    Serial.println("####################  LIMITS #############################");
    Serial.printf("Charge: %.2fVol, %dAmps\nDischarge:  %.2fVol, %dAmps\n",
            (float_t)jkbms.battery_limits.battery_charge_voltage/100,
            jkbms.battery_limits.battery_charge_current_limit,
            (float_t)jkbms.battery_limits.battery_discharge_voltage/100,
            jkbms.battery_limits.battery_discharge_current_limit);
    Serial.println("################   ALARMS  ##############################");
    Serial.printf("low capacity||overhead||OVCharge||UVDischar||overTempcells||\n");
    Serial.printf("      %d         %d        %d        %d             %d     \n ",
            jkbms.battery_alarms.low_capacity,
            jkbms.battery_alarms.power_tube_overtemperature,
            jkbms.battery_alarms.charging_overvoltage,
            jkbms.battery_alarms.discharging_undervoltage,
            jkbms.battery_alarms.battery_over_temperature );   
    
}


String toStringJkBms(JK_bms_battery_info * jkbms){
    char buffer[256];
    int sizeBuffer=0;
    String texto="######################   INFO   ##########################\n";
    sizeBuffer=sprintf(buffer, "Voltage:%.2fV\t Current:%.2fA\t SOC:%d% \tID:%08x\n", 
                        (float_t)jkbms->battery_status.battery_voltage/100, 
                        (float_t)jkbms->battery_status.battery_current/100,
                                 jkbms->battery_status.battery_soc,
                                 jkbms->ID);
    texto += String(buffer, sizeBuffer); 
    texto += "----------------------------------------------------------\n";
    sizeBuffer=sprintf(buffer, "Temp: %dºC\t\tT1: %dºC\t\tT2: %dºC\n ", 
                        jkbms->battery_status.power_tube_temperature,
                        jkbms->battery_status.sensor_temperature_1, 
                        jkbms->battery_status.sensor_temperature_2
                        );
    texto += String(buffer, sizeBuffer); 
    texto += "----------------------------------------------------------\n";
    sizeBuffer=sprintf(buffer, "Capacity: %dAh\t\t Cycles count: %d\n",
                        jkbms->battery_status.battery_cycle_capacity,
                        jkbms->battery_status.battery_cycles
                        );
    texto += String(buffer, sizeBuffer);
    texto += "###################  STATUS CELLS  ########################\n";
    float voltajemedio=0;
    for(int i=0; i<jkbms->cells_number; i++){
        voltajemedio+=jkbms->cells_voltage[i].cell_voltage;
    }
    voltajemedio=voltajemedio/jkbms->cells_number;
    sizeBuffer=sprintf(buffer, "NºCells:%d, \t\tVoltage_averange:%.2fV\n ",
                        jkbms->cells_number, voltajemedio/1000
                        );
    texto += String(buffer, sizeBuffer);
    texto += "-  -  -  -  -  -  -  -   -  -  -  -  -  -  -  -  -  -  -  -\n";
    for(int i=0; i<jkbms->cells_number; i++){
        if(i%4==0 && i/4>=1){ //cambiar de línea cada 4celdas
            texto += "\n";
        }
        sizeBuffer=sprintf(buffer, "Cell_%d  %.3fVol  \t", 
                            jkbms->cells_voltage[i].cell_number,
                            (float_t)jkbms->cells_voltage[i].cell_voltage/1000);
        texto += String(buffer, sizeBuffer);                    
    }
    texto += "\n####################  LIMITS #############################\n";
    sizeBuffer=sprintf(buffer, "Charge: %.2fVol, %dAmps\nDischarge:  %.2fVol, %dAmps\n",
                    (float_t)jkbms->battery_limits.battery_charge_voltage/100,
                             jkbms->battery_limits.battery_charge_current_limit,
                    (float_t)jkbms->battery_limits.battery_discharge_voltage/100,
                             jkbms->battery_limits.battery_discharge_current_limit
                      );
    texto += String(buffer, sizeBuffer);
    texto += "#####################   ALARMS  ###########################\n";
    texto += "low bat||overtempmosfet||OVCharge||UVDischar||overTemp||lowtemp||UVCell||OVCell||DeltaCell\n";
    sizeBuffer=sprintf(buffer, 
             "   %d         %d             %d          %d         %d          %d       %d      %d        %d\n ",
                            jkbms->battery_alarms.low_capacity,
                            jkbms->battery_alarms.power_tube_overtemperature,
                            jkbms->battery_alarms.charging_overvoltage,
                            jkbms->battery_alarms.discharging_undervoltage,
                            jkbms->battery_alarms.battery_over_temperature,
                            jkbms->battery_alarms.battery_low_temperature,
                            jkbms->battery_alarms.cell_undervoltage,
                            jkbms->battery_alarms.cell_overvoltage,
                            jkbms->battery_alarms.cell_pressure_difference
                        );
    texto += String(buffer, sizeBuffer);
    texto += "######################   MOSFET   ###########################\n";
    texto += " CHARGE  ||  DISCHARGE  ||  BALANCE  ||  EMERGENCY? \n";
    sizeBuffer=sprintf(buffer, 
             "   %d            %d             %d           %d",
                            jkbms->battery_mosfet.charge,
                            jkbms->battery_mosfet.discharge,
                            jkbms->battery_mosfet.balance,
                            jkbms->battery_mosfet.emergencia
                        );
    texto += String(buffer, sizeBuffer);                    
    texto += "\n\n";                    
    return texto;
}



/** Parsea texto hexadecimal en un byte */
uint8_t parseByteToText(char * textoHexadecimal){
    unsigned int entero;
    std::istringstream iss(textoHexadecimal);
    iss >> std::hex >>  entero;
    return (uint8_t) entero; 
}   


/**Funcion auxiliar para enviar tramas de datos por el Terminal del pc simulando el rs485 de jkbms */
void parseaTextoTobytes(char * ptr_cadena, uint8_t * bufferdatos){
    int indice=0;
    char * token=NULL ;
    //Separacion de elementos por token de separacion :
    token = strtok(ptr_cadena,":");  
    while( token !=NULL){
        bufferdatos[indice]=parseByteToText(token);
        indice++;   
        token = strtok(NULL,":");
    }
  //Serial.printf("Número de bytes de la trama: %d\n", indice);
}


/** Concatena string con el buffer de 8bytes formateados a hexdecimal y devuleve el string 
 * utilidad para representar mensajes CAN
*/
String bufferToString(String text, uint8_t * buffer){
    int size;
    char cadena[64]{};
    size=sprintf(cadena,  "%02x %02x %02x %02x %02x %02x %02x %02x", buffer[0], buffer[1],
                                    buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]); 
    text+=String(cadena, size);
    return text;      
}






