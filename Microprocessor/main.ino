#include <WiFi.h>
#include <driver/i2s.h>
#include "parameters.h"

#include  <DHT.h>

volatile bool timerFlag = false;

static TaskHandle_t task_wifi = NULL;
static TaskHandle_t task_i2s = NULL;

static TimerHandle_t timer1 = NULL;

static QueueHandle_t q_i2s2wifi = NULL;

void setupWifi();
void setupQueues();
void setupTasks();
void i2s_install();
void i2s_setpin();
void TaskWifi(void* parameters);
void TaskI2S(void*  parameters);
void parser(String s);
void timer1_callback(TimerHandle_t timer);
void setupTimers();
void(* resetFunc) (void) = 0;  // to reset esp via code

uint16_t servo_duty(int degree);
uint16_t motor_duty(int rot);

void setup()
{
  Serial.begin(115200);
  setupWifi();
  delay(100);
  setupQueues();
  delay(100);
  setupTasks();
  delay(100);
  setupTimers();
  vTaskDelete(NULL);
}

void loop()
{
}

void TaskWifi(void* parameters)
{
  Serial.println("Wifi Task started");
  WiFiClient client;

  uint16_t voiceData[I2S_SIZE];
  client.connect(ServerIP, ServerPort);

  DHT dht(DHTPIN,DHTTYPE);

  for(;;)
  {
      if(xQueueReceive(q_i2s2wifi, voiceData, 0) != pdTRUE){
         //Queue boş ise burası çalışır
      }
      else {
        client.write_P((const char*)voiceData, I2S_SIZE * 2);
        //Veriyi Wi-Fi üzerinden gönder
      }

      if(client.available())
      {
        String in = "";
        in = client.readString();
        for(int i=0; i<in.length(); i+=4)
        {
          String cmd = in.substring(i,i+4);
          parser(cmd);
        }
      }

      if (timerFlag)
      {
        uint8_t sensor[128] = {};
        // Sensör verisi header'ı
        sensor[0] = 0; sensor[1] = 1; sensor[2] = 0;
        sensor[3] = 1; sensor[4] = 0; sensor[5] = 1;
        sensor[6] = 0; sensor[7] = 1; sensor[8] = 0;
        sensor[9] = 1;

        sensor[10] = dht.readTemperature();
        sensor[11] = dht.readHumidity();
        client.write_P((const char*)sensor, 128);
        timerFlag = false;
      }
  }
}

void TaskI2S(void*  parameters)
{
  Serial.println("Setup I2S ...");

  delay(1000);
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  delay(500);

  int16_t sBuffer[I2S_SIZE];
  size_t bytesIn = 0;
  size_t buffSize = sizeof(sBuffer);
  
  for(;;)
  {
    esp_err_t result = i2s_read(I2S_PORT, &sBuffer, buffSize, &bytesIn, portMAX_DELAY);
    if(result == ESP_OK)
    {
      if(xQueueSend(q_i2s2wifi, sBuffer, I2S_SIZE) != pdTRUE)
      {
        //Serial.println("000");
      }
      else
      {
        //Serial.println("0000");
      }
    } 
  }
}

void setupWifi()
{
  Serial.println("Started...");
  WiFi.begin(ssid, password);
  uint8_t wifiCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
    wifiCnt++;
    if(wifiCnt > 10)
    {resetFunc();}
  }
  Serial.print("WiFi connected with IP:");
  Serial.println(WiFi.localIP());
}

void setupQueues()
{
    // i2s 54 tane uint16_t alacak ve wifi'a gönderecek.
  q_i2s2wifi = xQueueCreate(QUEUE_SIZE, I2S_SIZE * sizeof(uint16_t));  
  if(q_i2s2wifi == NULL)
  {
    Serial.println("i2s2wifi Q could not be created");
    while(1);
  }
  else
  {
    Serial.println("i2s2wifi Q created");
  }
}

void setupTasks()
{
                    //   | Func Name |    Name    | size | Params | prio |  handle  | core |
  xTaskCreatePinnedToCore(  TaskWifi  ,"Task Wifi", 3100 ,  NULL  ,   1  , &task_wifi,  0   );
  xTaskCreatePinnedToCore(  TaskI2S   ,"Task I2S" , 2200 ,  NULL  ,   1  , &task_i2s ,  1   );
}

void i2s_install(){
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin(){
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setupTimers()
{                     // name      time           auto reload   ID    Func 
  timer1 = xTimerCreate("1 Sec", pdMS_TO_TICKS(10000), pdTRUE, 0, timer1_callback);
  if(xTimerStart(timer1, 0) == pdPASS)
  {
    Serial.println("Timer started");
  }
}

void parser(String s)
{
  if(s[0] == 'l')
  {
    printCommand(s);
    uint8_t pin = s[1];
    uint8_t state = s[2];
    pinMode(pin, OUTPUT);
    digitalWrite(pin,state);
  }
  else if(s[0] == 's')
  {
    printCommand(s);
    uint8_t pin = s[1];
    uint8_t degree = s[2];
    uint8_t channel = s[3];
    ledcSetup(channel, SERVO_FREQ, SERVO_RESOLUTION);
    ledcAttachPin(pin, channel);
    ledcWrite(channel,servo_duty(degree));
  }
  else if(s[0] == 'm')
  {
    printCommand(s);
    uint8_t pin = s[1];
    uint8_t rotation = s[2];
    uint8_t channel = s[3];
    ledcSetup(channel, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(pin, channel);
    ledcWrite(channel,motor_duty(rotation));
  }
}

void printCommand(String s)
{ 
  Serial.print(s[0]);
  Serial.print("  ");
  Serial.print((uint8_t)s[1]);
  Serial.print("  ");
  Serial.print((uint8_t)s[2]);
  Serial.print("  ");
  Serial.println((uint8_t)s[3]);
}

void timer1_callback(TimerHandle_t timer)
{
  timerFlag = true;
}

uint16_t servo_duty(int degree)
{
  return degree * 100 / 180 + 25;
}

uint16_t motor_duty(int rot)
{
  return rot * 1024 / 100;;
}