#include <Arduino.h>
#include "tareas.h"
#include "pylon_can.h"
#include "pylonHV_can.h"
#include "batrium_can.h"
#include "jk_bms_485.h"
#include "utilidades.h"
#include "PortalWeb.h"


#define ONBOARD_LED_BLUE        GPIO_NUM_2 //2
#define FREQ_MUESTREO           3 //cada 3segundos
const TickType_t delayMuestreo = (1000 * FREQ_MUESTREO) / portTICK_PERIOD_MS; //peticiones cada 1000msg * muestreo


void bajaCapacidad();

//----------FUNCIONES DE TAREAS----------------------// 

void peticionrs485_task(void * parameters){
  while(true){
    timerWrite((hw_timer_t *)parameters, 0); // reset timer (feed watchdog)
    if(configuracion.comunicarJKrs485){
      Request_JK_Battery_485_Status_Frame(); 
    } 
    vTaskDelay(delayMuestreo);  //cada 3 segundos
  }  
}


void capturarSerialToColaLectura_task(void *parameters){  
  while(true){
    int sizeBufferStream=0;
    sizeBufferStream=Serial.available();
    if(sizeBufferStream>0){
        int leidos=Serial.readBytesUntil( '\n', bufferStream, 512);  //cambiar a 512
        //Serial.printf("\n\t Leidos:%d",leidos);
        if( xQueueSendToBack(colaPuertoSerie_handle, ( void * ) &bufferStream, ( TickType_t ) 200  )== false){
          Serial.println("ERROR push cola");
        }
        //borrado buffer
        memset(bufferStream, 0, 512);   //280
    }
  }
}


void capturarSerial2ToColaLectura_task(void *parameters){  
  while(true){
    uint16_t bytes_available=0;
    uint16_t buffersize=0;
    bytes_available=Serial2.available();
    //Si hay bytes para leer y obtengo el semaforo y espero para obtenerlo  // 0.5sg
    if(bytes_available && xSemaphoreTake( xMutex, portMAX_DELAY)){ //500 )){
     
        buffersize=Serial2.readBytes( bufferStream, 512); //la trama nunca es superior a 512bytes
        //comprobar si es la trama esperada de bytes
        if(JK_bms_check_header(bufferStream)==true && JK_Battery_485_Check_Frame_CRC(bufferStream, buffersize)){
            //Serial.println("TRAMA OK");
            if( xQueueSendToBack(colaPuertoSerie_handle, ( void * ) &bufferStream, ( TickType_t ) 200  )== false){
              Serial.println("#[TAREAS capturarSerial2ToColaLectura]-ERROR push cola");
            }
        }        
        //borrado buffer
        memset(bufferStream, 0, 512);  
        xSemaphoreGive( xMutex );
      }       
    } 
}



/** Tarea principal de actualización de datos del BMS  */
void parseoColaLecturaToJk_bms_battery_info_task(void * parameters){
    uint16_t contador=0;
    const TickType_t esperar2sg = 2000 / portTICK_PERIOD_MS;
    while(true){
        timerWrite((hw_timer_t *)parameters, 0); // reset timer (feed watchdog)
        uint32_t timestampActual=millis();
        if(xQueueReceive(colaPuertoSerie_handle, bufferReceiver , esperar2sg) ){
              Parse_JK_Battery_485_Status_Frame(bufferReceiver);
              //actualizarContadorEnergia();
              //ajustarSOC();  no usar 
              bajaCapacidad();
              ajustarAmperiosCargaDescarga();
              controlCargaDescarga();
              contador=0;
              configuracion.errorComunicacionJK=false;
        }else{
          // si pasan más de 6sg sin recibir datos establecer error de comunicación
          if(contador>=4)configuracion.errorComunicacionJK=true;
          if(contador==0xFFFF)contador=3;
          contador++;
        }
    }
}





