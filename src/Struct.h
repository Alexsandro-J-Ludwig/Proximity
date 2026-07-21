#ifndef STRUCT_H
#define STRUCT_H

union L298N_Motor
{
    struct {
        int motorA_in1;
        int motorA_in2;
        int motorB_in1;
        int motorB_in2;
    } p;
    int pins[4];
};

union L298N_PWM
{
    struct {
        int enA;
        int enB;
    } p;
    int pins[2];
};

struct Motor {
    L298N_Motor driver;
    L298N_PWM pwm;
    int pwm_atual;
};

enum Estados {
    VOLTAR,
    AVANCAR,
    PROSSEGUIR
};

enum DirecaoDefinida {
    CurvaFechada,
    curva,
    Centro
};

#endif