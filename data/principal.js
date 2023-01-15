//declarción de elementos html

var array_celdas_id=["celda1", "celda2", "celda3", "celda4", "celda5", "celda6", "celda7", "celda8",
					"celda9", "celda10", "celda11", "celda12", "celda13", "celda14", "celda15", "celda16",
					"celda17", "celda18", "celda19", "celda20", "celda21", "celda22", "celda23", "celda24"];


var  opcionmenu=document.getElementById("opcionmenu");


const  servidorJson = new XMLHttpRequest();


servidorJson.onload=function(){
	//if(this.readyState!=4 && this.status!=200)return;
	if(opcionmenu.value=="home"){
		try{
			var objs=JSON.parse(this.responseText);
			rellenarDatosBateria(objs);
			//console.log("PARSE_OK")
		}catch ( e){
			console.log("ERROR:"+ e);
		}
		return;	
	}
}



function peticion3sg(){
	setTimeout(peticion3sg, 3000);
	if(opcionmenu.value=="home"){
		servidorJson.open("GET", "estadoactualbateria.json");
		servidorJson.send();
		//console.log("peticion:estadoactualbateria.json");
	}
}



function rellenarDatosBateria(jsondoc){
	//si no hay lectura
	let numeroCeldas=jsondoc["number_cells"];
	
	//if(numeroCeldas==0){
	//	document.getElementById("tabla_celdas").innerHTML="<img src='wait.png'>";
	//	return;
	//}
	

	document.getElementById("t0").innerText= "T mostf: "+ jsondoc["mosfet_temp"] + "ºC";
	document.getElementById("t1").innerText="T batt1: " + jsondoc["temp1"] + "ºC";
	document.getElementById("t2").innerText="T batt2: " + jsondoc["temp2"] + "ºC";
	document.getElementById("batt_vol").innerText=(jsondoc["batt_vol"]/100) + "V";
	document.getElementById("batt_amp").innerText=(jsondoc["batt_amp"]/100) + "A";
	document.getElementById("batt_power").innerText=(
		((jsondoc["batt_vol"]/100) * (jsondoc["batt_amp"]/100))  ).toFixed(2)+"W";//dos decimales 
	document.getElementById("soc").innerText="SOC: " + jsondoc["soc"] + "%";
	document.getElementById("soh").innerText="SOH: " + jsondoc["soh"] + "%";
	document.getElementById("batt_ah_count").innerText="METER: " + jsondoc["batt_ah_count"] + "AH";
	document.getElementById("cell_voltage_average").innerText="Avrg cells: " + jsondoc["cell_voltage_average"] + "mV";
	document.getElementById("delta_cell_voltage").innerText="Delta cells: " + jsondoc["delta_cell_voltage"] + "mV";
	document.getElementById("batt_cycle").innerText="Cycles: " + jsondoc["cycles"];

	let ncellmin=jsondoc["cell_number_vmin"];
	let ncellmax=jsondoc["cell_number_vmax"];
	
	for(let i=0; i<numeroCeldas; i++){
		document.getElementById(array_celdas_id[i])
			.innerText="Cell-"+(i+1)+":  "+jsondoc[array_celdas_id[i]] + "mV";
		document.getElementById(array_celdas_id[i]).style.backgroundColor="lightslategray";	
	}
	if(numeroCeldas > 0){
		document.getElementById(array_celdas_id[ncellmin-1]).style.backgroundColor="red";
		document.getElementById(array_celdas_id[ncellmax-1]).style.backgroundColor="blue";
	}
	const list_alarm=document.getElementsByClassName("alarm");
	Array.from(list_alarm).forEach(element => {
		element.style.backgroundColor="lightslategray";
	});
	if(jsondoc["habilitarCarga"]==true)document.getElementById("habilitarCarga").style.backgroundColor="green";
	if(jsondoc["habilitarDescarga"]==true)document.getElementById("habilitarDescarga").style.backgroundColor="green";
	if(jsondoc["lowCapacity"]==true)document.getElementById("lowCapacity").style.backgroundColor="orange";
	if(jsondoc["mosfet_overtemp"]==true)document.getElementById("mosfet_overtemp").style.backgroundColor="red";
	if(jsondoc["OVC"]==true)document.getElementById("OVC").style.backgroundColor="red";
	if(jsondoc["OCC"]==true)document.getElementById("OCC").style.backgroundColor="red";
	if(jsondoc["OVcell"]==true)document.getElementById("OVcell").style.backgroundColor="red";
	if(jsondoc["UVD"]==true)document.getElementById("UVD").style.backgroundColor="red";
	if(jsondoc["OCD"]==true)document.getElementById("OCD").style.backgroundColor="red";
	if(jsondoc["UVcell"]==true)document.getElementById("UVcell").style.backgroundColor="red"; 
	if(jsondoc["HighTemp"]==true)document.getElementById("HighTemp").style.backgroundColor="red";
	if(jsondoc["LowTemp"]==true)document.getElementById("LowTemp").style.backgroundColor="red";
	if(jsondoc["ODeltaCell"]==true)document.getElementById("ODeltaCell").style.backgroundColor="red";
	if(jsondoc["comRS485_JK"]==true)document.getElementById("comRS485_JK").style.backgroundColor="red";
}


