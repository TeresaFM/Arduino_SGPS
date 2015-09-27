/*Programa desarrollo en Arduino del SGPS.

Se obtienen las coordenadas geograficas con
solo la informacion de la hora, obtenida mediante un RTC. La informacion es guardada 
cada dia en un nuevo archivo que se creara en una tarjeta de memoria SD. Es necesario
introducir las coordenadas reales donde se va a poner a medir el dispositivo de SGPS ya 
que el programa tambi´n calcula el error que existe entre las cooredenadas que ha calculado
y las reales.

Tambien se ha de tener en cuenta que los valores umbrales light_sr y lights_ss se han establecido
mediante la calibracion del dispositivo, con el programa disponible en Calibracios_SGPS en este
mismo repositorio.*/


#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <math.h>

#define MIN2HOUR 24.0f/1440.0f
#define DEG2RAD M_PI/180.f

int bisiesto(int anno);
int dias_de_mes(int m,int anno);

String filename;
String ext;
char myfile[17];
int cont=0;
const float LatitudReal= 40.15359;
const float LongitudReal= -3.60009;

int CS=10;
String dataString;
File  dataFile;
const int sensorReading = 0;
float sensorval = 0;
RTC_DS1307 RTC;

int anio;              
int mes;
int dia;
int fecha;
int UTC = 2;         //En España zona horaria es UTC+2
int contmes;

float minuto=0;      
float hora=0;
float min_dec;
float hora_dec;

int cuenta=0;
int cuenta2=0;
float anterior1=0;
float anterior2=0;
float anterior3=0;
float anterior4=0;

float light_sr=0.85;
float light_ss=0.10;
float sr=0;
float ss=0;

float hora_SR=0;
float minuto_SR=0;
float hora_SS=0;
float minuto_SS=0;



void setup(){
  
 Serial.begin(9600);
 Wire.begin();                                     //Inicializacion del reloj
 RTC.begin();
 RTC.adjust(DateTime(__DATE__, __TIME__));         // Establece la fecha y hora
 Serial.println("Iniciando lectura tarjeta SD");   //Se comprueba que tarjeta esta correcta tarjeta
 pinMode(CS, OUTPUT);
 if(!SD.begin(CS))
   { Serial.println("Tarjeta erronea");
     return;
   }
 Serial.println("Tarjeta OK");
 
}
void loop()
{
  
  DateTime now = RTC.now();
  anio = now.year();           //Obtencion de la fecha y hora del RTC
  mes = now.month();
  dia = now.day();
  hora = now.hour();
  minuto = now.minute();     
  min_dec = (minuto/60);          //Se convierte la hora a decimal, se necesita asi para introducirla en el modelo celestial.
  hora = hora-UTC;
  if(hora<0)
  {
    hora=hora+24;
  }
  hora_dec = hora+min_dec;
  float media=0;
  int xdias = dia;
  int f;
  contmes=mes;
  while(contmes>=2)
    {
      contmes=contmes-1;
      f=dias_de_mes(contmes,anio);
      xdias=xdias + f;
    }
    
  for(int sampleT = 1 ; sampleT<=3 ; sampleT++) 
  {
   sensorval = analogRead(sensorReading);               //se obtiene valor del sensor
   sensorval= (sensorval*5)/1024;
   media=media+sensorval/3;                         //Se toman datos cada 20 segundos, 3 datos cada hora, asi                                                    
   delay(20000);                                      //hago la media de esos tres y obtengo un valor medio/minut
  }
   
 anterior1=sensorval;
 anterior2=anterior1;
 anterior3=anterior2;
 anterior4=anterior3;
 if(sensorval>anterior1&&anterior1>anterior2&&anterior2>anterior3&&anterior3>anterior4)
   {if(sensorval>=light_sr)
     {cuenta++;
      if(cuenta==1)
        {sr=hora_dec;}
     }
   }
 if(sensorval<anterior1&&anterior1<anterior2&&anterior2<anterior3&&anterior3<anterior4)
   {if(sensorval<=light_sr)
     {cuenta2++;
      if(cuenta2==1)
        {ss=hora_dec;}
     }
   }  
  int midDay=(sr+ss)/2;
  if (sr>=ss)                                  //Si es un dia fraccionado
   midDay=midDay+12;
  if (midDay>=24)
   midDay=midDay%24;

 filename="generic";
 ext=".txt";
 if(hora_dec==0.00)
 {cont++;
  cuenta=0;
  cuenta2=0;
  filename=filename+cont+ext;                  //Gracias a este contador, se crear´ un archivo nuevo, numerado, cada nuevo dia que pase
  filename.toCharArray(myfile,17);
  File dataFile=SD.open(myfile, FILE_WRITE);  //Se abre la tarjeta SD para guardar todos los datos
  if(dataFile)
     {float Beta;
      float Delta;
      float angularss;
      float EqT;
      float Longitud;
      float Latitud;
      float eLongitud;
      float eLatitud;
      float eKmLong;
      float eKmLat;
      float epercLong;
      float epercLat;
      Beta = anioFrac(xdias);
      Delta = sunDeclination(Beta);
      angularss = angularSunset(ss,midDay);
      Longitud = cLongitud(midDay);
      Latitud = cLatitud(Delta, angularss);
      eLongitud= errordegreesLongitud(Longitud, LongitudReal);
      eLatitud= errordegreesLongitud(Latitud, LatitudReal);
      eKmLong=errorKmLong(eLongitud);
      eKmLat=errorKmLat(eLatitud);
      epercLong=errorpercentLong(eLongitud);
      epercLat=errorpercentLat(eLatitud);
      dataFile.print(Beta);
      dataFile.print("  "); 
      dataFile.print(Delta);
      dataFile.print("  ");
      dataFile.println(angularss); 
      dataFile.print(sr);
      dataFile.print("  "); 
      dataFile.println(ss);
      dataFile.print("Longitud y latitud reales:  "); 
      dataFile.print(LongitudReal); 
      dataFile.print("  "); 
      dataFile.println(LatitudReal); 
      dataFile.print(Longitud); 
      dataFile.print("  "); 
      dataFile.println(Latitud);
      dataFile.print("Errores por orden, gradosLong gradosLat, KmLong KmLat, %Long %Lat:");
      dataFile.print(eLongitud);
      dataFile.print(" "); 
      dataFile.print(eLatitud); 
      dataFile.print(" ");   
      dataFile.print(eKmLong);
      dataFile.print(" "); 
      dataFile.print(eKmLat); 
      dataFile.print(" ");   
      dataFile.print(epercLong);
      dataFile.print(" "); 
      dataFile.println(epercLat);   
      dataFile.print(dia); 
      dataFile.print("  ");  
      dataFile.print(mes);  
      dataFile.print("  ");  
      dataFile.println(anio);
      dataFile.print(hora_dec); 
      dataFile.print("  ");  
      dataFile.println(sensorval); 
      dataFile.close();
    }
  else
    {Serial.println("Escritura erronea...");}
 }
 else
 {filename=filename+cont+ext;
  filename.toCharArray(myfile,17);
  File dataFile=SD.open(myfile, FILE_WRITE);
  if(dataFile)
   {dataFile.print(hora_dec); 
    dataFile.print("  ");  
    dataFile.println(sensorval);  
    Serial.print(hora_dec);
    Serial.print("  ");
    Serial.println(sensorval);
    dataFile.close();
    }
  else
   {Serial.println("Escritura Erronea...");}
}
 delay(2000);
}



