/*
  _  _               _     _ _ _         
 _| || |_  __ _  __ _| | __| (_) |_ _   _ 
|_  ..  _|/ _` |/ _` | |/ _` | | __| | | |
|_      _| (_| | (_| | | (_| | | |_| |_| |
  |_||_|  \__, |\__,_|_|\__,_|_|\__|\__,_|
          |___/       
  sagarrabanana@bizkaia.eu asier@komunikatik.com
  10/06/2015
  
*/                                                                        

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                    Librerias, constantes (define) y variables                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <GSM.h> /Librera FRM de Arduino

// Datos de acceso APN para laa conexion de daatos
#define GPRS_APN       "sm2ms.movilforum.es" // indicar el APN del proveedor GPRS
#define GPRS_LOGIN     ""  // indicar el login del proveedor GPRS
#define GPRS_PASSWORD  ""  // indicar password del proveedor GPRS
#define PINNUMBER      ""       // en blanco si la SIM no tiene PIN

// Se inicializa la librera GSM de Arduino    http://arduino.cc/en/Reference/GSM
GSMClient client;
GPRS gprs;
GSM gsmAccess; 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                   Variables                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables de configuracion:                                                                                    //
// ===========================                                                                                    //
// GPRS_APN          = APN de proveedor de servicios                                                              //
// tiempoInstalacion = Tiempo de margen tras el encendido para comenzar a contar                                  //
// servidor          = Sitio web que almacena la informaicion de las detecciones                                  //
// panel_info        = Nombre del panel informatvo                                                                //
// intervaloEnvio    = Ciclo de tiempo para realizar envios                                                       // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//============ Variables personalizables ============
unsigned long tiempoInstalacion=300000;    // Disponemos de 5 minutos para ajustar el sensor antes de que comience a realizar detecciones
long          intervaloEnvio=37200000;     // ciclo de tiempo para enviar valor del contador
char          servidor[] = "www.??????.com";  // Direccion de la plataforma IoT Web
char          panel_info[]= "Galditu";     // <= ENTRE COMILLAS INDICAMOS EL NOMBRE DEL PANEL CADA VEZ QUE CAMBIEMOS DE SITIO A GALDITU

//============ Variables para control del sensor ============
long          ultimoEnvio=0;              // Ultimo envio
long unsigned int lowIn;                  // Es el tiempo en el que el sensor detecta movimiento
long unsigned int pausa = 5000;           // Es el tiempo de pausa que el sensor inhibe lecturas
boolean       lockLow = true;
boolean       takeLowTime;

//============ Variables para indicar URL y fichero de comprobacion de envio ============
int     contador=0;
String  path = "/??????.php?variable1=";

//============ Variables para control de pines en Arduino ============
int     pirPin = 9;    // Ees el pin que se conecta con el sensor PIR
int     boton=10;      // boton para enviar datos antes de apagar
int     ledEnvio=12;   // Led blanco (se enciende durante el envio)
int     ledPIR = 13;   // Led rojo (aviso de deteccion)
boolean estadoBoton=LOW;
boolean estadoBotonCambio=LOW;
boolean sensorEscucha=false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                   Setup                                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  pinMode(boton,INPUT);     // Inicializa pin de boton de envio, en modo entrada
  pinMode(ledEnvio,OUTPUT); // Inicializa pin del led de envio en modo salida
  pinMode(ledPIR,OUTPUT);   // Inicializa pin del led del sensor PIR en modo salida
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                 Conectar a internet                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void conectar()
{
  Serial.println("*******************************************");
  Serial.println("* Levantando comunicaciones GSM           *");
  digitalWrite(2,HIGH); // Apaga Modem
  digitalWrite(3,HIGH); // Enciende modem
  char myGsmAccess=gsmAccess.begin(PINNUMBER,true,false);
  digitalWrite(ledEnvio,HIGH); // Enciende led para indicar envio
  delay(15000);
  digitalWrite(ledPIR,HIGH); // Enciende led para indicar envio  
  delay(15000);  
  Serial.println("* Levantando comunicaciones GPRS          *");
  char myGprs=gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD);
  delay(10000);
  digitalWrite(ledPIR,LOW); // Enciende led para indicar envio  

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                 Enviar valor del contador                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void enviar()
{
///////////////////////////////////////////////////////////////////////////////////
// Comenzamos la concatenacion de URL y parametros que enviamos a la plataforma IoT
///////////////////////////////////////////////////////////////////////////////////

  path=("/?????.php?variable1=");
  path.concat(contador);
  path.concat("&variable2=");
  path.concat(panel_info);
  boolean envioOk = false; // retorna resultado del envio
  
///////////////////////////////////////////////////////////////////////////////////
// Se el resultado de la conexion http es correcto, continuamos con el envio 

  if (client.connect(servidor, 80))
  {
   
    Serial.println("     ======================================"); // Envia contenido de variable de contador de actividad
    Serial.println("     =     Inicio de llamada HTTP         =");
    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(servidor);
    delay(1000);
    client.println("Connection: close");
    client.println();    
    
  } 


///////////////////////////////////////////////////////////////////////////////////
// Bucle de lectura de la respuesta del servidor
///////////////////////////////////////////////////////////////////////////////////
while(client.available() || client.connected())
  {
  char c = client.read();
  if (c == '@') // En la plataforma IoT se ha definido este caracter para confirma la entrada correcta en la base de datos
    {
    envioOk = true;
    }
  }
  if (envioOk == true) 
  {
    Serial.println("     =     Alta confirmada                =");
    contador = 0;
  }
  else
  {
    Serial.println("     =     Alta no confirmada             =");
  }  


///////////////////////////////////////////////////////////////////////////////////
// Cierra la conexion GSM
///////////////////////////////////////////////////////////////////////////////////
  client.stop(); // Detiene conexion gsm
  delay(5000);  
  gsmAccess.shutdown();
  delay (1000);
  digitalWrite(2,LOW); // Apaga Modem
  digitalWrite(3,LOW); // Apaga Modem
  digitalWrite(ledEnvio,LOW); // Apaga led para indicar envio
  Serial.println("     =     Fin de conexion                ="); 
  Serial.println("     ======================================");
  Serial.println("*******************************************");  
///////////////////////////////////////////////////////////////////////////////////  
}


