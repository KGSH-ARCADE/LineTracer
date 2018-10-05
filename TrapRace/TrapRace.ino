
#define SS_TX 2
#define SS_RX 3

#define MOTOR_A_DIR 4
#define MOTOR_A_PWM 5

#define SV_HANDLE  9

#define LINE_L   A5
#define LINE_LC  A4
#define LINE_RC  A3
#define LINE_R   A2

#include <Servo.h>
#include <SoftwareSerial.h>

enum MOVE_STATUS
{
  MOVE_STOP = -1,
  MOVE_FORWARD,
  MOVE_REVERSE,
};

Servo myServo;
SoftwareSerial ss(SS_TX, SS_RX);

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  ss.begin(9600);
  
  pinMode(LINE_L, INPUT);
  pinMode(LINE_LC, INPUT);
  pinMode(LINE_RC, INPUT);
  pinMode(LINE_R, INPUT);

  pinMode(MOTOR_A_DIR, OUTPUT);
  pinMode(MOTOR_A_PWM, OUTPUT);

  myServo.attach(SV_HANDLE);
  myServo.write(90);

  digitalWrite(MOTOR_A_DIR, LOW);
  analogWrite(MOTOR_A_PWM, 0);
}

String str;
String tmp;
char c;

unsigned long GameTime = 0;
int GameStatus = 0;

unsigned long TrapTime = 0;
int CarStatus = 0;
int CarSpeed = 0;
int PrevCarSpeed = 0;
int CarHandle = 0;
int PrevCarHandle = 0;

void loop()
{
  if (Serial.available() > 0)
  {
    str = Serial.readString();
    Enter();
  }

  while (ss.available() > 0)
  {
    char a = ss.read();
    if (a == '\0')
    {
      Serial.println(str);
      Enter();
      break;
    }

    str += a;
    continue;
  }

  if (GameStatus == 1)
  {
    GameRun();
    
    // 게임시간 1분 30초 = 90초 * 1000ms
    if (millis() - GameTime > 90000)
    {
      GameStatus = 0;
      
      Move(MOVE_STOP, 255);
      Serial.println("Race Timeout!");
    }
  }

  // 이전 핸들값과 다른 값이 계산되면 적용
  if (PrevCarHandle != CarHandle)
  {
    PrevCarHandle = CarHandle;
    myServo.write(CarHandle);
    Serial.print("Handle: ");
    Serial.println(CarHandle);
  }
}

void Enter()
{
  if (str.equals("START"))
  {
    GameTime = millis();
    GameStatus = 1;

    TrapTime = millis();
    CarStatus = 0;
    CarSpeed = 0;
    PrevCarSpeed = 0;
    Serial.println("Race Start!");
  }
  else if (str.equals("STOP"))
  {
    GameStatus = 0;

    Move(MOVE_STOP, 255);
    Serial.println("Race Stop!");
  }
  else
  {
    int sp = str.indexOf(',');
    if (sp > 0)
    {
      tmp = str.substring(0, sp);
      if (tmp[0] == 'M')
      {
        int n = str.substring(sp + 1).toInt();
        if (n >= 0 && n <= 1023)
        {
          CarHandle = map(n, 0, 1023, 35, 145);
        }
      }
    }
  }

  str = "";
}

void GameRun()
{
  // 라인센서에 300 보다 작은 값이 들어오면 트랩 가동
  for (int i = 0; i < 4; i++)
  {
    if (analogRead(A2 + i) < 300)
    {
      TrapTime = millis();
      CarStatus = 0;
      CarSpeed = 130;
      Serial.println("Trap On");
      break;
    }
  }

  if (CarStatus == 0)
  {
    // 트랩 가동중 트랩을 벗어난지 0.5초가 지나면 트랩 해제
    if (millis() - TrapTime >= 500)
    {
      CarStatus = 1;
      Serial.println("Trap Off");
    }
  }
  else if (CarStatus == 1)
  {
    // 트랩 해제된 상태에서 천천히 가속
    int c = (millis() - TrapTime) / 50;
    // 최대 가속 상태이므로 더이상 계산하지 않기 위함
    if (c >= 125)
    {
      CarStatus = 2;
      c = 125;
    }
    
    CarSpeed = 130 + c;
  }

  // 이전 속도와 다른 속도가 계산되면 적용
  if (PrevCarSpeed != CarSpeed)
  {
    PrevCarSpeed = CarSpeed;
    Move(MOVE_FORWARD, CarSpeed);
    Serial.print("Speed UP: ");
    Serial.println(CarSpeed);
  }
}

void Move(int8_t _dir, int _speed)
{
  // 이동방향
  if (_dir == MOVE_STOP)
  {
    _speed = 0;
    _dir++;
  }

  // 속도 최대값과 최소값을 넘길시 조정
  if (_speed > 255)
  {
    _speed = 255;
  }
  else if (_speed < 0)
  {
    _speed = 0;
  }

  digitalWrite(MOTOR_A_DIR, _dir);
  analogWrite(MOTOR_A_PWM, _speed);
}
