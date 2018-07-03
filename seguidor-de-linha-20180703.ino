#include <QTRSensors.h>


//Mapeo de pines
#define STBY 6
#define AIN1  5
#define AIN2  4
#define PWMB  9
#define PWMA  3
#define BIN1  8
#define BIN2  7
#define NUM_SENSORS             6
#define NUM_SAMPLES_PER_SENSOR  4
#define EMITTER_PIN             11
#define LED     13
#define S_DIR A0
#define S_ESQ A7

// Constantes para PID
float KP = 0.10;
float KD = 1.10;
float Ki = 0.0366;

// Regulación de la velocidad Máxima
int Velmax = 77;

// Data para intrgal 
int error1=0;
int error2=0;
int error3=0;
int error4=0;
int error5=0;
int error6=0;

// Data para S_ESQ
int sesq1=0;
int sesq2=0;
int sesq3=0;
int sesq4=0;
int sesq5=0;
int sesq6=0;
int sesq7=0;
int sesq8=0;
int sesq9=0;
int sesq10=0;

// Data para S_DIR
int sdir1=0;
int sdir2=0;
int sdir3=0;
int sdir4=0;
int sdir5=0;
int sdir6=0;
int sdir7=0;
int sdir8=0;
int sdir9=0;
int sdir10=0;

int cruzamento = 0;

// Configuración de la librería QTR-8A         
QTRSensorsAnalog qtra((unsigned char[]) 
  {A1, A2, A3, A4, A5, A6}
, NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);

unsigned int sensorValues[NUM_SENSORS];

// Función accionamiento motor izquierdo
void Motoriz(int value)
{
  if ( value >= 0 )
  {
    digitalWrite(BIN1,HIGH);
    digitalWrite(BIN2,LOW);
  }
  else
  {
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
    value *= -1;
  }
  analogWrite(PWMB,value);
}

// Función accionamiento motor derecho
void Motorde(int value)
{  
  if ( value >= 0 )
  {
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
  }
  else
  {
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,HIGH);
    value *= -1;
  }    
  analogWrite(PWMA,value);
}



//Accionamiento de motores
void Motor(int left, int righ)
{
  digitalWrite(STBY,HIGH);
  Motoriz(left);
  Motorde(righ);
}



//función de freno
void freno(boolean left, boolean righ, int value)
{
  digitalWrite(STBY,HIGH);
  if ( left )
  {
    digitalWrite(BIN1,HIGH);
    digitalWrite(BIN2,HIGH);
    analogWrite (PWMB, value);
  }
  if ( righ )
  {
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,HIGH);
    analogWrite (PWMA, value);
  }
}

int somadireita;
int somaesquerda;

void soma()
{
 somadireita = sdir1+sdir2+sdir3+sdir4+sdir5+sdir6+sdir7+sdir8+sdir9+sdir10;
 somaesquerda = sesq1+sesq2+sesq3+sesq4+sesq5+sesq6+sesq7+sesq8+sesq9+sesq10;
}

void chegada_partida() 
{
  if(somadireita < 2500 && somaesquerda > 5000){
    delay(2000);
  }
}



void setup()
{
// Declaramos como salida los pines utilizados
  pinMode(LED   ,OUTPUT);
  pinMode(BIN2  ,OUTPUT); 
  pinMode(STBY  ,OUTPUT);
  pinMode(BIN1  ,OUTPUT);
  pinMode(PWMB  ,OUTPUT);
  pinMode(AIN1  ,OUTPUT);
  pinMode(AIN2  ,OUTPUT);
  pinMode(PWMA  ,OUTPUT);
  Serial.begin(57600);


// Calibramos con la función qtra.calibrate();, y dejamos parpadeando el led, mientras se produce la calibración.
  for ( int i=0; i<70; i++)
  {
    digitalWrite(LED, HIGH); delay(20);
    qtra.calibrate();
    digitalWrite(LED, LOW);  delay(20);
  }
  delay(3000);

}


unsigned int position = 0; 

//declaraos variables para utilizar PID
int proporcional = 0;         // Proporcional
int integral = 0;           //Intrgral
int derivativo = 0;          // Derivativo


int diferencial = 0;   // Diferencia aplicada a los motores
int last_prop;         // Última valor del proporcional (utilizado para calcular la derivada del error)
int Target = 2500; // Setpoint (Como utilizamos 6 sensores, la línea debe estar entre 0 y 5000, por lo que el ideal es que esté en 2500)

void loop()
{   
 
  position = qtra.readLine(sensorValues, true, true);
  proporcional = ((int)position) - 2500;


  if ( proporcional <= -Target )
  {
    Motorde(0);
    freno(true,false,255);
  }
  else if ( proporcional >= Target )
  {
    Motoriz(0);
    freno(false,true,255);
  }
  
  derivativo = proporcional - last_prop; 
  integral = error1+error2+error3+error4+error5+error6;
  last_prop = proporcional;
  

  error6=error5;
  error5=error4;  
  error4=error3;  
  error3=error2;
  error2=error1;
  error1=proporcional;

  sdir10=sdir9;
  sdir9=sdir8;
  sdir8=sdir7;
  sdir7=sdir6;
  sdir6=sdir5;
  sdir5=sdir4;
  sdir4=sdir3;
  sdir3=sdir2;
  sdir2=sdir1;
  sdir1=analogRead(S_DIR);

  sesq10=sesq9;
  sesq9=sesq8;
  sesq8=sesq7;
  sesq7=sesq6;
  sesq6=sesq5;
  sesq5=sesq4;
  sesq4=sesq3;
  sesq3=sesq2;
  sesq2=sesq1;
  sesq1=analogRead(S_ESQ);

 
  int diferencial = ( proporcional * KP ) + ( derivativo * KD )+ (integral*Ki) ;
  
  if ( diferencial > Velmax ) diferencial = Velmax; 
  else if ( diferencial < -Velmax ) diferencial = -Velmax;

  ( diferencial < 0 ) ? 
    Motor(Velmax+diferencial, Velmax) : Motor(Velmax, Velmax-diferencial);

  somadireita = sdir1+sdir2+sdir3+sdir4+sdir5+sdir6+sdir7+sdir8+sdir9+sdir10;
  somaesquerda = sesq1+sesq2+sesq3+sesq4+sesq5+sesq6+sesq7+sesq8+sesq9+sesq10;
  
  if(somadireita < 3000 && somaesquerda > 9500){
    Motor(0,0);
    delay(2000);
  }
  
//    Serial.print("Position: ");
//    Serial.print(position);
//    Serial.print(" Proporcional: ");
//    Serial.println(proporcional);
//    Serial.print(" S_ESQ: ");
//    Serial.print(analogRead(S_ESQ));
//    Serial.print(" S_DIR: ");
//    Serial.print(analogRead(S_DIR));
//    Serial.print(" Soma_ESQ: ");
//    Serial.print(somaesquerda);
//    Serial.print(" Soma_DIR: ");
//    Serial.println(somadireita);
}
