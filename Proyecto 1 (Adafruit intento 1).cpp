//UNIVERSIDAD DEL VALLE DE GUATEMALA 
//  PROYECTO 1 : ELECTRÓNICA DIGITAL 2
//  28/07/2026
// Camila Aristizabal 24367

//*******************************************************************************************
// LIBRERÍAS
//*******************************************************************************************

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
//*******************************************************************************************
// DEFINICIÓN DE PINES
//*******************************************************************************************

//----------------------------
// Sensor LM35
//----------------------------

#define LM35 34

//----------------------------
// Pushbutton
//----------------------------

#define B1 18

//----------------------------
// LED RGB
//----------------------------

#define LED_R 25
#define LED_G 26
#define LED_B 27

//----------------------------
// Servomotor
//----------------------------

#define SERVO 14

//----------------------------
// Display 7 segmentos
//----------------------------

// Segmentos

#define SEG_A 17      //TX2
#define SEG_B 23
#define SEG_C 33
#define SEG_D 15
#define SEG_E 22
#define SEG_F 5
#define SEG_G 13
#define SEG_DP 32

// Displays

#define DISP1 2
#define DISP2 4
#define DISP3 16      //RX2

//*******************************************************************************************
// CONFIGURACIÓN PWM
//*******************************************************************************************

#define CANAL_R 0
#define CANAL_G 1
#define CANAL_B 2
#define CANAL_SERVO 3

#define FRECUENCIA_RGB 5000
#define RESOLUCION_RGB 8

#define FRECUENCIA_SERVO 50
#define RESOLUCION_SERVO 16

#define IO_LOOP_DELAY 5000

//*******************************************************************************************
// PROTOTIPOS
//*******************************************************************************************
//*******************************************************************************************
// PROTOTIPOS
//*******************************************************************************************

// Configuración

void configPWM(void);
void configInterrupts(void);
void configDisplay(void);

// Sensor

void leerTemperatura(void);

// Actuadores

void actualizarRGB(void);
void actualizarCompuerta(void);
void moverServo(int angulo);

// Display

void separarTemperatura(void);
void mostrarTemperatura(void);
void mostrarDigito(uint8_t numero, bool punto);
void apagarDisplays(void);

// Interrupción

void IRAM_ATTR B1_ISR();
void handleMessage(AdafruitIO_Data *data);

//*******************************************************************************************
// VARIABLES GLOBALES
//*******************************************************************************************

// Pushbutton

volatile bool B1_presionado=false;

unsigned long tiempoB1=0;

const uint16_t antiRebote=200;

// Sensor

int lecturaADC=0;

float voltaje=0;

float temperatura=0;

// Display

int temperatura10=0;

uint8_t decenas=0;
uint8_t unidades=0;
uint8_t decimal=0;

//Adafruit

unsigned long lastUpdate = 0;

// set up the 'counter' feed
AdafruitIO_Feed *canalTemperatura = io.feed("temperatura");

//*******************************************************************************************
// TABLA DE SEGMENTOS
//*******************************************************************************************

const bool numero[10][7]=
{
    {1,1,1,1,1,1,0}, //0
    {0,1,1,0,0,0,0}, //1
    {1,1,0,1,1,0,1}, //2
    {1,1,1,1,0,0,1}, //3
    {0,1,1,0,0,1,1}, //4
    {1,0,1,1,0,1,1}, //5
    {1,0,1,1,1,1,1}, //6
    {1,1,1,0,0,0,0}, //7
    {1,1,1,1,1,1,1}, //8
    {1,1,1,1,0,1,1}  //9
};

//*******************************************************************************************
// SETUP
//*******************************************************************************************

