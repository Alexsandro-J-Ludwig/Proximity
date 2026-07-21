#include "init.h"

// Definição real das variáveis (alocação de memória)
Motor motor_direito = {{{33, 25, 0, 0}}, {{32, 0}}, 0};
Motor motor_esquerdo = {{{0, 0, 26, 27}}, {{0, 14}}, 0};

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Servo meuServo;

WiFiClient esp32client;
PubSubClient client(esp32client);

const char *ssid = "Vero";
const char *password = "tk43n@g.c";
const char *mqtt_server = "broker.hivemq.com";

unsigned long lastMsg = 0;

void setup_sensor()
{
  if (!Wire.begin(21, 22))
  {
    Serial.println(F("Falha ao inicializar o barramento I2C!"));
    while (1)
      ;
  }
  if (!lox.begin(VL53L0X_I2C_ADDR, false, &Wire,
                 Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED))
  {
    Serial.println("Erro ao iniciar VL53L0X");
    while (1)
      ;
  }

  Serial.println("VL53L0X iniciado");
}

void setup_servo()
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  meuServo.setPeriodHertz(50);
  meuServo.attach(18, 544, 2400);
  if (meuServo.attached())
    Serial.println(F("Servo conectado com sucesso!"));
  meuServo.write(90);
}

void setup_motores()
{
  pinMode(motor_direito.driver.p.motorA_in1, OUTPUT);
  pinMode(motor_direito.driver.p.motorA_in2, OUTPUT);
  pinMode(motor_direito.pwm.p.enA, OUTPUT);

  pinMode(motor_esquerdo.driver.p.motorB_in1, OUTPUT);
  pinMode(motor_esquerdo.driver.p.motorB_in2, OUTPUT);
  pinMode(motor_esquerdo.pwm.p.enB, OUTPUT);

  digitalWrite(motor_direito.driver.p.motorA_in1, HIGH);
  digitalWrite(motor_direito.driver.p.motorA_in2, LOW);
  digitalWrite(motor_esquerdo.driver.p.motorB_in1, LOW);
  digitalWrite(motor_esquerdo.driver.p.motorB_in2, HIGH);
}