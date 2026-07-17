#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Servo.h>
#include "Struct.h"
#include "PID.h"

Motor motor_direito = {
    {{25, 32, 0, 0}},
    {{4, 0}},
    0};

Motor motor_esquerdo = {
    {{0, 0, 18, 17}},
    {{0, 16}},
    0};

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Servo meuServo;

unsigned long servoDelay = 33;
unsigned long servoTimer = 0;
unsigned long motorDelay = 2;
unsigned long motorTimer = 0;

int i = 0;
bool servoDirection = true;
int tofValues[5] = {};
int tofIndex = 0;
int maiorDistancia = 0;
int melhorAngulo = 0;

// Função para configurar o servo e sensor
// !!! Será isolado em um header em breve !!!
void setup_servo()
{
  meuServo.attach(12, 544, 2400);

  if (meuServo.attached())
  {
    Serial.println(F("Servo conectado com sucesso!"));
  }
  else
  {
    Serial.println(F("Falha ao conectar o servo!"));
  }

  meuServo.write(90); // Posição inicial do servo

  if (meuServo.read() == 90)
  {
    Serial.println(F("Servo posicionado corretamente!"));
  }
  else
  {
    Serial.println(F("Falha ao posicionar o servo!"));
  }
}

void setup_sensor()
{
  if (!Wire.begin(21, 22))
  { // Inicialização do barramento I2C
    Serial.println(F("Falha ao inicializar o barramento I2C!"));
    while (1)
      ;
  }

  lox.begin(VL53L0X_I2C_ADDR, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED); // Inicialização do sensor

  lox.startMeasurement(); // Inicia a medição do sensor
}

// Não faz nada :)
void setup()
{

  Serial.begin(115200);
}

void setup1()
{
  while (!Serial)
    delay(10); // para nativos USB

  xTaskCreatePinnedToCore(
      tarefaCore0, // Função que contém o código do seu loop1
      "Varredura", // Nome da tarefa
      4096,        // Tamanho da memória (Stack) alocada
      NULL,        // Parâmetros para a tarefa
      1,           // Prioridade da tarefa
      NULL,        // Tratador da tarefa
      0            // ID do Core (0 ou 1) -> Forçamos no Core 0!
  );
}

void loop()
{
  // Aqui ficará a lógica de estados do robô
  int currentTime = millis();

  if (currentTime - motorTimer >= motorDelay)
  {
    motorTimer = currentTime;

    pid(motor_direito, motor_esquerdo, motorDelay, melhorAngulo);
  }
}

void tarefaCore0(void *pvParameters)
{
  // Tudo o que estava no seu antigo setup1() entra aqui:
  setup_servo();
  setup_sensor();

  // O loop infinito que vai rodar no Core 0 em paralelo:
  for (;;)
  {
    unsigned long currentMillis = millis();

    VL53L0X_RangingMeasurementData_t measure;
    lox.getRangingMeasurement(&measure);
    int tofValue = measure.RangeMilliMeter / 10;

    if (currentMillis - servoTimer >= servoDelay)
    {
      servoTimer = currentMillis;

      i += servoDirection ? 1 : -1;
      i = constrain(i, 0, 180);
      if (i == 180)
        servoDirection = false;
      if (i == 0)
        servoDirection = true;

      if (i % 5 == 0)
      {
        meuServo.write(i);
        int distanciaTratada = (tofValue > 50) ? 50 : tofValue;

        if (distanciaTratada > maiorDistancia)
        {
          maiorDistancia = distanciaTratada;
          melhorAngulo = i;
        }
      }
    }
    // Dica de ouro para o ESP32: Um delay mínimo pro watchdog do sistema respirar
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}