//UNIVERSIDAD DEL VALLE DE GUATEMALA 
//  PROYECTO 1 : ELECTRÓNICA DIGITAL 2
//  28/07/2026
// Camila Aristizabal 24367

//***************************************************************************************
// LIBRERÍAS
//***************************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>

//***************************************************************************************
// DEFINICIÓN DE PINES
//***************************************************************************************

// LM35
#define LM35 34

// Pushbutton
#define B1 18

// LED RGB
#define LED_R 25
#define LED_G 26
#define LED_B 27

// Servo

#define SERVO 14

//***************************************************************************************
// DISPLAY 7 SEGMENTOS
//***************************************************************************************

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

//***************************************************************************************
// PWM
//***************************************************************************************

#define CANAL_R 0
#define CANAL_G 1
#define CANAL_B 2

#define CANAL_SERVO 3

#define FRECUENCIA_RGB 5000
#define RESOLUCION_RGB 8

#define FRECUENCIA_SERVO 50
#define RESOLUCION_SERVO 16

//***************************************************************************************
// PROTOTIPOS
//***************************************************************************************

void configInterrupts(void);

void configPWM(void);

void configDisplay(void);

void leerTemperatura(void);

void actualizarRGB(void);

void actualizarCompuerta(void);

void moverServo(int angulo);

void separarTemperatura(void);

void actualizarDisplay(void);

void mostrarNumero(uint8_t numero);

void seleccionarDisplay(uint8_t display);

void apagarDisplays(void);

void IRAM_ATTR B1_ISR();

//***************************************************************************************
// VARIABLES GLOBALES
//***************************************************************************************

volatile bool B1_presionado=false;

unsigned long tiempoB1=0;

const int antiRebote=200;

// LM35

int lecturaADC=0;

float voltaje=0;

float temperatura=0;

//***************************************************************************************
// DISPLAY
//***************************************************************************************

int temperaturaDisplay=0;

uint8_t decenas=0;

uint8_t unidades=0;

uint8_t decimal=0;

uint8_t displayActual=0;

//***************************************************************************************
// TABLA DE SEGMENTOS
//***************************************************************************************

const bool digitos[10][7]=
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

//***************************************************************************************
// SETUP
//***************************************************************************************

void setup()
{
    Serial.begin(115200);

    pinMode(B1,INPUT_PULLDOWN);

    configPWM();

    configDisplay();

    configInterrupts();
}

//***************************************************************************************
// LOOP
//***************************************************************************************

