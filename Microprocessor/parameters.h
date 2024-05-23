#pragma once

// I2S Parameters
#define bufferCnt 8
#define I2S_SIZE 64

#define QUEUE_SIZE 50

// INMP441 PIN NUMBERS
#define I2S_WS 15
#define I2S_SD 13
#define I2S_SCK 4
#define I2S_PORT I2S_NUM_0

// SENSOR PARAMETERS
#define DHTPIN 32
#define DHTTYPE DHT11

// SERVO AND DC MOTOR PARAMETERS
#define SERVO_FREQ 50
#define SERVO_RESOLUTION 10

#define MOTOR_FREQ 50000
#define MOTOR_RESOLUTION 10

// WIFI PARAMETERS
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* ServerIP = "YOUR_SERVER_ID";
const uint16_t ServerPort = YOUR_SERVER_PORT;