function limpiarventana(){
	document.getElementById("estado_div").style.display="none";
	document.getElementById("ajustes_div").style.display="none";
	document.getElementById("reglas_div").style.display="none";
	document.getElementById("acercade_div").style.display="none";
}

function habilitarLimitesCargaDescarga(){
	var check=document.getElementById("checkcustomtension").checked;
	if(check == true){
		document.getElementById("tensionMaximaCarga").removeAttribute("readonly");
		document.getElementById("tensionMinimaDescarga").removeAttribute("readonly");
	}
}

/* 
	Funciones de menú:
	1º cambia la variable del html opcionmenu
	2º llama al servidor para cargar la página
	3º cuando se produce el callback window.addListener('load',) se llama
		a la función correspondiente para la opción de menu
	4º Con los datos cargados previamente, se oculta toda la página y solo se muestra 
		el bloque html correspondiente
*/

function clickHome(){
	if(opcionmenu.value !="home"){
		window.location.href="./principal";
		opcionmenu.value="home";
		return;
	}
	limpiarventana();
	document.getElementById("estado_div").style.display="contents";
	document.getElementById("clickHome").style.color="white";
	opcionmenu.value="home";
}	

function clickAjustes(){	
	if(opcionmenu.value != "ajustes"){
		window.location.href="./ajustes";
		opcionmenu.value="ajustes";
		return;
	}
	limpiarventana();
	document.getElementById("ajustes_div").style.display="contents";
	document.getElementById("clickAjustes").style.color="white";
	opcionmenu.value="ajustes";
}

function clickReglas(){
	if(opcionmenu.value !="reglas"){
		window.location.href="./reglas";
		opcionmenu.value="reglas";
		return;
	}
	limpiarventana();
	document.getElementById("reglas_div").style.display="contents";
	document.getElementById("clickReglas").style.color="white";
	opcionmenu.value="reglas";
	habilitarLimitesCargaDescarga(); 
}

function clickAcercade(){
	if(opcionmenu.value !="acercade"){
		window.location.href="./acercade";
		opcionmenu.value="acercade";
		return;
	}
	opcionmenu.value="acercade";
	limpiarventana();
	document.getElementById("acercade_div").style.display="contents";
	document.getElementById("clickAcercade").style.color="white";
}
	

window.addEventListener('load', function(){
	peticion3sg();
	limpiarventana();
	if(opcionmenu.value=="home" || opcionmenu.value==="") clickHome();
	if(opcionmenu.value=="ajustes") clickAjustes();
	if(opcionmenu.value=="reglas")clickReglas();
	if(opcionmenu.value=="acercade")clickAcercade();
	
});

window.onmessage = (event)=>{
	console.log("Mensaje:" + event.data);
};




