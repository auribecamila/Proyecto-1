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

// Sensor LM35 (va a ser una entrada analógica)
#define LM35 35

// Botón
#define B1 18

// *******************************************************************************************
// PROTOTIPOS DE FUNCIONES
// *******************************************************************************************
void configInterrupts(void);
void leerTemperatura(void);

void IRAM_ATTR B1_ISR();

// *******************************************************************************************
// VARIABLES GLOBALES
// *******************************************************************************************

// Variable para detectar la pulsación
volatile bool B1_presionado = false;

// Variables para anti-rebote
unsigned long tiempoB1 = 0;

const int antiRebote = 200;

// Variables del sensor

int lecturaADC = 0;

float voltaje = 0;

float temperatura = 0;

// *******************************************************************************************
// CONFIGURACIÓN
// *******************************************************************************************
void setup()
{
  // Inicializar Monitor Serial

  Serial.begin(115200);

  // Configurar botón

  pinMode(B1, INPUT_PULLDOWN);

  // Configurar interrupción

  configInterrupts();
}
 
// *******************************************************************************************
// LOOP
// *******************************************************************************************

void loop()
{
  // ****************************************************
  // DETECTAR PULSACIÓN DEL BOTÓN
  // ****************************************************

  if (B1_presionado)
  {
    if (millis() - tiempoB1 > antiRebote)
    {
      leerTemperatura();

      tiempoB1 = millis();
    }
    //al pasar el ISR y el antirebote, presionar el boton activará una lectura en el sensor, guardando ese dato
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
// ISR DEL BOTÓN
// *******************************************************************************************

void IRAM_ATTR B1_ISR()
{
  B1_presionado = true;
}

// *******************************************************************************************
// FUNCIÓN PARA LEER EL LM35
// *******************************************************************************************

void leerTemperatura(void)
{
  // Leer el ADC

  lecturaADC = analogRead(LM35);

  // Convertir la lectura a voltaje

  voltaje = lecturaADC;

  // Convertir el voltaje a temperatura

  temperatura = voltaje /10;

  // Mostrar resultados
  // Serial print para verificar que el sensor sí este arrojando lecturas al presionarse el boton

  Serial.print("ADC: ");
  Serial.print(lecturaADC);

  Serial.print("   Voltaje: ");
  Serial.print(voltaje);

  Serial.print(" V   Temperatura: ");
  Serial.print(temperatura);

  Serial.println(" °C");
}


