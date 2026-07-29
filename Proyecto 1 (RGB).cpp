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

// Canales PWM
#define CANAL_R 0
#define CANAL_G 1
#define CANAL_B 2

// Configuración PWM
#define FRECUENCIA_PWM 5000
#define RESOLUCION_PWM 8

// *******************************************************************************************
// PROTOTIPOS DE FUNCIONES
// *******************************************************************************************

void configInterrupts(void);
void configPWM(void);

void leerTemperatura(void);
void actualizarRGB(void);

void IRAM_ATTR B1_ISR();

// *******************************************************************************************
// VARIABLES GLOBALES
// *******************************************************************************************

volatile bool B1_presionado = false;

// Anti-rebote
unsigned long tiempoB1 = 0;
const int antiRebote = 200;

// Variables del LM35

int lecturaADC = 0;

float voltaje = 0;

float temperatura = 0;

// *******************************************************************************************
// CONFIGURACIÓN
// *******************************************************************************************

void setup()
{
  // Inicializar monitor serial

  Serial.begin(115200);

  // Configurar botón

  pinMode(B1, INPUT_PULLDOWN);

  // Configurar PWM del LED RGB

  configPWM();

  // Configurar interrupción

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
// ISR
// *******************************************************************************************

void IRAM_ATTR B1_ISR()
{
  B1_presionado = true;
}

// *******************************************************************************************
// FUNCIÓN PARA LEER EL SENSOR LM35
// *******************************************************************************************

void leerTemperatura(void)
{
  // Leer el ADC

  lecturaADC = analogRead(LM35);

  // Convertir a voltaje

  voltaje = lecturaADC * 3.3 / 4095.0;

  // Convertir a temperatura

  temperatura = voltaje * 100.0;

  // Actualizar el color del LED

  actualizarRGB();

  // Mostrar resultados

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");
}
// *******************************************************************************************
// CONFIGURACIÓN PWM DEL LED RGB
// *******************************************************************************************

void configPWM(void)
{
  // Configurar los canales PWM

  ledcSetup(CANAL_R, FRECUENCIA_PWM, RESOLUCION_PWM);
  ledcSetup(CANAL_G, FRECUENCIA_PWM, RESOLUCION_PWM);
  ledcSetup(CANAL_B, FRECUENCIA_PWM, RESOLUCION_PWM);

  // Asociar cada canal a su pin

  ledcAttachPin(LED_R, CANAL_R);
  ledcAttachPin(LED_G, CANAL_G);
  ledcAttachPin(LED_B, CANAL_B);

  // Iniciar el LED apagado

  ledcWrite(CANAL_R, 0);
  ledcWrite(CANAL_G, 0);
  ledcWrite(CANAL_B, 0);
}

// *******************************************************************************************
// ACTUALIZAR COLOR DEL LED RGB
// *******************************************************************************************

void actualizarRGB(void)
{
  // Apagar todos los colores

  ledcWrite(CANAL_R, 0);
  ledcWrite(CANAL_G, 0);
  ledcWrite(CANAL_B, 0);

  // Riesgo por frío
  // Temperatura menor a 23 °C
  // Color Azul

  if (temperatura < 23)
  {
    ledcWrite(CANAL_B, 255);

    Serial.println("Estado: Riesgo por frio");
    Serial.println("LED: Azul");
  }

  // Rango seguro
  // Entre 23 y 25 °C
  // Color Verde

  else if (temperatura < 25)
  {
    ledcWrite(CANAL_G, 255);

    Serial.println("Estado: Rango seguro");
    Serial.println("LED: Verde");
  }

  // Cerca del límite
  // Entre 25 y 27 °C
  // Color Amarillo

  else if (temperatura < 27)
  {
    ledcWrite(CANAL_R, 255);
    ledcWrite(CANAL_G, 255);

    Serial.println("Estado: Cerca del limite");
    Serial.println("LED: Amarillo");
  }

  // Riesgo por calor
  // Mayor o igual a 27 °C
  // Color Rojo

  else
  {
    ledcWrite(CANAL_R, 255);

    Serial.println("Estado: Riesgo por calor");
    Serial.println("LED: Rojo");
  }

  Serial.println("--------------------------------");
}