void loop()
{
    actualizarDisplay();

    if(B1_presionado)
    {
        if(millis()-tiempoB1>antiRebote)
        {
            leerTemperatura();

            tiempoB1=millis();
        }

        B1_presionado=false;
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
// ISR BOTÓN
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
    voltaje = lecturaADC * 3.3 / 4095.0;

    // Convertir a temperatura
    temperatura = voltaje * 100.0;

    // Preparar dato para el display
    separarTemperatura();

    // Actualizar actuadores
    actualizarRGB();

    actualizarCompuerta();

    // Monitor Serial

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.println("----------------------------");
}

//*******************************************************************************************
// RGB
//*******************************************************************************************

void actualizarRGB(void)
{
    // Apagar RGB (ánodo común)

    ledcWrite(CANAL_R,255);
    ledcWrite(CANAL_G,255);
    ledcWrite(CANAL_B,255);

    // Riesgo por frío

    if(temperatura<23)
    {
        ledcWrite(CANAL_B,0);

        Serial.println("Estado: Riesgo por frio");

        Serial.println("LED Azul");
    }

    // Seguro

    else if(temperatura<25)
    {
        ledcWrite(CANAL_G,0);

        Serial.println("Estado: Seguro");

        Serial.println("LED Verde");
    }

    // Cerca del límite

    else if(temperatura<27)
    {
        ledcWrite(CANAL_R,0);
        ledcWrite(CANAL_G,0);

        Serial.println("Estado: Cerca del limite");

        Serial.println("LED Amarillo");
    }

    // Riesgo por calor

    else
    {
        ledcWrite(CANAL_R,0);

        Serial.println("Estado: Riesgo por calor");

        Serial.println("LED Rojo");
    }
}

//*******************************************************************************************
// SERVO
//*******************************************************************************************

void moverServo(int angulo)
{
    uint32_t duty;

    if(angulo==0)
    {
        duty=1638;
    }

    else if(angulo==45)
    {
        duty=2458;
    }

    else
    {
        duty=3277;
    }

    ledcWrite(CANAL_SERVO,duty);
}

//*******************************************************************************************
// COMPUERTA
//*******************************************************************************************

void actualizarCompuerta(void)
{
    if(temperatura<23)
    {
        moverServo(0);

        Serial.println("Compuerta Cerrada");
    }

    else if(temperatura<25)
    {
        moverServo(45);

        Serial.println("Compuerta Media");
    }

    else if(temperatura<27)
    {
        moverServo(45);

        Serial.println("Compuerta Media");
    }

    else
    {
        moverServo(90);

        Serial.println("Compuerta Abierta");
    }
}
//*******************************************************************************************
// CONFIGURACIÓN DISPLAY
//*******************************************************************************************

void configDisplay(void)
{
    // Segmentos

    pinMode(SEG_A,OUTPUT);
    pinMode(SEG_B,OUTPUT);
    pinMode(SEG_C,OUTPUT);
    pinMode(SEG_D,OUTPUT);
    pinMode(SEG_E,OUTPUT);
    pinMode(SEG_F,OUTPUT);
    pinMode(SEG_G,OUTPUT);
    pinMode(SEG_DP,OUTPUT);

    // Displays

    pinMode(DISP1,OUTPUT);
    pinMode(DISP2,OUTPUT);
    pinMode(DISP3,OUTPUT);

    apagarDisplays();

    digitalWrite(SEG_A,LOW);
    digitalWrite(SEG_B,LOW);
    digitalWrite(SEG_C,LOW);
    digitalWrite(SEG_D,LOW);
    digitalWrite(SEG_E,LOW);
    digitalWrite(SEG_F,LOW);
    digitalWrite(SEG_G,LOW);
    digitalWrite(SEG_DP,LOW);
}

//*******************************************************************************************
// APAGAR TODOS LOS DISPLAYS
//*******************************************************************************************

void apagarDisplays(void)
{
    digitalWrite(DISP1,LOW);
    digitalWrite(DISP2,LOW);
    digitalWrite(DISP3,LOW);
}

//*******************************************************************************************
// SELECCIONAR DISPLAY
//*******************************************************************************************

void seleccionarDisplay(uint8_t display)
{
    apagarDisplays();

    switch(display)
    {
        case 0:

            digitalWrite(DISP1,HIGH);

        break;

        case 1:

            digitalWrite(DISP2,HIGH);

        break;

        case 2:

            digitalWrite(DISP3,HIGH);

        break;
    }
}

//*******************************************************************************************
// MOSTRAR UN NÚMERO
//*******************************************************************************************

void mostrarNumero(uint8_t numero)
{
    digitalWrite(SEG_A,digitos[numero][0]);
    digitalWrite(SEG_B,digitos[numero][1]);
    digitalWrite(SEG_C,digitos[numero][2]);
    digitalWrite(SEG_D,digitos[numero][3]);
    digitalWrite(SEG_E,digitos[numero][4]);
    digitalWrite(SEG_F,digitos[numero][5]);
    digitalWrite(SEG_G,digitos[numero][6]);
}
//*******************************************************************************************
// SEPARAR TEMPERATURA
//*******************************************************************************************

void separarTemperatura(void)
{
    // Ejemplo:
    // 24.7°C -> 247

    temperaturaDisplay = (int)(temperatura * 10.0);

    decenas = temperaturaDisplay / 100;

    unidades = (temperaturaDisplay / 10) % 10;

    decimal = temperaturaDisplay % 10;
}

//*******************************************************************************************
// ACTUALIZAR DISPLAY
//*******************************************************************************************

void actualizarDisplay(void)
{
    static unsigned long tiempoDisplay = 0;

    if(micros() - tiempoDisplay < 2000)
        return;

    tiempoDisplay = micros();

    apagarDisplays();

    switch(displayActual)
    {
        //**************************************
        // DISPLAY 1 (DECENAS)
        //**************************************

        case 0:

            mostrarNumero(decenas);

            digitalWrite(SEG_DP,LOW);

            seleccionarDisplay(0);

        break;

        //**************************************
        // DISPLAY 2 (UNIDADES)
        //**************************************

        case 1:

            mostrarNumero(unidades);

            // Encender punto decimal

            digitalWrite(SEG_DP,HIGH);

            seleccionarDisplay(1);

        break;

        //**************************************
        // DISPLAY 3 (DÉCIMAS)
        //**************************************

        case 2:

            mostrarNumero(decimal);

            digitalWrite(SEG_DP,LOW);

            seleccionarDisplay(2);

        break;
    }

    displayActual++;

    if(displayActual>2)
    {
        displayActual=0;
    }
}