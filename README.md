# JKbms_Emu_Pylon
¡¡ATENCIÓN NO USE ESTE SOFTWARE, USTED ES EL RESPONSABLE DE LOS DAÑOS PERSONALES O MATERIALES QUE PUEDAN SUCEDER!!<br>
Interfaz con esp32 entre JK e inversor Ingeteam, emulando protocolo pylon LV y HV.<br>
Software solo para pruebas y/o apredizaje, NO SE RECOMIENDA SU USO.<br> 
Su autor no tiene experiencia ni los conocimientos adecuados, no se resposabiliza de cualquier daño personal o material.<br>
El código fuente es experimental no sigue ningún estandar, está desorganizado y con variables y/o funciones sin terminar o utilizar.<br>

# Conexión directa
JK-bms Vbat------X<br>
JK-bms TX----------GPIO16(RXD_2) ESP32 <br>
JK-bms RX----------GPIO17(TXD_2) ESP32 <br>
JK-bms GND-------------------GND ESP32 <br>

<br>
ESP32 (GPIO33)------CAN-TX----------(tx)TJA1050(CAN-H)-----------(CAN-H)Inversor<br>
ESP32 (GPIO32)------CAN-RX----4k7---(rx)TJA1050(CAN-L)-----------(CAN-L)Inversor<br>
ESP32 GND--------------------------(gnd)TJA1050<br>
ESP32 +5v--------------------------(+5v)TJA1050<br>

# Instrucciones

Pruebas con módulo ESP32 DevKit v1<br>
Conectar a la WiFi del ESP32 "JKBMS_EMU_PYLONTECH" (parpadeo led azul) y configurar SSID y PASSWORD de WiFi local (fijo led azul).<br>
Buscar ip del esp32 en wifi local y acceder a su página web para configurar.<br>
Hacer pruebas de funcionamiento.<br>



# DONE List 
Se prueba el protocolo de alto voltaje con exito. El protocolo de bajo, por mi configuración de batería(24s) el inversor muestra un error.<br>
Se prueba la rampa de carga con exito, el inversor adapta la corriente carga a los valores configurados en el esp32, los valores de corriente enviados al inversor entre escalas de SOC, es proporcional<br>
Se prueba con exito el control ante un posible fallo de comunicación entre en esp32 y la jkbms para poder parar toda actividad con el inversor.<br>
Se prueba con exito la perdida de conexión CAN y reconexión con el inversor<br>
Se añade publicación de mensaje MQTT con el estado de la batería<br>
Se puede configurar los voltajes de carga y descarga diferentes a los proporcionados por el bms JK<br>
Ahora las rampas de carga/descarga se pueden establecer 5reglas para adaptar la intensidad en función del SOC o voltaje medio de celda<br>


# TODO List
<ul>
<li>Mejorar la interfaz web, página de configuración Wifi, etc.</li>
<li>Añadir mensajes para INFLUXDB.</li>
<li>Añadir pantalla led para mostrar datos básicos.</li>
<li>Poner la batería en reposo/activa cuando lo solicite el inversor.</li>
</ul>

# Dependencies
# References
<ul>
<li>https://github.com/syssi/esphome-jk-bms copiar partes de su código y adaptarlo a mi proyecto</i>
<li>https://secondlifestorage.com/index.php?threads/jk-b1a24s-jk-b2a24s-active-balancer.9591/</i>
<li>https://github.com/maxx-ukoo/jk-bms2pylontech copiar partes de su código y adaptarlo a mi proyecto</li>
<li>https://github.com/stuartpittaway/diyBMSv4ESP32 copiar partes de su código y adaptarlo a mi proyecto</li>
<li>https://github.com/pablozg/freeds copiar partes de su código y adaptarlo a mi proyecto</li>
<ul>
  