void setup()
{
    Serial.begin(115200);

    // connect to io.adafruit.com
    io.connect();

    pinMode(B1,INPUT_PULLDOWN);

    configPWM();

    configDisplay();
    

    configInterrupts();

    // Primera lectura

    leerTemperatura();
    // received from adafruit io.
    canalTemperatura->onMessage(handleMessage);
    // wait for a connection
    while (io.status() < AIO_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    // we are connected
    Serial.println();
    Serial.println(io.statusText());
    canalTemperatura->get();
}

//*******************************************************************************************
// LOOP
//*******************************************************************************************

void loop()
{
   
    // Mantener conexión con Adafruit IO
    io.run();

    if (millis() > (lastUpdate + IO_LOOP_DELAY))
    {
        Serial.print("sending -> ");
        Serial.println(temperatura);

        canalTemperatura->save(temperatura);

        lastUpdate = millis();
    }

    // El display siempre permanece encendido
    mostrarTemperatura();

    // Nueva inspección mediante botón
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
//*******************************************************************************************
// CONFIGURACIÓN DE INTERRUPCIONES
//*******************************************************************************************

void configInterrupts(void)
{
    attachInterrupt(B1, B1_ISR, RISING);
}

//*******************************************************************************************
// INTERRUPCIÓN BOTÓN
//*******************************************************************************************

void IRAM_ATTR B1_ISR()
{
    B1_presionado = true;
}

//*******************************************************************************************
// LECTURA DEL SENSOR LM35
//*******************************************************************************************

void leerTemperatura(void)
{
    // Leer ADC
    lecturaADC = analogRead(LM35);

    // Convertir a voltaje
    voltaje = (lecturaADC * 3.3) / 4095.0;

    // Convertir a temperatura 
    temperatura = voltaje * 100.0;

    // Preparar dato para el display
    separarTemperatura();

    // Actualizar actuadores
    actualizarRGB();

    actualizarCompuerta();

    // Monitor Serial
    Serial.println("-----------------------------");
    Serial.print("Temperatura: ");
    Serial.print(temperatura,1);
    Serial.println(" °C");
}

//*******************************************************************************************
// LED RGB
//*******************************************************************************************

void actualizarRGB(void)
{
    // Apagar todos los colores
    // (LED Ánodo común)

    ledcWrite(CANAL_R,255);
    ledcWrite(CANAL_G,255);
    ledcWrite(CANAL_B,255);

    // Riesgo por frío

    if(temperatura < 23)
    {
        ledcWrite(CANAL_B,0);

        Serial.println("Estado: Riesgo por frio");
        Serial.println("LED: Azul");
    }

    // Rango seguro

    else if(temperatura < 25)
    {
        ledcWrite(CANAL_G,0);

        Serial.println("Estado: Rango seguro");
        Serial.println("LED: Verde");
    }

    // Cerca del límite

    else if(temperatura < 27)
    {
        ledcWrite(CANAL_R,0);
        ledcWrite(CANAL_G,0);

        Serial.println("Estado: Cerca del limite");
        Serial.println("LED: Amarillo");
    }

    // Riesgo por calor

    else
    {
        ledcWrite(CANAL_R,0);

        Serial.println("Estado: Riesgo por calor");
        Serial.println("LED: Rojo");
    }
}

//*******************************************************************************************
// CONFIGURACIÓN PWM
//*******************************************************************************************

void configPWM(void)
{
    // ---------------- RGB ----------------

    ledcSetup(CANAL_R,FRECUENCIA_RGB,RESOLUCION_RGB);
    ledcSetup(CANAL_G,FRECUENCIA_RGB,RESOLUCION_RGB);
    ledcSetup(CANAL_B,FRECUENCIA_RGB,RESOLUCION_RGB);

    ledcAttachPin(LED_R,CANAL_R);
    ledcAttachPin(LED_G,CANAL_G);
    ledcAttachPin(LED_B,CANAL_B);

    // Apagar RGB

    ledcWrite(CANAL_R,255);
    ledcWrite(CANAL_G,255);
    ledcWrite(CANAL_B,255);

    // ---------------- Servo ----------------

    ledcSetup(CANAL_SERVO,FRECUENCIA_SERVO,RESOLUCION_SERVO);

    ledcAttachPin(SERVO,CANAL_SERVO);

    moverServo(0);
}

//*******************************************************************************************
// MOVER SERVOMOTOR
//*******************************************************************************************

void moverServo(int angulo)
{
    uint32_t duty;

    switch(angulo)
    {
        case 0:
            duty = 1638;
        break;

        case 45:
            duty = 2458;
        break;

        case 90:
            duty = 3277;
        break;

        default:
            duty = 1638;
        break;
    }

    ledcWrite(CANAL_SERVO,duty);
}

//*******************************************************************************************
// ACTUALIZAR COMPUERTA
//*******************************************************************************************

void actualizarCompuerta(void)
{
    if(temperatura < 23)
    {
        moverServo(0);

        Serial.println("Compuerta: Cerrada");
    }

    else if(temperatura < 25)
    {
        moverServo(45);

        Serial.println("Compuerta: Medio");
    }

    else if(temperatura < 27)
    {
        moverServo(45);

        Serial.println("Compuerta: Medio");
    }

    else
    {
        moverServo(90);

        Serial.println("Compuerta: Abierta");
    }
}
//*******************************************************************************************
// CONFIGURACIÓN DISPLAY
//*******************************************************************************************

void configDisplay(void)
{
    // Segmentos

    pinMode(SEG_A, OUTPUT);
    pinMode(SEG_B, OUTPUT);
    pinMode(SEG_C, OUTPUT);
    pinMode(SEG_D, OUTPUT);
    pinMode(SEG_E, OUTPUT);
    pinMode(SEG_F, OUTPUT);
    pinMode(SEG_G, OUTPUT);
    pinMode(SEG_DP, OUTPUT);

    // Displays

    pinMode(DISP1, OUTPUT);
    pinMode(DISP2, OUTPUT);
    pinMode(DISP3, OUTPUT);

    apagarDisplays();

    // Apagar todos los segmentos

    digitalWrite(SEG_A, LOW);
    digitalWrite(SEG_B, LOW);
    digitalWrite(SEG_C, LOW);
    digitalWrite(SEG_D, LOW);
    digitalWrite(SEG_E, LOW);
    digitalWrite(SEG_F, LOW);
    digitalWrite(SEG_G, LOW);
    digitalWrite(SEG_DP, LOW);
}

//*******************************************************************************************
// APAGAR TODOS LOS DISPLAYS
//*******************************************************************************************

void apagarDisplays(void)
{
    digitalWrite(DISP1, LOW);
    digitalWrite(DISP2, LOW);
    digitalWrite(DISP3, LOW);
}

//*******************************************************************************************
// SEPARAR TEMPERATURA
//*******************************************************************************************

void separarTemperatura(void)
{
    

    temperatura10 = (int)(temperatura * 10 + 0.5);

    decenas = temperatura10 / 100;

    unidades = (temperatura10 / 10) % 10;

    decimal = temperatura10 % 10;
}

//*******************************************************************************************
// MOSTRAR UN DÍGITO
//*******************************************************************************************

void mostrarDigito(uint8_t valor, bool punto)
{
    digitalWrite(SEG_A, numero[valor][0]);
    digitalWrite(SEG_B, numero[valor][1]);
    digitalWrite(SEG_C, numero[valor][2]);
    digitalWrite(SEG_D, numero[valor][3]);
    digitalWrite(SEG_E, numero[valor][4]);
    digitalWrite(SEG_F, numero[valor][5]);
    digitalWrite(SEG_G, numero[valor][6]);

    digitalWrite(SEG_DP, punto);
}
//*******************************************************************************************
// MOSTRAR TEMPERATURA EN DISPLAY
//*******************************************************************************************

void mostrarTemperatura(void)
{
    //==========================
    // DISPLAY 1
    //==========================

    apagarDisplays();

    mostrarDigito(decenas, false);

    digitalWrite(DISP1, HIGH);

    delay(2);

    //==========================
    // DISPLAY 2
    //==========================

    apagarDisplays();

    mostrarDigito(unidades, true);

    digitalWrite(DISP2, HIGH);

    delay(2);

    //==========================
    // DISPLAY 3
    //==========================

    apagarDisplays();

    mostrarDigito(decimal, false);

    digitalWrite(DISP3, HIGH);

    delay(2);

    apagarDisplays();
}
// this function is called whenever a 'counter' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());
}