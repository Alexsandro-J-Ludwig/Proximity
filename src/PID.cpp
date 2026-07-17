#ifndef PID_H
#define PID_H
#include "Struct.h"
#include <Arduino.h>

extern Motor motores_direito;
extern Motor motores_esquerdo;

float Kp = 0.5; // Ganho proporcional
float Ki = 0.05; // Ganho integral
float Kd = 0.2; // Ganho derivativo

int VELOCIDADE_BASE = 70;

unsigned long msTP = 0;

int anguloAnterior = 0;

void pid(Motor &motor_direito, Motor &motor_esquerdo, unsigned long msT, int angulo)
{   
    unsigned long tempoAnterior = msT - msTP;
    if (tempoAnterior == 0)
        tempoAnterior = 1;
    msTP = msT;
    

    float erroD = angulo - 90.0;
    float derivada = (erroD - anguloAnterior) / (tempoAnterior / 1000.0);

    static float integralErro = 0;

    if (abs(erroD) > 0.1)
    {
        integralErro = constrain(integralErro + erroD * (tempoAnterior / 1000.0), -50, 50);

        float saidaD = Kp * erroD + Ki * integralErro + Kd * derivada;

        int pwmDireito = constrain(VELOCIDADE_BASE - saidaD, 0, 255);
        int pwmEsquerdo = constrain(VELOCIDADE_BASE + saidaD, 0, 255);

        analogWrite(motor_direito.pwm.p.enA, pwmDireito);
        analogWrite(motor_esquerdo.pwm.p.enB, pwmEsquerdo);
    }   
    else
    {
        integralErro = 0;
        analogWrite(motor_direito.pwm.p.enA, VELOCIDADE_BASE);
        analogWrite(motor_esquerdo.pwm.p.enB, VELOCIDADE_BASE);
    }

    anguloAnterior = erroD;
}

#endif // PID_H