void parpadeoLed_task(void * parameters){
  while(true){
        //timerWrite((hw_timer_t *)parameters, 0); // reset timer (feed watchdog)
        if(configuracion.wifiConfigured && WiFi.isConnected()){
            gpio_set_level( ONBOARD_LED_BLUE, 1);
            //digitalWrite(23, 1);
        }else{
             digitalWrite(ONBOARD_LED_BLUE, !gpio_get_level( ONBOARD_LED_BLUE));
        }
        vTaskDelay(500);
  }
}


void resetConfiguracion_task(void * parameters){
    
    int contador=0;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS; 
    while(true){
        //timerWrite((hw_timer_t *)parameters, 0); // reset timer (feed watchdog)
        vTaskDelay(xDelay); 
        if(gpio_get_level(GPIO_NUM_0)== 0){
            contador++;
            Serial.println("PULSADO RESET");
            if(contador>5){
                contador=0;
                configuracionInicial();
                Serial.println("HARD_RESET.........."); //TODO ¿implementar un hard reset o un reset WIFI?
                delay(100);
                ESP.restart();
            }
              
        }
        
    }
}



void send_canbus_message(uint32_t identifier, uint8_t *buffer, uint8_t length, bool id29bits=false){
  static const char *TAG = "canbus";
  twai_message_t message;
  message.data_length_code = length;
  if(id29bits==true){
    message.identifier = identifier;
    message.flags = TWAI_MSG_FLAG_EXTD;
  }else{
    message.identifier = identifier;
    message.flags = TWAI_MSG_FLAG_NONE;
  }

  memcpy(&message.data, buffer, length);

  //Queue message for transmission
  if (twai_transmit(&message, pdMS_TO_TICKS(250)) != ESP_OK){
    ESP_LOGE(TAG, "Fail to queue message");
    //canbus_messages_failed_sent++;
  } else {
    //ESP_LOGI(TAG, "Sent CAN message %03x", identifier);
    //ESP_LOG_BUFFER_HEX_LEVEL(TAG, &message, sizeof(can_message_t), esp_log_level_t::ESP_LOG_DEBUG);
    //canbus_messages_sent++;
  }

  
}



void envioCAN_task(void * parameters){
  const TickType_t xDelay250msg = 250 * portTICK_PERIOD_MS; //pausa de 250msg
  const TickType_t xDelay100msg = 100 * portTICK_PERIOD_MS; //pausa de 100msg
  const TickType_t xDelay1sg = 1000 * portTICK_PERIOD_MS; //repeticiones cada 1000msg
  twai_message_t mensajeCAN;
  
  
  while(true){
    timerWrite((hw_timer_t *)parameters, 0); // reset timer (feed watchdog)
    //no esta habilitada comunicaciónCAN
    if(configuracion.comunicarCAN==false || configuracion.dispositivoCAN==false){ 
      vTaskDelay(xDelay250msg);  //dar tiempo a otros hilos/tareas
      continue;
    } 
        
    // CAN-OK 
    if(configuracion.pylontechHV && xQueueReceive(colaLecturaCAN_handle, &mensajeCAN, xDelay100msg)){
        const uint32_t request=0x420; //11bits identifier
        const uint32_t request29bits=0x4200; // 29bits identifier
        const uint32_t control_request=0x620; //11bits identifier
        const uint32_t control_request29bits=0x8200; //29bits identifier

        if(mensajeCAN.identifier==request){
          if(mensajeCAN.data[0]==0x00){  
              enviarCANpylonHV(false); 
              if(configuracion.comunicarSerialDebug1){
                Serial.printf("SEND-CAN PylonHV(%.3f)\n",millis()/1000.0);
                mostrarMensajeCAN_pylonHV();
              }
          }else if(mensajeCAN.data[0]==0X02){
              enviarCANpylonHVinfoSystem(false);
              if(configuracion.comunicarSerialDebug1){
                Serial.printf("SEND-CAN PylonHV Info System(%.3f)\n",millis()/1000.0);
              } 
          }
          continue;
        }

        if(mensajeCAN.identifier==request29bits){
          if(mensajeCAN.data[0]==0x00){  
            enviarCANpylonHV(true);
            if(configuracion.comunicarSerialDebug1){
              Serial.printf("SEND-CAN PylonHV_id29bits(%.3f)\n",millis()/1000.0);
              mostrarMensajeCAN_pylonHV();
            }
          }else if(mensajeCAN.data[0]==0X02){
              enviarCANpylonHVinfoSystem(true);
              if(configuracion.comunicarSerialDebug1){
                Serial.printf("SEND-CAN PylonHV_id29bits Info System(%.3f)\n",millis()/1000.0);
              } 
          }
          continue;
        }

        if(mensajeCAN.identifier==control_request || mensajeCAN.identifier==control_request29bits){
            if(mensajeCAN.data[0]==0xAA){
              //TODO: despertar bateria 
            continue;
            }

            if(mensajeCAN.data[0]==0x55){
              //TODO: dormir batería
            continue;
            }
        }                   
    } //if pylonHV



    if(configuracion.pylontechHV==false){    
     
        enviarCANpylonLV();
        //enviarCANbatrium();
     
        if(configuracion.comunicarSerialDebug1){
          Serial.printf("SEND-CAN PylonLV(%.3f)\n",millis()/1000.0);
          mostrarMensajeCAN_pylonLV();
          //mostrarMensajeCAN_Batrium();
      
        vTaskDelay(xDelay1sg); 
        //vTaskDelay(xDelay100msg);
      }    
    } 
           
    //TODO: otros protocolos  

  }//end while
}

