
#include <Arduino.h>
#include "jk_bms_485.h"
#include  <string.h>
#include "configuracion.h"


extern Config configuracion;


// Status request
  // -> 0x4E 0x57 0x00 0x13 0x00 0x00 0x00 0x00 0x06 0x03 0x00 0x00 0x00 0x00 0x00 0x00 0x68 0x00 0x00 0x01 0x29
  //
  // Status response
  // <- 0x4E 0x57 0x01 0x1B 0x00 0x00 0x00 0x00 0x06 0x00 0x01: Header
  //
  // *Data*
  //
  // Address Content: Description      Decoded content                         Coeff./Unit
  // 0x79: Individual Cell voltage
  // 0x2A: Cell count               42 / 3 bytes = 14 cells
  // 0x01 0x0E 0xED: Cell 1         3821 * 0.001 = 3.821V                        0.001 V
  // 0x02 0x0E 0xFA: Cell 2         3834 * 0.001 = 3.834V                        0.001 V
  // 0x03 0x0E 0xF7: Cell 3         3831 * 0.001 = 3.831V                        0.001 V
  // 0x04 0x0E 0xEC: Cell 4         ...                                          0.001 V
  // 0x05 0x0E 0xF8: Cell 5         ...                                          0.001 V
  // 0x06 0x0E 0xFA: Cell 6         ...                                          0.001 V
  // 0x07 0x0E 0xF1: Cell 7         ...                                          0.001 V
  // 0x08 0x0E 0xF8: Cell 8         ...                                          0.001 V
  // 0x09 0x0E 0xE3: Cell 9         ...                                          0.001 V
  // 0x0A 0x0E 0xFA: Cell 10        ...                                          0.001 V
  // 0x0B 0x0E 0xF1: Cell 11        ...                                          0.001 V
  // 0x0C 0x0E 0xFB: Cell 12        ...                                          0.001 V
  // 0x0D 0x0E 0xFB: Cell 13        ...                                          0.001 V
  // 0x0E 0x0E 0xF2: Cell 14        3826 * 0.001 = 3.826V                        0.001 V








uint8_t request_Status_Frame[] = {0x4E, 0x57,  //Start Frame
 								  0x00, 0x13,  //legth frame   (19bytes apartir de startframe)
								  0x00, 0x00, 0x00, 0x00,  //bms terminal number (4byte ID)
								  0x06,     //Command world   (0x06 read all data)
								  0x03,     //frame source (0.bms, 1.bluetooth, 2.GPS, 3.PC upper computer)
								  0x00,     //Transmision type (0.read data, 1.reply frame, 2.BMS active upload)
								  0x00, 
								  0x00, 0x00, 0x00, 0x00,  //Record number  (El 1 byte superior es código aleatorio,sin sentido (reservado para el cifrado),
															//y los 3 bytes inferiores son código aleatorio número de registro)
								  0x68,  //End identification
								  0x00, 0x00, 0x01, 0x29};  //Checksum  Accumulated Checksum (High 2 bytes for CRC not enabled fill 0, Low 2

uint8_t response_Status_header_start_frame[]={0x4E, 0x57};

int16_t getCurrent(const uint16_t value) {
	// TODO test current data on real BMS
	if ((value & 0x8000) == 0x8000) {
		return (value & 0x7FFF);
	} else {
		return (value & 0x7FFF) * -1;
	}
}

int8_t getTemperature(const int16_t value) {
   if (value > 100)
     return (100 - (int16_t) value);

   return value;
};

/**
 * @brief Solicitud de datos a la bms JK
 * 
 */
void Request_JK_Battery_485_Status_Frame() {	
	
	for(int i=0; i<21; i++){
		Serial2.write(request_Status_Frame[i]);
		//Serial.printf(" %2x ", request_Status_Frame[i]);
	}
	Serial2.flush();
	//Serial.flush();
}




