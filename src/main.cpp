#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>

#include "Struct.h"
#include "PID.h"
#include "init.h"

unsigned long servoDelay = 150;
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

      bool cheguei_na_ponta = (tofIndex == 0 || tofIndex == 5);

      if (servoDirection)
      {
        tofIndex++;
        if (tofIndex >= 5)
          servoDirection = false;
      }
      else
      {
        tofIndex--;
        if (tofIndex <= 0)
          servoDirection = true;
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

    if ((maiorDistancia < 200 || menorDistancia < 80) && !voltando)
    {
      estadoAtual = Estados::VOLTAR;
      voltando = true;
      voltarTimer = currentTime;
    }

    if (voltando && (currentTime - voltarTimer >= voltarDelay))
    {
      voltando = false;

      if (maiorDistancia < 500)
        estadoAtual = Estados::AVANCAR;
    //   else
    //     estadoAtual = Estados::PROSSEGUIR;
   }

    switch (estadoAtual)
    {
    case Estados::AVANCAR:
      digitalWrite(motor_direito.driver.p.motorA_in1, HIGH);
      digitalWrite(motor_direito.driver.p.motorA_in2, LOW);
      digitalWrite(motor_esquerdo.driver.p.motorB_in1, LOW);
      digitalWrite(motor_esquerdo.driver.p.motorB_in2, HIGH);

      Serial.print("Melhor: ");
      Serial.println(melhorAngulo);

      pid(motor_direito, motor_esquerdo, currentTime, anguloAlvo, maiorDistancia);
      break;
    // case Estados::VOLTAR:
    //   digitalWrite(motor_direito.driver.p.motorA_in1, LOW);
    //   digitalWrite(motor_direito.driver.p.motorA_in2, HIGH);
    //   digitalWrite(motor_esquerdo.driver.p.motorB_in1, HIGH);
    //   digitalWrite(motor_esquerdo.driver.p.motorB_in2, LOW);

    //   analogWrite(motor_direito.pwm.p.enA, VELOCIDADE_BASE);
    //   analogWrite(motor_esquerdo.pwm.p.enB, VELOCIDADE_BASE);
    //   break;
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