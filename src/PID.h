#ifndef PID_H
#define PID_H

#include "Struct.h"
#include <Arduino.h>

extern float Kp;
extern float Ki;
extern float Kd;
extern int VELOCIDADE_BASE;

bool pid(Motor &motor_direito, Motor &motor_esquerdo, unsigned long msT, int angulo, int maiorDistancia);

#endif // PID_H