void recibirCAN_task(void * parameters){
  const TickType_t xDelay250msg = 250 * portTICK_PERIOD_MS; //pausa de 250msg
  const TickType_t xDelay100msg = 100 * portTICK_PERIOD_MS; //pausa de 100msg 
  twai_message_t message;
  bool id_extend=false;  //11 or 29bits id

  while(true){
    //timerWrite((hw_timer_t *)parameters, 0); // reset timer (feed watchdog)
    if(configuracion.comunicarCAN && twai_receive(&message, portMAX_DELAY) == ESP_OK){
     
      if( xQueueSendToBack(colaLecturaCAN_handle, (void*) &message, xDelay100msg )== false){
              Serial.println("#[TAREAS recibirCAN]-ERROR push cola");      
      }

      if(configuracion.comunicarSerialDebug1){
        Serial.printf("CAN recibido (%.3fsg)  ID:%08x ID_EXTENDED:%d\tDATA::",
                            millis()/1000.0, message.identifier, message.extd);
        for(int i=0; i <message.data_length_code; i++){
          Serial.printf("%02x ", message.data[i]);
        }
        Serial.println("\n");
      }

    }else{
      // si no hay CAN dejar tiempo otras tareas
      vTaskDelay(xDelay250msg);
    }
    
  }//fin while
}

void mostrarMensajeCAN_pylonLV(){
  uint8_t buffer[8];
  uint8_t buffer2[2];
  String text="";
  text="\n";   

  parseJK_message_0x359(buffer, &jk_bms_battery_info);
  text+="0x359::";
  text=bufferToString(text, buffer);
  text+="\tProtection Alarm Nºmodules\n";

  parseJK_message_0x351(buffer,  &jk_bms_battery_info);
  text+="0x351::";
  text=bufferToString(text, buffer);
  text+="\tVmaxCharge  ImaxCarge  ImaxDischarge\n";

  parseJK_message_0x355(buffer,  &jk_bms_battery_info);
  text+="0x355::";
  text=bufferToString(text, buffer);
  text+="\tSOC SOH\n";

  parseJK_message_0x356(buffer,  &jk_bms_battery_info);
  text+="0x356::";
  text=bufferToString(text, buffer); 
  text+="\tVol Amp Temp\n";
          
  parseJK_message_0x35C(buffer2,  &jk_bms_battery_info);
  char cadena[64]{};
  int size=sprintf(cadena, "0x35C::%02x %02x ", buffer2[0], buffer2[1]);
  text+=String(cadena, size);  
  text+="\tFlags Charge Discharge \n";
          
  parseJK_message_0x35E(buffer,  &jk_bms_battery_info);
  text+="0x35E::";
  text=bufferToString(text, buffer);      
  text+="\t PYLON\n";
  Serial.println(text);
}

