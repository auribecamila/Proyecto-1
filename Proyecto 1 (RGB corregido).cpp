//UNIVERSIDAD DEL VALLE DE GUATEMALA 
//  PROYECTO 1 : ELECTRÓNICA DIGITAL 2
//  28/07/2026
// Camila Aristizabal 24367

//*************************************************************************************** */
// LIBRERÍAS 
//******************************************************************************************** */
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>

// *******************************************************************************************
// DEFINICIÓN DE PINES
// *******************************************************************************************

// Sensor LM35
#define LM35 34

// Pushbutton
#define B1 18

// LED RGB
#define LED_R 25
#define LED_G 26
#define LED_B 27

// Servo
#define SERVO 14

// *******************************************************************************************
// CONFIGURACIÓN PWM
// *******************************************************************************************

// Canales LED RGB
#define CANAL_R 0
#define CANAL_G 1
#define CANAL_B 2

// Canal Servo
#define CANAL_SERVO 3

// PWM LED RGB
#define FRECUENCIA_RGB 5000
#define RESOLUCION_RGB 8

// PWM Servo
#define FRECUENCIA_SERVO 50
#define RESOLUCION_SERVO 16

// *******************************************************************************************
// PROTOTIPOS DE FUNCIONES
// *******************************************************************************************

void configInterrupts(void);
void configPWM(void);

void leerTemperatura(void);

void actualizarRGB(void);

void moverServo(int angulo);
void actualizarCompuerta(void);

void IRAM_ATTR B1_ISR();

// *******************************************************************************************
// VARIABLES GLOBALES
// *******************************************************************************************

volatile bool B1_presionado = false;

// Anti-rebote

unsigned long tiempoB1 = 0;

const int antiRebote = 200;

// Variables LM35

int lecturaADC = 0;

float voltaje = 0;

float temperatura = 0;

// *******************************************************************************************
// CONFIGURACIÓN
// *******************************************************************************************

void setup()
{
  Serial.begin(115200);

  pinMode(B1, INPUT_PULLDOWN);

  configPWM();

  configInterrupts();
}

// *******************************************************************************************
// LOOP
// *******************************************************************************************

void loop()
{
  if(B1_presionado)
  {
    if(millis() - tiempoB1 > antiRebote)
    {
      leerTemperatura();

      tiempoB1 = millis();
    }

    B1_presionado = false;
  }
}
// *******************************************************************************************
// CONFIGURACIÓN DE INTERRUPCIONES
// *******************************************************************************************

void configInterrupts(void)
{
  attachInterrupt(B1, B1_ISR, RISING);
}

// *******************************************************************************************
// ISR DEL PUSHBUTTON
// *******************************************************************************************

void IRAM_ATTR B1_ISR()
{
  B1_presionado = true;
}

// *******************************************************************************************
// LECTURA DEL SENSOR LM35
// *******************************************************************************************

void leerTemperatura(void)
{
  // Leer ADC
  lecturaADC = analogRead(LM35);

  // Convertir a voltaje
  voltaje = lecturaADC * 3.3 / 4095.0;

  // Convertir a temperatura   
  temperatura = voltaje * 100.0;

  // Actualizar actuadores
  actualizarRGB();
  actualizarCompuerta();

  // Mostrar información
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.println("----------------------------");
}

// *******************************************************************************************
// ACTUALIZAR COLOR DEL LED RGB
// *******************************************************************************************

void actualizarRGB(void)
{
  // Apagar todos los colores

  ledcWrite(CANAL_R, 255);
  ledcWrite(CANAL_G, 255);
  ledcWrite(CANAL_B, 255);

  // Riesgo por frío

  if (temperatura < 23)
  {
    ledcWrite(CANAL_B, 0);

    Serial.println("Estado: Riesgo por frio");
    Serial.println("LED: Azul");
  }

  // Rango seguro

  else if (temperatura < 25)
  {
    ledcWrite(CANAL_G, 0);

    Serial.println("Estado: Rango seguro");
    Serial.println("LED: Verde");
  }

  // Cerca del límite

  else if (temperatura < 27)
  {
    ledcWrite(CANAL_R, 0);
    ledcWrite(CANAL_G, 0);

    Serial.println("Estado: Cerca del limite");
    Serial.println("LED: Amarillo");
  }

  // Riesgo por calor

  else
  {
    ledcWrite(CANAL_R, 0);

    Serial.println("Estado: Riesgo por calor");
    Serial.println("LED: Rojo");
  }
}
// *******************************************************************************************
// CONFIGURACIÓN PWM
// *******************************************************************************************

void configPWM(void)
{
  // -------------------------------
  // Configuración LED RGB
  // -------------------------------

  ledcSetup(CANAL_R, FRECUENCIA_RGB, RESOLUCION_RGB);
  ledcSetup(CANAL_G, FRECUENCIA_RGB, RESOLUCION_RGB);
  ledcSetup(CANAL_B, FRECUENCIA_RGB, RESOLUCION_RGB);

  ledcAttachPin(LED_R, CANAL_R);
  ledcAttachPin(LED_G, CANAL_G);
  ledcAttachPin(LED_B, CANAL_B);

  // Apagar RGB al iniciar

  ledcWrite(CANAL_R, 255);
  ledcWrite(CANAL_G, 255);
  ledcWrite(CANAL_B, 255);

  // -------------------------------
  // Configuración Servo
  // -------------------------------

  ledcSetup(CANAL_SERVO, FRECUENCIA_SERVO, RESOLUCION_SERVO);

  ledcAttachPin(SERVO, CANAL_SERVO);

  // Posición inicial: cerrada

  moverServo(0);
}

// *******************************************************************************************
// MOVER SERVOMOTOR
// *******************************************************************************************

void moverServo(int angulo)
{
  uint32_t duty;

  if (angulo == 0)
  {
    duty = 1638;
  }

  else if (angulo == 45)
  {
    duty = 2458;
  }

  else if (angulo == 90)
  {
    duty = 3277;
  }

  else
  {
    duty = 1638;
  }

  ledcWrite(CANAL_SERVO, duty);
}

// *******************************************************************************************
// ACTUALIZAR POSICIÓN DE LA COMPUERTA
// *******************************************************************************************

void actualizarCompuerta(void)
{
  // Riesgo por frío
  if (temperatura < 23)
  {
    moverServo(0);

    Serial.println("Compuerta: Cerrada (0°)");
  }

  // Rango seguro
  else if (temperatura < 25)
  {
    moverServo(45);

    Serial.println("Compuerta: Medio (45°)");
  }

  // Cerca del límite
  else if (temperatura < 27)
  {
    moverServo(45);

    Serial.println("Compuerta: Medio (45°)");
  }

  // Riesgo por calor
  else
  {
    moverServo(90);

    Serial.println("Compuerta: Abierta (90°)");
  }
}