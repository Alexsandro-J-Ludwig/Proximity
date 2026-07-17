#ifndef PID_H
#define PID_H

#include "Struct.h"
#include <Arduino.h>

// Declaração das constantes para que outros arquivos possam ler/ajudar a tunar
extern float Kp;
extern float Ki;
extern float Kd;
extern int VELOCIDADE_BASE;

// Função recebendo os motores por REFERÊNCIA (&)
void pid(Motor &motor_direito, Motor &motor_esquerdo, unsigned long msT, int angulo);

#endif // PID_H