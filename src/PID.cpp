#ifndef PID_H
#define PID_H
#include "Struct.h"
#include <Arduino.h>
#include "PID.h"
#include "init.h"

float Kp = 0.3;
float Ki = 0.05;
float Kd = 0.3;
int VELOCIDADE_BASE = 60;

// Variáveis de estado do PID
static unsigned long tempoUltimoCalculo = 0;
static float erroAnterior = 0.0;
static float integralErro = 0.0;

DirecaoDefinida direcao = DirecaoDefinida::Centro;

bool pid(Motor &motor_direito, Motor &motor_esquerdo, unsigned long tempoAtual, int anguloAlvo, int maiorDistancia)
{
    float dt = (tempoAtual - tempoUltimoCalculo) / 1000.0;

    // Proteção contra divisão por zero no primeiro ciclo ou chamadas rápidas demais
    if (dt <= 0.0)
        return false;

    tempoUltimoCalculo = tempoAtual;

    // Erro: Onde eu quero ir (90 graus, que é o centro) vs Onde o caminho livre está
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

    // Calcula as componentes do PID
    float derivada = (erroD - erroAnterior) / dt;

    if (abs(erroD) > 0.1)
    {
        // Acumula a integral com proteção contra "Windup" (limite de saturação)
        integralErro = constrain(integralErro + (erroD * dt), -50.0, 50.0);

        float kp_ativo = Kp;
        float kd_ativo = Kd;

        switch (direcao)
        {
        case DirecaoDefinida::CurvaFechada:
            kp_ativo = 0.6;
            kd_ativo = 0.3;
            break;
        case DirecaoDefinida::curva:
            kp_ativo = 0.2;
            kd_ativo = 0.4;
            break;
        case DirecaoDefinida::Centro:
            kp_ativo = 0.3;
            kd_ativo = 0.3;
            break;
        default:
            break;
        }

        // A Mágica Acontece Aqui
        float saidaPID = (kp_ativo * erroD) + (Ki * integralErro) + (kd_ativo * derivada);

        // Cinemática Diferencial: Ajusta as velocidades dos motores
        int pwmDireito = constrain(VELOCIDADE_BASE - saidaPID, 0, 255);
        int pwmEsquerdo = constrain(VELOCIDADE_BASE + saidaPID, 0, 255);

        analogWrite(motor_direito.pwm.p.enA, pwmDireito);
        analogWrite(motor_esquerdo.pwm.p.enB, pwmEsquerdo);

        erroAnterior = erroD;

        return 1;
    }
    else
    {
        // Se está alinhado, zera a memória da integral para não dar tranco depois
        integralErro = 0;
        analogWrite(motor_direito.pwm.p.enA, VELOCIDADE_BASE);
        analogWrite(motor_esquerdo.pwm.p.enB, VELOCIDADE_BASE);

        erroAnterior = erroD;

        return 0;
    }
}

#endif // PID_H