void enviarCANpylonLV(){
  uint8_t buffer[8];
  uint8_t buffer2[2];
 
  send_canbus_message(0x359, parseJK_message_0x359(buffer,  &jk_bms_battery_info), 8);
  send_canbus_message(0x351, parseJK_message_0x351(buffer,  &jk_bms_battery_info), 8 );
  send_canbus_message(0x355, parseJK_message_0x355(buffer,  &jk_bms_battery_info), 8 );
  send_canbus_message(0x356, parseJK_message_0x356(buffer,  &jk_bms_battery_info), 8 );
  send_canbus_message(0x35C, parseJK_message_0x35C(buffer2, &jk_bms_battery_info), 2 );
  send_canbus_message(0x35E, parseJK_message_0x35E(buffer,  &jk_bms_battery_info), 8 );
  
}

void mostrarMensajeCAN_pylonHV(){
 uint8_t buffer[8];
  String text="";
  text="\n\n";
 
  parseJK_message_0x4210(buffer, &jk_bms_battery_info);
  text+="0x421::";
  text=bufferToString(text, buffer);
  text+="\tVol, Amp, Temp, SOC, SOH\n";

  parseJK_message_0x4220(buffer, &jk_bms_battery_info);
  text+="0x422::";
  text=bufferToString(text, buffer);
  text+="\tVmaxcharge Vmindischarge Imaxcharge Imax discharge\n";

  parseJK_message_0x4230(buffer, &jk_bms_battery_info);
  text+="0x423::";
  text=bufferToString(text, buffer);
  text+="\tVmaxcell Vmincell nCellVmax nCellVmin\n";

  parseJK_message_0x4240(buffer, &jk_bms_battery_info);
  text+="0x424::";
  text=bufferToString(text, buffer);
  text+="\tCellTempMax CellTempMin nCellTempMax nCellTempMin\n";

  parseJK_message_0x4250(buffer, &jk_bms_battery_info);
  text+="0x425::";
  text=bufferToString(text, buffer);
  text+="\tStatus, Cycles, Error, Alarm, Protection\n";

  parseJK_message_0x4260(buffer, &jk_bms_battery_info);
  text+="0x426::";
  text=bufferToString(text, buffer);
  text+="\tModuleVMax ModuleVMin nModuleVMax nModuleVMin\n";

  parseJK_message_0x4270(buffer, &jk_bms_battery_info);
  text+="0x427::";
  text=bufferToString(text, buffer);
  text+="\tModuleTempMax ModuleTempMin nModuleTempMax nModuleTempMin\n";

  parseJK_message_0x4280(buffer, &jk_bms_battery_info);
  text+="0x428::";
  text=bufferToString(text, buffer);
  text+="\tFlags Charged(0xAA)  Discharged(0XAA) null:0x00\n";

  parseJK_message_0x4290(buffer, &jk_bms_battery_info);
  text+="0x429::";
  text=bufferToString(text, buffer);
  text+="\tList extension system error\n";

  parseJK_message_0x42A0(buffer, &jk_bms_battery_info);
  text+="0x42A::";
  text=bufferToString(text, buffer);
  text+="\tModuleTerminal TempMax TempMin nModuleMax nModuleMin\n";
  Serial.println(text);
  
}

