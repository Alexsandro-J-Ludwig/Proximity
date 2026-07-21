#ifndef INIT_H
#define INIT_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Struct.h"

extern Motor motor_direito;
extern Motor motor_esquerdo;
extern Adafruit_VL53L0X lox;
extern Servo meuServo;
extern WiFiClient esp32client;
extern PubSubClient client;

extern const char *mqtt_server;

void setup_sensor();
void setup_servo();
void setup_motores();

#endif // INIT_H