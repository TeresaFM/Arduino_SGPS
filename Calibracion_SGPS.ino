/*Programa de calibracion del dispositivo SGPS, para obtener los valores umbrales mediante
los que el programa SGPS identificar´ la hora de amanecer y anochecer*/


#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
#include <SPI.h>


int bisiesto(int anno);
int dias_de_mes(int m, int anno);

String filename;
String ext;
char myfile[17];
int cont=0;
const float LatitudReal= 40.15359;  //Coordenadas de Ciempozuelos, Madrid, España donde se realiz´ la calibracion
const float LongitudReal= -3.60009;

int CS=10;
String dataString;
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


void setup() 
{

 Serial.begin(9600);
 Wire.begin();                             //Inicializacion del puerto I2C, para la comunicacion con la tarjeta SD
 RTC.begin();                              //Inicializacion del reloj RTC
 RTC.adjust(DateTime(__DATE__, __TIME__)); // Ajuste de l ahora con la hora del ordenador
  Serial.println("Iniciando lectura tarjeta SD");   //Abre tarjeta de memoria para comprobar que esta OK 
  pinMode(CS, OUTPUT);
  if(!SD.begin(CS))
  {
    Serial.println("Tarjeta erronea");
    return;
  }
  Serial.println("Tarjeta OK");
   
}
void loop()
{
  
  DateTime now = RTC.now();
  anio = now.year();           //Se obtiene la fecha y hora del RTC
  mes = now.month();
  dia = now.day();
  hora = now.hour();
  minuto = now.minute();        //Se pasa la hora a decimal ya que el modelo celectial mas tarde la necesitara en este formato
  min_dec = (minuto/60);
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
   sensorval = analogRead(sensorReading);           //Se obtiene el valor del sensor
   sensorval= (sensorval*5)/1024;
   media=media+sensorval/3;                         //Se toman datos cada 20 segundos, 3 datos cada hora, asi                                                    
   delay(20000);                                     //Calculo de la media de esos tres y obtengo un valor medio/minut
  }
  filename="generic";
  ext=".txt";
 
  if(hora_dec==0.00)                                 //Se abre un archivo nuevo cada dia, numerados 
         {cont++;
          filename=filename+cont+ext;
          filename.toCharArray(myfile,17);
          File dataFile=SD.open(myfile, FILE_WRITE);   //Se abre la tarjeto en modo escritura para guardar todos los datos y la hora a la que se tomaron
            if(dataFile)   
            {
             dataFile.println("genericdec");
             dataFile.print(LongitudReal);
             dataFile.print("   ");
             dataFile.println(LatitudReal);
             dataFile.print(dia);
             dataFile.print("   ");
             dataFile.print(mes);
             dataFile.print("   ");
             dataFile.println(anio);
             dataFile.print(hora_dec); 
             dataFile.print("  ");  
             dataFile.println(media);  
             dataFile.close();
            }
            else
            {Serial.println("Escritura Erronea...");}
        }
 else
      {filename=filename+cont+ext;
       filename.toCharArray(myfile,17);
       File dataFile=SD.open(myfile, FILE_WRITE);   
       if(dataFile) 
        {dataFile.print(hora_dec);
         dataFile.print("  ");  
         dataFile.println(media);  
         Serial.print(hora_dec);
         Serial.print("  ");
         Serial.println(media);
         dataFile.close();
        }
       else
     { Serial.println("Escritura Erronea..."); }     
  }
}

int bisiesto(int anno){
  
  if((anno%4==0 && anno%100!=0)||(anno%400==0)){
    return 1;
  }
  else{
    return 0;
  }
  
}

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
