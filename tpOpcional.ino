#include <ssl_client.h>
#include <WiFiClientSecure.h>

/*-----------------------------------------------------------------------------------------------------
  Sol Wiemeyer e Isabella Mastrángelo
  5LA 2023

  ----------------------------------------------------------------------------------------------------- */
/****LIBRERIAS****/
#include "U8g2lib.h"
#include "string"
//#include "DHT.h"
#include "DHT_U.h"
#include "Adafruit_Sensor.h"
#include "Arduino.h"
#include "ESP32Time.h"
#include "time.h"
#include "WiFi.h"
#include "Wire.h"

/*********CONSTRUCTORES Y VARIABLES GLOBALES**********/
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#define PIN_LED 25
int maquina = 0;
int VALOR_UMBRAL = 10;
#define BTN_SUMA 35
#define BTN_RESTA 34

#define pantalla1 0
#define pantalla2 1
#define pantalla3 2
#define limpiar1 3
#define limpiar2 4
#define limpiar3 5

String estadoRiego;
int horaRiego;
int minRiego;

int segRiego;

unsigned long tiempoActual;

void pedir_lahora(void);
void setup_rtc_ntp(void);

struct tm timeinfo;
ESP32Time rtc;


/// time
long unsigned int timestamp; // hora
const char *ntpServer = "south-america.pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";


char horaReal[2];
char minutosReal[2];
char horaString[2];
char minString[2];
char segString[2];

void setup()
{

  Serial.begin(115200);
  Serial.println(F("OLED test"));
  u8g2.begin();
  Serial.println("Connecting to Wi-Fi...");
  initWiFi();
  setup_rtc_ntp();

  pinMode(BTN_SUMA, INPUT_PULLUP);
  pinMode(BTN_RESTA, INPUT_PULLUP);

  pinMode(PIN_LED, OUTPUT);
}

void loop()
{
  pedir_lahora();
  int hora = timeinfo.tm_hour;
  int minuto = timeinfo.tm_min;

  sprintf(horaReal, "%i", hora);
  sprintf(minutosReal, "%i", minuto);
  sprintf(horaString, "%i", horaRiego);
  sprintf(minString, "%i", minRiego);
  sprintf(segString, "%i", segRiego);

  switch (maquina) {
    case pantalla1:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 12, "Hora:");
      u8g2.drawStr(0, 27, horaReal);
      u8g2.drawStr(25, 27, ":");
      u8g2.drawStr(30, 27, minutosReal);

      if (digitalRead(BTN_SUMA) == LOW && digitalRead(BTN_RESTA) == LOW) {
        maquina = limpiar1;
      }

      if ((horaReal == horaString) && (minutosReal == minString))
      {
        digitalWrite(PIN_LED, HIGH);
        tiempoActual = millis();
        u8g2.drawStr(0, 60, "Regando");
      }
      if (millis() - tiempoActual >= (segRiego * 1000))
      {
        digitalWrite(PIN_LED, LOW);
        u8g2.drawStr(0, 60, "No regando");
      }
      u8g2.sendBuffer();
      break;

    case limpiar1:
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == HIGH && digitalRead(BTN_RESTA) == HIGH) {
        maquina = pantalla2;
      }
      break;

    case pantalla2:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 20, "Hora de riego: ");
      u8g2.drawStr(0, 40, horaString);
      u8g2.drawStr(25, 40, " : ");
      u8g2.drawStr(30, 40, minString);
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == LOW && digitalRead(BTN_RESTA) == LOW) {
        maquina = limpiar2;
      }
      while (digitalRead(BTN_SUMA) == LOW) {
        if (digitalRead(BTN_SUMA) == HIGH) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          horaRiego++;
          if (horaRiego >= 24)
          {
            horaRiego = 0;
          }
        }
      }
      while (digitalRead(BTN_RESTA) == LOW) {
        if (digitalRead(BTN_RESTA) == HIGH) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          minRiego++;
          if (minRiego >= 60)
          {
            minRiego = 0;
          }
        }
      }
      break;

    case limpiar2:
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == HIGH && digitalRead(BTN_RESTA) == HIGH) {
        maquina = pantalla3;
      }
      break;

    case pantalla3:
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 30, "Tiempo R: ");
      u8g2.drawStr(0, 50, segString);
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == LOW && digitalRead(BTN_RESTA) == LOW) {
        maquina = limpiar3;
      }
      while (digitalRead(BTN_SUMA) == LOW) {
        if (digitalRead(BTN_SUMA) == HIGH) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          segRiego += 30;
        }
      }
      while (digitalRead(BTN_RESTA) == LOW) {
        if (digitalRead(BTN_RESTA) == HIGH) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          segRiego -= 30;
        }
      }
      break;

    case limpiar3:
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == HIGH && digitalRead(BTN_RESTA) == HIGH) {
        maquina = pantalla1;
      }
      break;
  }

}

//Función conexión WiFi
void initWiFi()
{
  WiFi.begin(ssid , password );
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

//Funciones Tiempo
void setup_rtc_ntp(void)
{

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  timestamp = time(NULL);
  rtc.setTime(timestamp + gmtOffset_sec);
}


void pedir_lahora(void)
{
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("veo la hora del rtc interno ");
    timestamp = rtc.getEpoch() - gmtOffset_sec;
    timeinfo = rtc.getTimeStruct();
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
  else
  {
    Serial.print("NTP Time:");
    timestamp = time(NULL);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }

  return;
}