float anioFrac(const unsigned int ndias){
   return 2*M_PI/365*(ndias);
  
}

/*Obtencion de la cooredenada de Longitud*/

float cLongitud(const float mDay)
{
  float calcLongitud = (180/12*(12-mDay));
  return calcLongitud;
}

/*Obtencion de la coordenada de latitud*/

float cLatitud(const float delt, const float asunset){
  float min_error = 1000;
  float best_lat;
  float latitud = 0;
  for (latitud = -90; latitud <= 90; latitud = latitud+0.1){
    float error_now = fabs(((sin(-0.83*DEG2RAD)-sin(latitud*DEG2RAD)*sin(delt))/(cos(latitud*DEG2RAD)*cos(delt))) - cos(asunset));
		if (error_now < min_error) {
			min_error = error_now;
			best_lat = latitud;
                  }
  }
  return best_lat;
}

/*Se obtiene la declinacion solar*/

float sunDeclination(const float beta){
  return 0.006918f-0.399912f*cos(beta)+0.070257f*sin(beta)-0.006758f*cos(beta*2)+0.000907f*sin(beta*2)-0.002697f*cos(beta*3)+0.00148f*sin(beta*3);
}
   

float angularSunset(const float sunset, const float mDay){
  return M_PI/12*fabs(sunset-mDay);
}

/*A continuacion las funciones para el calculo del error de longitud y de latitud
en grados, en kilometros y en porcentaje*/

float errordegreesLongitud(float Long, float LongReal){
  float error;
  error=LongReal-Long;
  return error*100;
}
float errordegreesLatitud(float Lat, float LatReal){
  float error;
  error=LatReal-Lat;
  return error*100;
}

float errorKmLong(float errordegreesLongitud){
  float error;
  error=errordegreesLongitud*111,11;
  return error;
}

float errorKmLat(float errordegreesLatitud){
  float error;
  error=errordegreesLatitud*111,11;
  return error;
}

float errorpercentLong(float errordegreesLongitud){
  float error;
  error=(errordegreesLongitud/360)*100;
  return error;
}

float errorpercentLat(float errordegreesLatitud){
  float error;
  error=(errordegreesLatitud/180)*100;
  return error;
}

/*Funcion que retorna si el año introducido es bisiesto*/

int bisiesto(int anno){           
  
  if((anno%4==0 && anno%100!=0)||(anno%400==0)){
    return 1;
  }
  else{
    return 0;
  }
  
}

/* Funcion que devuelve los dias que tiene un mes*/

int dias_de_mes(int m, int anno){        
 int xdias; 
  if(m==4 || m==6 || m==9 || m==11)
  { xdias=30;
    return xdias;
  }
  else
  {
    if(m==2)
    {
      if(bisiesto(anno)==1)
      {
        xdias=29;
        return xdias;
      }
      else
      {
        xdias=28;
        return xdias;
      }
    }
    else
    {
      xdias=31;
      return xdias;
    }
  }
}


