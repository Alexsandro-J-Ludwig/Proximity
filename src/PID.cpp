#ifndef PID_H
#define PID_H
#include "Struct.h"
#include <Arduino.h>
#include "PID.h"
#include "init.h"

float Kp = 0.25;
float Ki = 0.001;
float Kd = 0.0;
int VELOCIDADE_BASE = 60;

static unsigned long tempoUltimoCalculo = 0;
static float erroAnterior = 0.0;
static float integralErro = 0.0;

DirecaoDefinida direcao = DirecaoDefinida::Centro;

bool pid(Motor &motor_direito, Motor &motor_esquerdo, unsigned long tempoAtual, int anguloAlvo, int maiorDistancia)
{
    float dt = (tempoAtual - tempoUltimoCalculo) / 1000.0;

    if (dt <= 0.0)
        return false;

    tempoUltimoCalculo = tempoAtual;

    float erroD = anguloAlvo - 90;

    if (abs(erroD) >= 45)
    {
        direcao = DirecaoDefinida::CurvaFechada;
    }
    else if (abs(erroD) >= 20)
    {
        direcao = DirecaoDefinida::curva;
    }
    else
    {
        direcao = DirecaoDefinida::Centro;
    }

    float derivada = (erroD - erroAnterior) / dt;

    if (abs(erroD) > 0.1)
    {
        integralErro = constrain(integralErro + (erroD * dt), -50.0, 50.0);

        float kp_ativo = Kp;
        float kd_ativo = Kd;

        // switch (direcao)
        // {
        // case DirecaoDefinida::CurvaFechada:
        //     kp_ativo = 0.25;
        //     kd_ativo = 0.15;

        //     break;
        // case DirecaoDefinida::curva:
        //     kp_ativo = 0.20;
        //     kd_ativo = 0.15;
        //     break;
        // case DirecaoDefinida::Centro:
        //     kp_ativo = 0.15;
        //     kd_ativo = 0.15;
        //     break;
        // default:
        //     break;
        // }

        float saidaPID = (kp_ativo * erroD) + (Ki * integralErro) + (kd_ativo * derivada);

        int pwmDireito = constrain(VELOCIDADE_BASE - saidaPID, 0, 255);
        int pwmEsquerdo = constrain(VELOCIDADE_BASE + saidaPID, 0, 255);

        analogWrite(motor_direito.pwm.p.enA, pwmDireito);
        analogWrite(motor_esquerdo.pwm.p.enB, pwmEsquerdo);

        erroAnterior = erroD;

        return 1;
    }
    else
    {
        integralErro = 0;
        analogWrite(motor_direito.pwm.p.enA, VELOCIDADE_BASE);
        analogWrite(motor_esquerdo.pwm.p.enB, VELOCIDADE_BASE);

        erroAnterior = erroD;

        return 0;
    }
}

#endif // PID_H