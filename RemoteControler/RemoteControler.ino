
#include <SoftwareSerial.h>

#define SS_TX 2
#define SS_RX 3

#define BT_PIN 9

SoftwareSerial ss(SS_TX, SS_RX);

void setup()
{
  Serial.begin(9600);
  Serial.println("Start!");
  ss.begin(9600);
  
  pinMode(A0, INPUT);
  pinMode(8, OUTPUT);
  pinMode(BT_PIN, INPUT);
  
  digitalWrite(8, HIGH);
}

int StartBT = 0;
int GameStatus = 0;
int Handle = 0;
int oldHandle = 0;
String msg;
int n = 0;

void loop()
{
  if (StartBT == 0 && digitalRead(BT_PIN) == LOW)
  {
    StartBT = 1;
  }
  if (StartBT == 1 && digitalRead(BT_PIN) == HIGH)
  {
    StartBT = 0;

    if (GameStatus == 0)
    {
      GameStatus = 1;
      ss.print("START");
      ss.print('\0');
      Serial.println("START");
    }
    else
    {
      GameStatus = 0;
      ss.print("STOP");
      ss.print('\0');
      Serial.println("STOP");
    }
  }
  
  if (Serial.available() > 0)
  {
    String tmp = Serial.readString() + '\0';
    ss.print(tmp);
    Serial.println(tmp);
  }
  Handle = analogRead(A0);//, 0, 1023, 35, 150);
  uint8_t a = abs(Handle - oldHandle);
  if (a > 5)
  {
    oldHandle = Handle;
    msg = "M," + String(Handle) + '\0';
    ss.print(msg);
    Serial.println(msg);
  }
}