bool JK_Battery_485_Check_Frame_CRC(uint8_t *data, uint16_t frame_size) {
	if (frame_size < 4) {
		return false;
	}else if(frame_size < 23){ // si es una respuesta a petición de escritura de parametro
		return false;
	}
	
	uint16_t data_len = (uint16_t)data[2] << 8 | data[2 + 1];
	uint16_t computed_crc = 0;

	for (uint16_t i = 0; i < data_len-2; i++) {  
		computed_crc = computed_crc + data[i];
	}
	
	uint16_t remote_crc = (uint16_t)data[data_len] << 8 | data[data_len + 1];
	if (computed_crc == remote_crc) {
		return true;
	}
	return false;
}


bool  JK_bms_check_header(uint8_t  *data){
	uint16_t sizedata=0;
	try{
		sizedata=data[2] << 8 | data[3];
	}catch(...){
		Serial.println("Puntero nulo en trama data");
		return false;
	}
	
	if(data[0]==response_Status_header_start_frame[0]  
		&& data[1]==response_Status_header_start_frame[1] 
		&& sizedata >21 ){  //solo trama larga, ignorar respuesta por escrituras
			return true;
	}
	// no es la trama que espero, muestrala
	if(configuracion.comunicarSerialDebug2){
		Serial.print("\nTRAMA RESPUESTA JK");
		for(int i=0; i<sizedata; i++){
			Serial.printf(": %02x", data[i]);
			if(i > 128)break;
		}
	}
	
	return false;
}


uint16_t cell_average_voltage(){
	uint32_t sumaVoltajes=0;
	int numeroCeldas=jk_bms_battery_info.cells_number;
	for(int i=0; i< numeroCeldas; i++){
		sumaVoltajes += jk_bms_battery_info.cells_voltage[i].cell_voltage;
	}
	return sumaVoltajes/numeroCeldas;
}

uint16_t delta_cell_voltage(){
	uint16_t delta=0;
	delta+=jk_bms_battery_info.cells_voltage[jk_bms_battery_info.cell_number_vmax - 1].cell_voltage;
	delta-=jk_bms_battery_info.cells_voltage[jk_bms_battery_info.cell_number_vmin - 1].cell_voltage;
	return delta;
}

/**Devuelve el número de celda de 1 a 24*/
uint8_t cell_number_vmin(){
	uint8_t numeroCeldas=jk_bms_battery_info.cells_number;
	uint8_t ncellmin=0;
	uint16_t vcellmin=0;
	for(int i=0; i<numeroCeldas; i++){
		if(i==0 || vcellmin > jk_bms_battery_info.cells_voltage[i].cell_voltage){
			ncellmin=jk_bms_battery_info.cells_voltage[i].cell_number;
			vcellmin=jk_bms_battery_info.cells_voltage[i].cell_voltage;
		}
	}
	return ncellmin;
}
/**Devuelve el número de celda de 1 a 24*/
uint8_t cell_number_vmax(){
	uint8_t numeroCeldas=jk_bms_battery_info.cells_number;
	uint8_t ncellmax=0;
	uint16_t vcellmax=0;
	for(int i=0; i<numeroCeldas; i++){
		if(i==0 || vcellmax < jk_bms_battery_info.cells_voltage[i].cell_voltage){
			ncellmax=jk_bms_battery_info.cells_voltage[i].cell_number;
			vcellmax=jk_bms_battery_info.cells_voltage[i].cell_voltage;
		}
	}
	return ncellmax;
}

uint8_t calcularSOH(){
	uint16_t ciclos=jk_bms_battery_info.battery_status.battery_cycles;
    uint16_t porcentaje=0;
    porcentaje= (ciclos * 100) / jk_bms_battery_info.chemical;
    porcentaje = 100 - porcentaje;
	return (uint8_t)porcentaje;
}



/**
 * @brief Parseo de datos leido de la bms JK
 * https://github.com/syssi/esphome-jk-bms/blob/main/docs/protocol-design.md
 * @param data 
 */
