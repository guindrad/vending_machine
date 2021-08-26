/*
 * Copyright 2021 Guilherme Andrade
 *
 * See the included readme for instructions.
 * https://github.com/DeanIsMe/SevSeg
 * 
 */

#ifndef MAIN_H
#define MAIN_H

#include <arduino.h>
#include <timer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <esp_intr_alloc.h>

/* Componente: EIXO */
#define EIXO_chaveIDC       4   //Sensor chave de início de curso
#define EIXO_releMotorA     33  //Relé de movimento de avanço do motor
#define EIXO_releMotorB     2   //Relé de movimento de retorno do motor
#define EIXO_encoder_A      14  //Polo A do encoder de posicionamento do carro
#define EIXO_encoder_B      27  //Polo B do encoder de posicionamento do carro

/* Componente: CARRO */
#define CARRO_servomotor        13  //Servomotor que derruba produto. Recomendado: PWM GPIO pins 2,4,12-19,21-23,25-27,32-33 

/* Componente: SLOTS */
#define SLOTENTRADA_sensorIR1   15  //Sensor de identificacao de produto
#define SLOTENTRADA_sensorIR2   3   //Sensor de identificacao de produto
#define SLOTENTRADA_sensorIR3   18  //Sensor de identificacao de produto
#define SLOTENTRADA_sensorIR4   16  //Sensor de identificacao de produto
#define SLOTENTRADA_sensorIR5   34  //Sensor de identificacao de produto
#define SLOTENTRADA_sensorIR6   7   //Sensor de identificacao de produto
#define SLOTSAIDA_sensorIR1     32  //Sensor de identificacao de saída do produto
#define SLOTENTRADA_sensorTampa 5   //Sensor de abertura da tampa, para reposicao

/* Componente: INTERFACE */
#define INTERFACE_tecladoA      25  //Botão de + 25
#define INTERFACE_tecladoB      26  //Botão de - 26
#define INTERFACE_tecladoC      23  //Botão de CONFIRMA
#define INTERFACE_displaySDA    21  //Comunicacao I2C com display
#define INTERFACE_displaySLC    22  //Comunicacao I2C com display
#define INTERFACE_lcdColumns    20  //Dimensoes do LCD
#define INTERFACE_lcdRows       4   //Dimensoes do LCD

#define SYSTEM_longPress1       1000 //Tempo para considerar um click longo de teclado
#define SYSTEM_longPress2       2000 //Tempo para considerar um click longo de teclado
#define DEBOUNCE_TIME           25

/* Define o PPR (Pulsos Por Rotação), especificados pelo fabricante */
#define ENCODER02_PPR           600

/*
    Ativa o Multiplicador de Resolução por software.
    Utilizando rotinas de interrupção para todas as bordas do sinal do encoder, é possível aumentar
    a resolução física do encoder em até 4x.
    Para um PPR de 600 obtemos a resolução de 360º/600ppr = 0,6º/pulso
    Ativando o Multiplicador de Resolução, obtemos a resolução de 360º/2400ppr = 0,15º/pulso
*/

#define ENCODER02_RESOLUTION_MULT_EN //Comment to disable
#ifdef ENCODER02_RESOLUTION_MULT_EN
    #define ENCODER02_RESOLUTION_MULT_4X 4
#else
    #define ENCODER02_RESOLUTION_MULT_1X 1
#endif

/*
    Se habilitado, exibe as revoluções do encoder em pulsos.
    Se desabilitado, exibe as revoluções do encoder em graus.
*/
#define ENCODER_IN_PULSE //Comment to disable

/*
    EEPROM
    Mapa de dados
    0-6:    Estoque de produtos
      7:    wifi_login
      8:    wifi_password
      9:    device_name
     10:    server_pgto
     11:    server_update
     12:    last_alert
     12:    last_error
*/

#endif