void enviarCANpylonHV(bool ID_29bits=false){
  uint8_t buffer[8];   
  if(ID_29bits){
    send_canbus_message(0x4211, parseJK_message_0x4210(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4221, parseJK_message_0x4220(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4231, parseJK_message_0x4230(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4241, parseJK_message_0x4240(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4251, parseJK_message_0x4250(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4261, parseJK_message_0x4260(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4271, parseJK_message_0x4270(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4281, parseJK_message_0x4280(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x4291, parseJK_message_0x4290(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x42A1, parseJK_message_0x4290(buffer, &jk_bms_battery_info), 8, ID_29bits);
  }else{
    send_canbus_message(0x421, parseJK_message_0x4210(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x422, parseJK_message_0x4220(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x423, parseJK_message_0x4230(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x424, parseJK_message_0x4240(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x425, parseJK_message_0x4250(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x426, parseJK_message_0x4260(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x427, parseJK_message_0x4270(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x428, parseJK_message_0x4280(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x429, parseJK_message_0x4290(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x42A, parseJK_message_0x4290(buffer, &jk_bms_battery_info), 8, ID_29bits);
  }
}

void enviarCANpylonHVinfoSystem(bool ID_29bits=false){
  uint8_t buffer[8];   
  if(ID_29bits){
    send_canbus_message(0x7311, parseJK_message_0x4210(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x7321, parseJK_message_0x4220(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x7331, parseJK_message_0x4230(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x7341, parseJK_message_0x4240(buffer, &jk_bms_battery_info), 8, ID_29bits);
  }else{
    send_canbus_message(0x731, parseJK_message_0x4210(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x732, parseJK_message_0x4220(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x733, parseJK_message_0x4230(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(0x734, parseJK_message_0x4240(buffer, &jk_bms_battery_info), 8, ID_29bits); 
  }
}

void enviarDatosMqtt_task(void * parameters){
  
  while(true){
    if(configuracion.comunicarMQTT && WiFi.isConnected()){
      if(mqtt.connected()){
        String payload="";
        DynamicJsonDocument docjson(4096);
        parseJK_JSON(docjson, &jk_bms_battery_info, &configuracion);
        int size=serializeJson(docjson, payload);
        mqtt.publish(configuracion.topicmqtt, 1, true, payload.c_str());
      }else{
        IPAddress ipmqtt(configuracion.ipmqtt[0], configuracion.ipmqtt[1], configuracion.ipmqtt[2], configuracion.ipmqtt[3]);
        mqtt.setCredentials(configuracion.usermqtt, configuracion.passmqtt);
        mqtt.setServer(ipmqtt, configuracion.portmqtt);
        mqtt.connect();
      }
    }else{
      mqtt.clearQueue();
      mqtt.disconnect();
    }
    vTaskDelay(delayMuestreo);  
  }
}

void mostrarMensajeCAN_Batrium(){
    uint8_t buffer[8];
    String text="";
    text="\n\n";

    parseJK_message_0x00(buffer, &jk_bms_battery_info);
    text+="ID + 0x00::";
    text=bufferToString(text, buffer);
    text+="\tDevice versioning\n";

    parseJK_message_0x01(buffer, &jk_bms_battery_info);
    text+="ID + 0x01::";
    text=bufferToString(text, buffer);
    text+="\tVmin, Vmax, Vavrg, Nmin, Nmax\n";

    parseJK_message_0x02(buffer, &jk_bms_battery_info);
    text+="ID + 0x02::";
    text=bufferToString(text, buffer);
    text+="\tTmin, Tmax, Tavrg, nTmin, nTmax\n";

    parseJK_message_0x03(buffer, &jk_bms_battery_info);
    text+="ID + 0x03::";
    text=bufferToString(text, buffer);
    text+="\tnCells balance, N_init, N_end, reserved\n";

    parseJK_message_0x04(buffer, &jk_bms_battery_info);
    text+="ID + 0x04::";
    text=bufferToString(text, buffer);
    text+="\tVoltaje, amperios, potencia \n";

    parseJK_message_0x05(buffer, &jk_bms_battery_info);
    text+="ID + 0x05::";
    text=bufferToString(text, buffer);
    text+="\tSOC, SOH, Ah_remaing, AH_nominal\n";

    parseJK_message_0x06(buffer, &jk_bms_battery_info);
    text+="ID + 0x06::";
    text=bufferToString(text, buffer);
    text+="\tV_charge, A_Charge, V_discharge, A_Discharge\n";

    parseJK_message_0x07(buffer, &jk_bms_battery_info);
    text+="ID + 0x07::";
    text=bufferToString(text, buffer);
    text+="\tCritical, charge, discharge, heat, cool, balance FLAGS\n";

    Serial.println(text); 
}

void enviarCANbatrium(){
  static uint8_t contador=0;
  twai_message_t mensajeCAN;
  const bool ID_29bits=true;
  uint8_t buffer[8];
  /* Se establece un batrium ID:0x00000000 id_extend*/
  uint32_t ID=0x00000000;
  if(contador>=10){
    contador=0;
    //cada 1000msg
    send_canbus_message(ID + 0x00, parseJK_message_0x00(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(ID + 0x02, parseJK_message_0x02(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(ID + 0x03, parseJK_message_0x03(buffer, &jk_bms_battery_info), 8, ID_29bits);
    send_canbus_message(ID + 0x05, parseJK_message_0x05(buffer, &jk_bms_battery_info), 8, ID_29bits);
  }
  //cada 100msg
  send_canbus_message(ID + 0x01, parseJK_message_0x01(buffer, &jk_bms_battery_info), 8, ID_29bits);
  send_canbus_message(ID + 0x04, parseJK_message_0x04(buffer, &jk_bms_battery_info), 8, ID_29bits);
  send_canbus_message(ID + 0x06, parseJK_message_0x06(buffer, &jk_bms_battery_info), 8, ID_29bits); 
  send_canbus_message(ID + 0x07, parseJK_message_0x07(buffer, &jk_bms_battery_info), 8, ID_29bits);
  contador++;
  
}

/* Establece los permisos de carga y descarga en función del SOC preconfigurado*/
void controlCargaDescarga(){
  uint8_t soc=jk_bms_battery_info.battery_status.battery_soc;
  bool hayCambio=false;
  bool descargar=true;
  bool cargar=true;
  uint16_t voltaje=jk_bms_battery_info.battery_status.battery_voltage/10;
  
  if(configuracion.bateria.stopCargaPorVoltaje){
    if(voltaje < configuracion.bateria.voltajeStopCarga) cargar=true;
    if(voltaje >= configuracion.bateria.voltajeStopCarga) cargar=false;
  }
  if(configuracion.bateria.stopDescargaPorVoltaje){
    if(voltaje > configuracion.bateria.voltajeStopDescarga)descargar=true;
    if(voltaje <= configuracion.bateria.voltajeStopDescarga)descargar=false;
  }


  if(soc > configuracion.bateria.soc_max_restart_descarga && jk_bms_battery_info.battery_mosfet.discharge && descargar){
    hayCambio = configuracion.habilitarDescarga? false : true;
    configuracion.habilitarDescarga=true;
  }

  if(soc <= configuracion.bateria.soc_min_stop_descarga || !jk_bms_battery_info.battery_mosfet.discharge || !descargar){
    hayCambio = configuracion.habilitarDescarga? true : false;
    configuracion.habilitarDescarga=false;
  }

  if(soc < configuracion.bateria.soc_min_restart_carga && jk_bms_battery_info.battery_mosfet.charge && cargar){
    hayCambio=configuracion.habilitarCarga? false: true;
    configuracion.habilitarCarga=true;
  }

  if(soc >= configuracion.bateria.soc_max_stop_carga || !jk_bms_battery_info.battery_mosfet.charge || !cargar){
    hayCambio = configuracion.habilitarCarga? true:false;
    configuracion.habilitarCarga=false; 
  }

  if(hayCambio){
    configuracionSalvarEEPROM();
  }
  
}

/* función sin terminar, volorar su uso, jkbms calcula soc con su contador de ah */
void actualizarContadorEnergia(){
  uint32_t cellAH=jk_bms_battery_info.battery_cell_capacity;
  uint16_t current=jk_bms_battery_info.battery_status.battery_current * 10; // (+)carga (-)descarga unit:10mA
  uint16_t C100=cellAH *10;   // 230AH 1C=230A  230*1000=230000ma 1% = 230000/100= 2300
  uint16_t cells_Vavrg=jk_bms_battery_info.cell_Vavrg;
  uint8_t  soc=jk_bms_battery_info.battery_status.battery_soc;
  if(configuracion.bateria.cargado_total){
    
    return;
  }

  if(current>=0  &&  current<C100  &&  cells_Vavrg>=configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_95]){
    configuracion.bateria.cargado_total=true;
    // si sube la tensión sube el soc y los AH absorvidos por la bateria
    //se aplica correccion del soc
    for(int i=CELL_SOC::SOC_95; i < CELL_SOC::SOC_COUNT; i++){
      if(cells_Vavrg >= configuracion.bateria.calibracion.voltageCell[i]){
        meter.miliamperioSegundo=cellAH*1000*3600*valorPorciento[i]/100; //Ah*1000= mAh   mAh *60m * 60sg= mAsg *%SOC
        //TODO aplicar ajuste en función del SOH, si está el número de ciclos cerca del final se aplicara 80% de capacidad inicial
      }
    }

  }else{
    configuracion.bateria.cargado_total=false;
    meter.miliamperioSegundo += current * FREQ_MUESTREO;  // * segundos de muestreo a la bms jk 
  }
} 

/* función sin terminar-depurar,  NO USAR */
/* no funciona bien diferente tensión carga/reposo/descarga para el mismo soc*/
void ajustarSOC(){
    uint8_t socJK=jk_bms_battery_info.battery_status.battery_soc;
    uint16_t current=jk_bms_battery_info.battery_status.battery_current;
    uint16_t cell_Vavrg=jk_bms_battery_info.cell_Vavrg;
    // esta en el medio de la curva?
    if((socJK < 80 && socJK > 15)){
      //TODO: calcular por contador de energía

    }else{
      //calcular por voltaje
      for(int i=0; i < CELL_SOC::SOC_COUNT; i++){
            if(cell_Vavrg >= configuracion.bateria.calibracion.voltageCell[i]){
              socJK=valorPorciento[i];
            }
          }
    }

    // en caso de que el SOC esté entre 15 y 80 por mal cálculo de la jk 
    //por ejemplo en los primeros ciclos se ajusta con voltaje
    if(cell_Vavrg <= configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_15]  ||
       cell_Vavrg >= configuracion.bateria.calibracion.voltageCell[CELL_SOC::SOC_80] ){
          
          for(int i=0; i < CELL_SOC::SOC_COUNT; i++){
            if(cell_Vavrg >= configuracion.bateria.calibracion.voltageCell[i]){
              socJK=valorPorciento[i];
            }
          }
       }

    jk_bms_battery_info.battery_status.battery_soc=socJK;
}

/* devuelve indices de posición de soc con respecto a un escala definada en una array ascendente*/
uint8_t * getIndexLimit(uint16_t soc, uint8_t *indexLimit, uint16_t * arrayEscalaAscnd, uint16_t sizearrayEscalaAscnd){
  
  for(int i=0; i < sizearrayEscalaAscnd; i++){
    
    if(soc < arrayEscalaAscnd[0]){
      indexLimit[0]=0;
      indexLimit[1]=0;
      break;
    }else if( soc > arrayEscalaAscnd[sizearrayEscalaAscnd-1] ){
      indexLimit[0]=sizearrayEscalaAscnd-1;
      indexLimit[1]=sizearrayEscalaAscnd-1;
      break;
    }else if(soc == arrayEscalaAscnd[i]){
      indexLimit[0]=i;
      indexLimit[1]=i;
      break;
    }
    if(soc < arrayEscalaAscnd[i] && soc > arrayEscalaAscnd[i-1]){
      indexLimit[0]=i-1;
      indexLimit[1]=i;
    }
  }
  return indexLimit;
} 

/** funcion auxiliar calcula proporción/diferencia out en funcion de in,excepto si no hay diferencia*/
long proporcion(long x, long in_min, long in_max, long out_min, long out_max) {
    const long dividend = out_max - out_min;
    const long divisor = in_max - in_min;
    const long delta = x - in_min;
    if(divisor == 0){
        return out_min; 
    }
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}


/* Modifica el valor de carga y descarga proporcionado por JKBMS para adaptarlo a la curva de carga*/
void ajustarAmperiosCargaDescarga2(){
  uint16_t data=0;
  uint8_t indices[]={0,0};
  //uint8_t mayorSOC=0;
  //uint8_t menorSOC=0;
  uint16_t superiorSOC[]={79,80,85,90,95};
  uint16_t soc=jk_bms_battery_info.battery_status.battery_soc;
  uint16_t limiteCarga[]={jk_bms_battery_info.battery_limits.battery_charge_current_limit,
                        configuracion.bateria.intensidad_carga.soc_80,
                        configuracion.bateria.intensidad_carga.soc_85,
                        configuracion.bateria.intensidad_carga.soc_90,
                        configuracion.bateria.intensidad_carga.soc_95 };
  getIndexLimit(soc, indices, superiorSOC, 5 );
  data=proporcion(soc,  superiorSOC[indices[0]], superiorSOC[indices[1]], limiteCarga[indices[0]], limiteCarga[indices[1]]);
  jk_bms_battery_info.battery_limits.battery_charge_current_limit=data;
  
  uint16_t inferiorSOC[]={5,10,15,20,21};
  uint16_t limiteDescarga[]={configuracion.bateria.intensidad_descarga.soc_05,
                            configuracion.bateria.intensidad_descarga.soc_10,
                            configuracion.bateria.intensidad_descarga.soc_15,
                            configuracion.bateria.intensidad_descarga.soc_20,
                            jk_bms_battery_info.battery_limits.battery_discharge_current_limit }; 
  getIndexLimit(soc, indices, inferiorSOC, 5 );
  data=proporcion(soc,  inferiorSOC[indices[0]], inferiorSOC[indices[1]], limiteDescarga[indices[0]], limiteDescarga[indices[1]]);
  jk_bms_battery_info.battery_limits.battery_discharge_current_limit=data;
}

void ajustarAmperiosCargaDescarga(){
  const int INF=0;
  const int SUP=1;
  uint8_t indice[2]={0};
  uint16_t amperios=0;
  uint16_t rampaAmperios[5]={0};
  uint16_t rampaEscala[5]={0};
  uint16_t soc=jk_bms_battery_info.battery_status.battery_soc;
  uint16_t avrcells=jk_bms_battery_info.cell_Vavrg;
  uint16_t valor=0;
  uint8_t escala;
  //rampa carga 
  if(configuracion.bateria.rampaCarga_mV){
      valor=avrcells;    
      escala=mV;
  }else{
      valor=soc;
      escala=SOC;
  }
  for(int i=0; i<5; i++){
    rampaAmperios[i]=configuracion.bateria.rampaCarga.norma[i].valor[Amp];
    rampaEscala[i]=configuracion.bateria.rampaCarga.norma[i].valor[escala];
  } 
  getIndexLimit(valor, indice, rampaEscala, 5);
  amperios=proporcion(valor, rampaEscala[indice[INF]], rampaEscala[indice[SUP]],
                            rampaAmperios[indice[INF]], rampaAmperios[indice[SUP]]);
    
  jk_bms_battery_info.battery_limits.battery_charge_current_limit=amperios;


  //rampa descarga
  
  if(configuracion.bateria.rampaDescarga_mV){
      valor=avrcells;    
      escala=mV;
  }else{
      valor=soc;
      escala=SOC;
  }
  for(int i=4, z=0; i>-1; i--, z++){
    rampaAmperios[z]=configuracion.bateria.rampaDescarga.norma[i].valor[Amp]; //voltear los array
    rampaEscala[z]=configuracion.bateria.rampaDescarga.norma[i].valor[escala];
  } 
  getIndexLimit(valor, indice, rampaEscala, 5);
  amperios=proporcion(valor, rampaEscala[indice[INF]], rampaEscala[indice[SUP]],
                            rampaAmperios[indice[INF]], rampaAmperios[indice[SUP]]);
  jk_bms_battery_info.battery_limits.battery_discharge_current_limit=amperios;                      
  
}

void bajaCapacidad(){
  if(jk_bms_battery_info.battery_status.battery_soc < configuracion.bateria.nivelSOCbajo){
    jk_bms_battery_info.battery_alarms.low_capacity=true;
  }
}