void Parse_JK_Battery_485_Status_Frame(uint8_t *data) {
	// Status response
	//     START         LENGTH           ID(24bits)     Command source   T.type
	// <- 0x4E 0x57 | 0x01 0x1B |  0x00 0x00 0x00 0x00 |  0x06  | 0x00  | 0x01: Header
	//
	uint32_t id=0;
	id = (uint32_t) data[5] <<12 | (uint32_t)data[6] << 8 | data[7];
	jk_bms_battery_info.ID=id;
	// *Data*
	// saltarse la cabecera
	data=data+11;	


  
  	//   Address Content: Description      Decoded content                         Coeff./Unit
  	//R  0x79: Individual Cell voltage   (0x79 is followed by one byte length data)
  	//R  0x30: Cell count               48 / 3 bytes = 16 cells
	//	 The first byte is the cell number, the next two bytes is the voltage value mV
  	//R  0x01 0x0E 0xED: Cell 1         3821 * 0.001 = 3.821V                        0.001 V
  	//R  0x02 0x0E 0xFA: Cell 2         3834 * 0.001 = 3.834V                        0.001 V
  	//R  0x03 0x0E 0xF7: Cell 3         3831 * 0.001 = 3.831V                        0.001 V
  	//R  0x04 0x0E 0xEC: Cell 4         ...                                          0.001 V
  	//R  0x05 0x0E 0xF8: Cell 5         ...                                          0.001 V
  	//R  0x06 0x0E 0xFA: Cell 6         ...                                          0.001 V
  	//R  0x07 0x0E 0xF1: Cell 7         ...                                          0.001 V
  	//R  0x08 0x0E 0xF8: Cell 8         ...                                          0.001 V
  	//R  0x09 0x0E 0xE3: Cell 9         ...                                          0.001 V
  	//R  0x0A 0x0E 0xFA: Cell 10        ...                                          0.001 V
  	//R  0x0B 0x0E 0xF1: Cell 11        ...                                          0.001 V
  	//R  0x0C 0x0E 0xFB: Cell 12        ...                                          0.001 V
  	//R  0x0D 0x0E 0xFB: Cell 13        ...                                          0.001 V
  	//R  0x0E 0x0E 0xF2: Cell 14        3826 * 0.001 = 3.826V                        0.001 V
  	//R  0x0F 0x0E 0xF2: Cell 15        3826 * 0.001 = 3.826V                        0.001 V
  	//R  0xA0 0x0E 0xF2: Cell 16        3826 * 0.001 = 3.826V                        0.001 V


	uint8_t cells = data[1] / 3;
	jk_bms_battery_info.cells_number = cells;    
	//lectura de voltaje de celdas
	for (uint8_t i = 0; i < cells; i++) {
		jk_bms_battery_info.cells_voltage[i].cell_number = data[3*i + 2];
		jk_bms_battery_info.cells_voltage[i].cell_voltage = (uint16_t) data[3*i + 3] << 8 | data[3*i + 4];
		//Serial.printf("\nCell nº:%d\t%d", jk_bms_battery_info.cells_voltage[i].cell_number, jk_bms_battery_info.cells_voltage[i].cell_voltage);
	}
	cells++;
	uint16_t pos = (cells - 1) * 3 + 2; // 50
	//situar indice 'pos' despues de todos los bytes de voltajes de celdas apuntando al byte de dirección con valor 0x80	
	//R 0x80 0x00 0x1D: Read power tube temperature                 29°C                      1.0 °C
	// --->  99 = 99°C, 100 = 100°C, 101 = -1°C, 140 = -40°C

			//51 52
	jk_bms_battery_info.battery_status.power_tube_temperature = getTemperature((uint16_t) data[pos + 1] << 8 | data[pos + 2]);
	pos += 3; // 53
	//R 0x81 0x00 0x1D: Read  temperature  
			//54 55
	jk_bms_battery_info.battery_status.sensor_temperature_1 = getTemperature((uint16_t) data[pos + 1] << 8 | data[pos + 2]);
	pos += 3; // 56
	//R 0x82 0x00 0x1D: Read  temperature  
			//57 58
	jk_bms_battery_info.battery_status.sensor_temperature_2 = getTemperature((uint16_t) data[pos + 1] << 8 | data[pos + 2]);

	pos += 3; // 59


	//R 0x83 0x14 0xEF: Total battery voltage                       5359 * 0.01 = 53.59V      0.01 V
			// 60 61
	jk_bms_battery_info.battery_status.battery_voltage = (uint16_t) data[pos + 1] << 8 | data[pos + 2];
	pos += 3; // 62
	//R  0x84 0x80 0xD0: Current data                                32976                     0.01 A
			//63 64
	jk_bms_battery_info.battery_status.battery_current = getCurrent((uint16_t) data[pos + 1] << 8 | data[pos + 2]);
	pos += 3; // 65
	//R  0x85 0x0F: Battery remaining capacity                       15 %
			// 66
	jk_bms_battery_info.battery_status.battery_soc = (uint8_t) data[pos + 1];
	pos += 2; // 67
	//R   0x86 0x02: Number of battery temperature sensors             2                        1.0  count
			// 68
	jk_bms_battery_info.battery_status.temperature_sensor_count = (uint8_t) data[pos + 1];
	pos += 2; // 69
	//R   0x87 0x00 0x04: Number of battery cycles                     4                        1.0  count
			// 70 71
	jk_bms_battery_info.battery_status.battery_cycles = (uint16_t) data[pos + 1] << 8 | data[pos + 2];
	pos += 3; // 72
	//R   0x89 0x00 0x00 0x00 0x00: Total battery cycle capacity
			// 73 74 75 76
	
	jk_bms_battery_info.battery_status.battery_cycle_capacity = (uint32_t) data[pos + 1] << 24 | (uint32_t) data[pos + 2] << 16 | (uint32_t) data[pos + 3] << 8 | data[pos + 4];
	pos += 5; // 77
	// ignore strings number
	//R  0x8A 0x00 0x00:  Total number of battery strings
	pos += 3;

	//R  0x8B 0x00 0x00: Battery warning message                     0000 0000 0000 0000
	//	Bit	Description									Values					Severity
	//	0	Low capacity alarm							1 (alarm), 0 (normal)	warning
	//	1	MOS tube over-temperature alarm				1 (alarm), 0 (normal)	alarm
	//	2	Charging over-voltage alarm					1 (alarm), 0 (normal)	alarm
	//	3	Discharge undervoltage alarm				1 (alarm), 0 (normal)	alarm
	//	4	Battery over temperature alarm				1 (alarm), 0 (normal)	alarm
	//	5	charging overcurrent alarm					1 (alarm), 0 (normal)	alarm
	//	6	Discharge overcurrent alarm					1 (alarm), 0 (normal)	alarm
	//	7	Cell pressure difference alarm				1 (alarm), 0 (normal)	alarm
	//	8	Over-temperature alarm in the battery box	1 (alarm), 0 (normal)	alarm
	//	9	Battery low temperature alarm				1 (alarm), 0 (normal)	alarm
	//	10	Monomer overvoltage alarm					1 (alarm), 0 (normal)	alarm
	//	11	Monomer undervoltage alarm					1 (alarm), 0 (normal)	alarm
	//	12	309_A protection							1 (alarm), 0 (normal)	alarm
	//	13	309_A protection							1 (alarm), 0 (normal)	alarm
	//	14	Reserved		
	//	15	Reserved	
	
	uint16_t alarms = (uint16_t) data[pos + 1] << 8 | data[pos + 2];
	jk_bms_battery_info.battery_alarms.alarm_data = alarms;

	jk_bms_battery_info.battery_alarms.low_capacity = alarms & 0x01; //TODO: verificar en jk si se produce este aviso
	jk_bms_battery_info.battery_alarms.power_tube_overtemperature = (alarms >> 1) & 0x01;
	jk_bms_battery_info.battery_alarms.charging_overvoltage = alarms >> 2 & 0x01;
	jk_bms_battery_info.battery_alarms.discharging_undervoltage = alarms >> 3 & 0x01;
	jk_bms_battery_info.battery_alarms.battery_over_temperature = alarms >> 4 & 0x01;
	jk_bms_battery_info.battery_alarms.charging_overcurrent = alarms >> 5 & 0x01;
	jk_bms_battery_info.battery_alarms.discharging_overcurrent = alarms >> 6 & 0x01;
	jk_bms_battery_info.battery_alarms.cell_pressure_difference = alarms >> 7 & 0x01;
	jk_bms_battery_info.battery_alarms.overtemperature_alarm_battery_box = alarms >> 8 & 0x01;
	jk_bms_battery_info.battery_alarms.battery_low_temperature = alarms >> 9 & 0x01;
	jk_bms_battery_info.battery_alarms.cell_overvoltage = alarms >> 10 & 0x01;
	jk_bms_battery_info.battery_alarms.cell_undervoltage = alarms >> 11 & 0x01;
	jk_bms_battery_info.battery_alarms.a_protection_309_1 = alarms >> 12 & 0x01;
	jk_bms_battery_info.battery_alarms.a_protection_309_2 = alarms >> 13 & 0x01;

	pos += 3;
	//R   0x8C 0x00 0x07: Battery status information  - ignore (charge discharge MOS tube )
						//Bit0  charging MOS state 1=0n 0=off 
						//Bit1  discharging MOS state 1=0n 0=off 
						//Bit2  balance switch  state 1=0n 0=off 
						//Bit3  battery dropped state 1=normal 0=offline 
						//Bit4-Bit15  resereved 
	uint16_t mosfet = (uint16_t) data[pos + 1] << 8 | data[pos + 2];	
	jk_bms_battery_info.battery_mosfet.charge = mosfet & 0x00001;
	jk_bms_battery_info.battery_mosfet.discharge = (mosfet >> 1) & 0x0001;
	jk_bms_battery_info.battery_mosfet.balance = (mosfet >> 2) & 0x0001;
	jk_bms_battery_info.battery_mosfet.emergencia = (mosfet >> 3) & 0x0001;

	pos += 3;
	//RW  0x8E 0x16 0x26: Total voltage overvoltage protection        5670 * 0.01 = 56.70V     0.01 V
	jk_bms_battery_info.battery_limits.battery_charge_voltage = (uint16_t) data[pos + 1] << 8 | data[pos + 2];
	pos += 3;
	//RW  0x8F 0x10 0xAE: Total voltage undervoltage protection       4270 * 0.01 = 42.70V     0.01 V
	jk_bms_battery_info.battery_limits.battery_discharge_voltage = (uint16_t) data[pos + 1] << 8 | data[pos + 2];
	
	//RW  0x90 0x10 0x68:  cell overvoltage proteccion voltage        4200 * 0.001  = 4.2V     0.001V
	//RW  0x91 0x10 0x36:  cell overvoltage recovery voltage			 4150 * 0.001  = 4.15V    0.001V	
	//RW  0x92 0x00 0x04:  cell overvoltage proteccion is delayed  for 4 seconds  (1-60seconds)
	//RW  0x93 0x0A 0xF0:  Cell undervoltage protection voltage		 2800 * 0.001  = 2.8V     0.001V
	//RW  0x94 0x0B 0x54:  cell undervoltage recovery voltage         2900 * 0.001  = 2.9V     0.001V
	//RW  0x95 0x00 0x04:  cell undervoltage portection is delayed  for 4 seconds  (1-60seconds)
	//RW  0x96 0x01 0x2C:  diferecntial voltage protection value of cell 300mV                 0.001V
	
	pos += 24;
	//RW  0x97 0x00 0x07: Discharge overcurrent protection value       7A                         1 A
	jk_bms_battery_info.battery_limits.battery_discharge_current_limit = (uint16_t) data[pos + 1] << 8 | data[pos + 2];

	//RW  0x98 0x00 0x04:  delayed for overcurrent discharge  4seconds  (1-60seconds)

	pos += 6;
	//RW 0x99 0x00 0x05: Charging overcurrent protection value        5A                         1 A
	jk_bms_battery_info.battery_limits.battery_charge_current_limit = (uint16_t) data[pos + 1] << 8 | data[pos + 2];

	//RW 0x9A 0x00 0x04: Charge overcurrent delay 4seconds
	//RW 0x9B 0x10 0x36: Balanced starting voltage	                 4150 * 0.001  = 4.15V    0.001V
	//RW 0x9C 0x00 0x64: Balanced opening pressure difference 	  100mv
	pos += 12;
	//RW 0x9D 0x00:      Active balance switch  0=OFF  1=ON
	jk_bms_battery_info.active_balance = data[pos + 1];
	//RW 0x9E 0x00 0x64: power tube temperature protecction value 100ºC
	//RW 0x9F 0x00 0x50: power tube temperature recovery value 80
	//RW 0xA0 0x00 0x50: Temperature protection value in the battery box   100ªC
	//RW 0xA1 0x00 0x46: Temperature recovery value in the battery box  70ºC
	//RW 0xA2 0x00 0x14: battery temperature difference protection value 20ºC
	//RW 0xA3 0x00 0x64: Battery charging high temperature protection value 100
	//RW 0xA4 0x00 0x64: Battery discharging high temperature protection value 100ºc
	//RW 0xA5 0xFF 0XEC: charging low temperature protection value -20ºC
	//RW 0xA6 0xFF 0xF6: recovery value of charging low temperature -10ºC
	//RW 0xA7 0xFF 0xEC: discharge low temperature protection -20ªC
	//RW 0xA8 0XFF 0xF6: recovery value  of discharge low temperature -10ºC
	//RW 0xA9 0x14:      battery string number setting 20
	pos += 37;
	//RW 0xAA 0x00 0x00 0x00 0x28:  battery capacity setting   40Ah     1Ah
	jk_bms_battery_info.battery_cell_capacity=(uint32_t) data[pos + 1] << 24 | (uint32_t) data[pos + 2] << 16 | (uint32_t) data[pos + 3] << 8 | data[pos + 4];
	
	//RW 0xAB 0x00: 	 charging MOS switch write control bit 0 close, 1 on (trigger)
	//RW 0xAC 0x00:      discharging MOS switch write control bit 0 close, 1 on (trigger)
	//RW 0xAD 0x03 0xE8: current calibration 1000mA
	//RW 0xAE 0x01:      protective board address (default 1) is reserved for use when cascading
	pos += 12;
	//RW 0xAF 0x01:      battery type defualt=1   (0=LiFePo 1=Li-ion 2=LTO) 
	switch (data[pos +1]){
		case 0 : jk_bms_battery_info.chemical=LiFePo4; break;
		case 1 : jk_bms_battery_info.chemical=LiIon; break;
		case 3 : jk_bms_battery_info.chemical=LTO; break;
		default : jk_bms_battery_info.chemical=AcidLead; break;

	};
	
	//RW 0xB0 0x00 0x0A: hibernation wait time initalization default 10 seconds
	pos +=6;
	//RW 0xB1 0x14:      low capacity alarm value (20%)
	jk_bms_battery_info.low_capacity_alarm_value=data[pos +1];
	//RW 0xB2 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00: modify parameter password default 0
	//RW 0xB3 0x01:      private charger switch  default 1  off=0  on=1 
	//RW 0xB4 0x36 0x30 0x33 0x30 0x30 0x30 0x30 0x31: Device ID CODE inicialization 60300001 (char)
												//Example 60300001 (60-nominal voltage level: defined by the voltage level, 
												//for example, 60 is 60V. Series 48 is 48V series; 3-material system: 
												//according to the system definition of battery materials such as iron. 
												//Lithium code is 1 manganic acid code 2 ternary code 3; 
												//00001-production serial number: according to manufacturing. 
												//The Nth group of the model produced by the manufacturer that month is numbered N 
												//(for example: a certain type The first group of the number, then N is 00001)) characters
	//RW 0xB5 0x32 0x30 0x30 0x34: Factory date 2004  (char)
	//RW 0xB6 0x00 0x00 0x00 0x01: system working time 1minute
	//R  0xB7 0x4E 0x57 0x5F 0x31 0x5F 0x30 0x5F 0x30 0x5F 0x32 0x30 0x30 0x34 0x32 0x38:  version number: NW_1_0_0_200428 (char)
	//RW 0xB8 0x00:      1 (Start calibration) 0 (Close calibration)
	//RW 0xB9 0x00 0x00 0x00 0x00: Actual  battery capacity Ah
	//RW 0xBA + (24bytes): Manufacture ID naming (char)
	
	// address only write, no read in to response
	//W  0xBB 0x00:  Restart system (only write)
	//W  0xBC 0x00:  restore factory reference parameters (only write)
	//W  0xBD 0x00: remote upgrade logo (only write)
	//W  0xBE 0x00 0x00: Battery low voltage turns off GPS, unit: mV ( turn off the power to GPS when low voltage is detected )
	//W  0xBF 0x00 0x00: Battery low voltage recovery GPS
	//  estas 5 direcciones anteriores, no estan en la trama de respuesta  

	//R  0xC0 0x00: 	Protocol version number
	//   0x00 0x00 0x00 0x00 0x00:  Record number  (El 1 byte superior es código aleatorio,sin sentido (reservado para el cifrado),
														//y los 3 bytes inferiores son código aleatorio número de registro)
	// 0x68:           End identification
	// 0x00 0x00 0x49 0x23:  Checksum  Accumulated Checksum (High 2 bytes for CRC not enabled fill 0, Low 2
	jk_bms_battery_info.cell_Vavrg=cell_average_voltage();
	jk_bms_battery_info.cell_number_vmax=cell_number_vmax();
	jk_bms_battery_info.cell_number_vmin=cell_number_vmin();
	jk_bms_battery_info.delta_cell_voltage=delta_cell_voltage();
	jk_bms_battery_info.battery_status.battery_soh=calcularSOH();

	
}


