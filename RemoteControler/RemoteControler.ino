#include <SoftwareSerial.h>

#define SS_TX 2
#define SS_RX 3
#define BT_PIN 8

SoftwareSerial ss(SS_TX, SS_RX);

void setup()
{
  Serial.begin(9600);
  Serial.println("Start!");
  ss.begin(9600);
  
  pinMode(BT_PIN, INPUT);  
  pinMode(A0, INPUT);
}

int StartBT = 0;
int GameStatus = 0;
int Handle = 0;
int oldHandle = 0;
String msg;

void loop()
{
  if (Serial.available() > 0)
  {
    String tmp = Serial.readString() + '\0';
    ss.print(tmp);
    Serial.println(tmp);
  }
  
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
      msg = "START" + '\0';
      ss.print(msg);
      Serial.println("START");
    }
    else
    {
      GameStatus = 0;
      msg = "STOP" + '\0';
      ss.print(msg);
      Serial.println("STOP");
    }
  }
  
  Handle = map(analogRead(A0), 1, 1022, 45, 145);
  if (oldHandle != Handle)
  {
    oldHandle = Handle;
    msg = "M," + String(Handle) + '\0';
    ss.print(msg);
    Serial.println(msg);
  }
}
