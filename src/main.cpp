#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>

#include "Struct.h"
#include "PID.h"
#include "init.h"

unsigned long servoDelay = 90;
unsigned long servoTimer = 0;
unsigned long motorDelay = 2;
unsigned long motorTimer = 0;
unsigned long voltarTimer = 0;
unsigned long voltarDelay = 500;

bool voltando = false;

int acumulador = 36;

bool servoDirection = true;
int tofValues[6] = {};
int tofIndex = 0;
int angulos[6] = {0, 36, 72, 108, 144, 180};
int maiorDistancia = 0;
int melhorAngulo = 90;
int menorDistancia = 999;

Estados estadoAtual = Estados::AVANCAR;

SemaphoreHandle_t xMutexRadar;

void tarefaCore0(void *pvParameters)
{
  setup_servo();
  setup_sensor();

  for (;;)
  {
    unsigned long currentMillis = millis();

    // Faz a leitura do sensor ToF
    VL53L0X_RangingMeasurementData_t measure;

    lox.rangingTest(&measure, false);

    int tofValue = 8191;

    if (measure.RangeStatus != 4)
    {
      tofValue = measure.RangeMilliMeter;
    }

    if (currentMillis - servoTimer >= servoDelay)
    {
      servoTimer = currentMillis;

      tofValues[tofIndex] = tofValue;
      meuServo.write(angulos[tofIndex]);

      // Guarda se ESTE índice (que acabamos de escrever) é uma ponta da varredura.
      // Precisa ser checado ANTES de mover o índice, senão o valor da ponta
      // (36° ou 180°) nunca é considerado no cálculo do melhor ângulo.
      bool cheguei_na_ponta = (tofIndex == 0 || tofIndex == 5);

      // Atualiza o índice para o próximo ciclo
      if (servoDirection)
      {
        tofIndex++;
        if (tofIndex >= 5)
          servoDirection = false; // Bateu no fim (180°) inverter
      }
      else
      {
        tofIndex--;
        if (tofIndex <= 0)
          servoDirection = true; // Bateu no começo (0°) inverter
      }

      if (cheguei_na_ponta)
      {
        int tempMaiorDistancia = 0;
        int tempMenorDistancia = 9999;
        int tempMelhorAngulo = 90;

        for (int c = 0; c < 6; c++)
        {
          if (tofValues[c] > tempMaiorDistancia)
          {
            tempMaiorDistancia = tofValues[c];
            tempMelhorAngulo = angulos[c];
          }
          // Só aceita a menor distância se for maior que 0 (ignora falhas e fantasmas)
          if (tofValues[c] > 0 && tofValues[c] < tempMenorDistancia)
          {
            tempMenorDistancia = tofValues[c];
          }
        }

        if (xSemaphoreTake(xMutexRadar, pdMS_TO_TICKS(6)) == pdTRUE)
        {
          maiorDistancia = tempMaiorDistancia;
          menorDistancia = tempMenorDistancia;
          melhorAngulo = tempMelhorAngulo;
          xSemaphoreGive(xMutexRadar);
        }

        // Limpa o array para a próxima varredura
        for (int r = 0; r < 6; r++)
        {
          tofValues[r] = 0;
        }
      }
    }

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  setup_motores();

  xMutexRadar = xSemaphoreCreateMutex();

  if (xMutexRadar == NULL)
  {
    Serial.println("Falha ao criar o mutex!");
    while (1)
      ;
  }

  xTaskCreatePinnedToCore(
      tarefaCore0, "Varredura", 4096, NULL, 1, NULL, 0);
}

void loop()
{
  unsigned long currentTime = millis();

  if (currentTime - motorTimer >= motorDelay)
  {
    motorTimer = currentTime;
    int anguloAlvo = 90;

    if (xSemaphoreTake(xMutexRadar, pdMS_TO_TICKS(2)) == pdTRUE)
    {
      anguloAlvo = melhorAngulo;
      xSemaphoreGive(xMutexRadar);
    }

    if ((maiorDistancia < 300 || menorDistancia < 100) && !voltando)
    {
      estadoAtual = Estados::VOLTAR;
      voltando = true;
      voltarTimer = currentTime; // marca o momento em que entrou em ré
    }

    // Sai da ré após 500 ms
    if (voltando && (currentTime - voltarTimer >= voltarDelay))
    {
      voltando = false;

      // Decide o próximo estado
      if (maiorDistancia < 700)
        estadoAtual = Estados::AVANCAR;
      else
        estadoAtual = Estados::PROSSEGUIR;
    }

    switch (estadoAtual)
    {
    case Estados::AVANCAR:
      digitalWrite(motor_direito.driver.p.motorA_in1, HIGH);
      digitalWrite(motor_direito.driver.p.motorA_in2, LOW);
      digitalWrite(motor_esquerdo.driver.p.motorB_in1, LOW);
      digitalWrite(motor_esquerdo.driver.p.motorB_in2, HIGH);
      pid(motor_direito, motor_esquerdo, currentTime, anguloAlvo, maiorDistancia);
      break;
    case Estados::VOLTAR:
      digitalWrite(motor_direito.driver.p.motorA_in1, LOW);
      digitalWrite(motor_direito.driver.p.motorA_in2, HIGH);
      digitalWrite(motor_esquerdo.driver.p.motorB_in1, HIGH);
      digitalWrite(motor_esquerdo.driver.p.motorB_in2, LOW);

      pid(motor_direito, motor_esquerdo, currentTime, anguloAlvo, maiorDistancia);
      break;
    case Estados::PROSSEGUIR:
      digitalWrite(motor_direito.driver.p.motorA_in1, HIGH);
      digitalWrite(motor_direito.driver.p.motorA_in2, LOW);
      digitalWrite(motor_esquerdo.driver.p.motorB_in1, LOW);
      digitalWrite(motor_esquerdo.driver.p.motorB_in2, HIGH);

      analogWrite(motor_direito.pwm.p.enA, VELOCIDADE_BASE);
      analogWrite(motor_esquerdo.pwm.p.enB, VELOCIDADE_BASE);
      break;
    default:
      break;
    }
  }
}