/**
 * @brief Experimental sin terminar, peticiones a JKBMS
 * 
 * @param buffer almenos 23bytes
 * @param direccion 
 * @param valor 
 * @return uint8_t 
 */
uint8_t crearTramaEscritura(uint8_t * buffer, uint8_t direccion, uint16_t valor, bool valor8bit){
	//Start Frame  0x4E, 0x57,    2byte
	buffer[0]=0x4E;  
	buffer[1]=0x57; 
	//longitud trama     2byte  Bigendian
	buffer[2]=0x00;  //21bytes (valor16bit) o 20bytes(valor8bit) desde el startframe
	buffer[3]= valor8bit? 20u : 21u; 	
	// Id bms            4byte
	buffer[4]=0x00;
	buffer[5]=0x00;
	buffer[6]=0x00;
	buffer[7]=0x00;
	//  comando   escritura 0x02
	buffer[8]=0x02;
	// fuente de la trama PC 0x03, GPS 0x02
	buffer[9]=0x03;
	//tipo de transmisión  0=request  1=answer  
	buffer[10]=0x00;    
	// payload
	buffer[11] = direccion;
	uint8_t index=12; //indice siguiente byte
	//16 o 8bit?
	if(valor8bit==false){
		buffer[index] = (valor >> 8) & 0xFF ;
		index++;
	}
	buffer[index] = valor & 0xFF;
	index++;
	//número de registro solicitud  4bytes
	buffer[index]=0x00; 
	index++;
	buffer[index]=0x00; 
	index++;
	buffer[index]=0x20;
	index++;
	buffer[index]=0x01;
	index++;
	//end frame
	buffer[index]=0x68;  //byte end frame to start frame=>checksum
	index++;
	//2bytes vacios
	buffer[index]=0x00;
	index++;
	buffer[index]=0x00;
	index++;
	//checksum
	uint16_t checksum=0;

	for(int i =0; i < index ; i++){  //¿index-2?
		checksum += buffer[i];
	}
	buffer[index]=(checksum >> 8) & 0xFF;
	index++;
	buffer[index]=checksum & 0xFF;
	index++;
	return index;
}


void sendRequestJKBMS(uint8_t * buffer, uint16_t size){

	for(int i=0; i<size; i++){
		Serial2.write(buffer[i]);
	}
	Serial2.flush();
	
}




