#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 17
#define DHTTYPE DHT22
#define PWM_FREQ 32000
#define PWM_RESOLUTION 8

DHT dht(DHTPIN, DHTTYPE);
void updateData();
void setupPWM(int pin);
void analogWrite(int pin, uint8_t val);
String get_color_string(String var);
void color_mode(const String &var);
void set_color(int r, int g, int b);
bool check_message();
void process_message();

long timeSinceLastUpdate = 0;
const int Update_Data_Interval = 2;

int red = 0;
int green = 0;
int blue = 0;
int rainbow = 0;
int Led_Green = 27;
int Led_Red = 14;
int Led_Blue = 32;
int _adc = 0;
int ADC_MAP[48] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1};
String message = "";
int messageFlag = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  pinMode(Led_Red, OUTPUT);
  pinMode(Led_Green, OUTPUT);
  pinMode(Led_Blue, OUTPUT);
  dht.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (millis() - timeSinceLastUpdate > (1000L * Update_Data_Interval))
  {
    updateData();
    timeSinceLastUpdate = millis();
  }
  if (Serial.available() > 0 || messageFlag == 1)
  {
    if (!messageFlag)
      message = Serial.readString();
    process_message();
  }
}

void process_message()
{
  if(message == "") return;
  if (message[0] == 'C')
  {
    Serial.println(message);
    sscanf(message.c_str(), "C:%d-%d-%d", &red, &green, &blue);
    if (!rainbow)
      set_color(red, green, blue);
  }
  else if (message[0] == 'R')
  {
    Serial.println(message);
    sscanf(message.c_str(), "R:%d", &rainbow);
    if (rainbow)
      color_mode("rainbow");
    else
      set_color(red, green, blue);
  }
  message = "";
  messageFlag = 0;
}

bool check_message()
{
  if (Serial.available() > 0)
  {
    message = Serial.readString();
    messageFlag = 1;
    if (message[0] != 'R')
    {
      process_message();
      return false;
    }
    process_message();
    return true;
  }
  else
  {
    return false;
  }
}

void updateData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.println("D:" + String(t) + '-' + String(h) + '-' + String(hic));
  return;
}

void setupPWM(int pin)
{
  pinMode(pin, OUTPUT);
#ifdef USE_SIGMADELTA
  sigmaDeltaSetup(_adc, PWM_FREQ);
  sigmaDeltaAttachPin(pin, _adc);
#else
  ledcSetup(_adc, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(pin, _adc);
#endif
  ADC_MAP[pin] = _adc;
  _adc = _adc + 1;
}

void analogWrite(int pin, uint8_t val)
{
  if (ADC_MAP[pin] == -1)
    setupPWM(pin);
#ifndef USE_SIGMADELTA
  ledcWrite(ADC_MAP[pin], val);
#else
  sigmaDeltaWrite(ADC_MAP[pin], val);
#endif
}

String get_color_string(String var)
{
  int i = var.length() - 1;
  for (; i >= 0; i--)
  {
    if (var[i] == ' ')
    {
      i++;
      break;
    }
  }
  if (i < 0)
    i = 0;
  for (int j = i; j < var.length() - 1; j++)
  {
    if (var[j] >= 'A' && var[j] <= 'Z')
      var[j] = var[j] - 'A' + 'a';
  }
  return var.substring(i);
}

void set_color(int r, int g, int b)
{
  analogWrite(Led_Red, r);
  analogWrite(Led_Green, g);
  analogWrite(Led_Blue, b);
  return;
}

void color_mode(const String &var)
{
  if (!strcmp(var.c_str(), "red"))
  {
    analogWrite(Led_Red, 255);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 0);
    return;
  }
  if (!strcmp(var.c_str(), "blue"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 255);
    analogWrite(Led_Green, 0);
    return;
  }
  if (!strcmp(var.c_str(), "green"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 255);
    return;
  }
  if (!strcmp(var.c_str(), "green"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 255);
    return;
  }
  if (!strcmp(var.c_str(), "rainbow"))
  {
    int val;
    while (1)
    {
      for (val = 255; val > 0; val--)
      {
        analogWrite(Led_Red, val);
        analogWrite(Led_Blue, 255 - val);
        analogWrite(Led_Green, 128 - val);
        if (check_message()){
          return;
        }
        delay(15);
      }
      updateData();
      for (val = 0; val < 255; val++)
      {
        analogWrite(Led_Red, val);
        analogWrite(Led_Blue, 255 - val);
        analogWrite(Led_Green, 128 - val);
        if (check_message()){
          return;
        }
        delay(15);
      }
      updateData();
    }
    return;
  }
  if (!strcmp(var.c_str(), "off"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 0);
    return;
  }
  return;
}