void loop()
{
///////////////////////////////////////////////////////////////////////////////////   
//  En esta parte determinamos si estamos en el tiempo de instalacion del sensor
/////////////////////////////////////////////////////////////////////////////////// 
  if (millis() <= tiempoInstalacion)
  {
    Serial.println("Ajusta sensor");
  }
  else
  {
    if (sensorEscucha == false)
    {     
      Serial.println("*******************************************");
      Serial.println("*    Sensor a la escucha                  *");
      Serial.println("*******************************************");
      sensorEscucha = true;
    }  
  }
/////////////////////////////////////////////////////////////////////////////////// 


  if(digitalRead(pirPin) == HIGH) // Si el recibimos seÃ±al del PIR encenemos el led de deteccin (el Rojo).
  {
    if (sensorEscucha == false) 
    {
      digitalWrite(ledPIR, HIGH); // enciende el LED de deteccion (ROJO) y no acumula
    }
    else
    {  
      if(lockLow)
      {  
         //makes sure we wait for a transition to LOW before any further output is made:
         lockLow = false;            
         Serial.println("     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
         Serial.print("     Inicio de actividad a los ");
         contador++;
         Serial.print(millis()/1000);
         Serial.println(" segundos"); 
         delay(50);
      }
      takeLowTime = true;
    }   
 }  

  if(digitalRead(pirPin) == LOW)
    digitalWrite(ledPIR, LOW); // apaga el LED de deteccion (ROJO)
    {       
      if(takeLowTime)
      {
        lowIn = millis();          //guardar el momento de la transiciÃ³n de HIGH a LOW
        takeLowTime = false;       //se asegura de que esto sÃ³lo se hace en el inicio de una fase LOW
      }
      //if the sensor is low for more than the given pausa, 
      //we assume that no more motion is going to happen
      if(!lockLow && millis() - lowIn > pausa)
      {  
        lockLow = true;
        Serial.print("     fin de actividad a los ");           
        Serial.print((millis() - pausa)/1000);
        Serial.println(" segundos");
        Serial.print("     Total detecciones: ");
        Serial.println(contador);  
        Serial.println("     <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");   
        delay(50);
      }
  }

  
// Lee entrada digital para detectar cambio en estado del boton de envio
  estadoBoton=digitalRead(boton);
// Si se ha superado el ciclo de envio, activa variable de pulsacion de boton para enviar de nuevo
  if (millis()- ultimoEnvio >= intervaloEnvio)
  {
    ultimoEnvio=millis();
    estadoBoton=HIGH;
  }     
// Detecta cambio en valor del boton pulsado y ademas detecta cambio a encendido
  if ((estadoBoton!=estadoBotonCambio) && (estadoBoton==HIGH))
  {
// Envia contenido de variable de contador de actividad
   Serial.println("*******************************************");
   Serial.println("* Boton de envio pulsado, inicia conexion *");
   conectar();
   enviar();
  }
// Guarda variable aanterior para poder detectar cambio de valor en la pulsacion del boton 
  estadoBotonCambio=estadoBoton;
  
}
