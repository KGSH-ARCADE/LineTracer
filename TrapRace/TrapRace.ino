#include <Servo.h>
#include <SoftwareSerial.h>

#define DEBUG 0

#define SS_TX 2 // 블루투스 TX
#define SS_RX 3 // 블루투스 RX
#define MOTOR_A_DIR 4 // IB
#define MOTOR_A_PWM 5 // IA
#define SV_HANDLE  6
#define LINE_L   A0
#define LINE_R   A1
#define LINE_TRAP 250

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
#if DEBUG == 1
  Serial.begin(9600);
  Serial.println("Start");
#endif
  ss.begin(9600);
  
  pinMode(LINE_L, INPUT);
  pinMode(LINE_R, INPUT);
  pinMode(MOTOR_A_DIR, OUTPUT);
  pinMode(MOTOR_A_PWM, OUTPUT);

  myServo.attach(SV_HANDLE);

  // 테스트. 핸들 맥시멈 위치 조정 후 정위치
  myServo.write(0);
  delay(700);
  myServo.write(180);
  delay(700);
  myServo.write(90);
  
  delay(500);
  
  // 테스트. 전후진 후 정지
  Move(MOVE_FORWARD, 100);
  delay(300);
  Move(MOVE_STOP, 0);
  delay(500);
  Move(MOVE_REVERSE, 100);
  delay(300);
  Move(MOVE_STOP, 0);
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
#if DEBUG == 1
  if (Serial.available() > 0)
  {
    str = Serial.readString();
    Enter();
  }
#endif

  if (ss.available() > 0)
  {
    c = ss.read();
    if (c == '\0')
    {
      Enter();
    }
    else str += c;
  }
  
  if (GameStatus == 1)
  {
    GameRun();
    
    // 게임시간 1분 = 60초 * 1000ms
    if (millis() - GameTime >= 60000)
    {
      GameStatus = 0;
      
      Move(MOVE_STOP, 0);
#if DEBUG == 1
      Serial.println("Race Timeout!");
#endif
    }
  }
  
  // 이전 핸들값과 다른 값이 계산되면 적용
  if (PrevCarHandle != CarHandle)
  {
    PrevCarHandle = CarHandle;
    myServo.write(CarHandle);
#if DEBUG == 1
    Serial.print("Handle: ");
    Serial.println(CarHandle);
#endif
  }
  
  // 이전 속도와 다른 속도가 계산되면 적용
  if (PrevCarSpeed != CarSpeed)
  {
    PrevCarSpeed = CarSpeed;
    Move(MOVE_FORWARD, CarSpeed);
#if DEBUG == 1
    Serial.print("Speed: ");
    Serial.println(CarSpeed);
#endif
  }
}

void Enter()
{
  str.toUpperCase();
  
  if (str.equals("START"))
  {
    GameTime = millis();
    GameStatus = 1;
    
    TrapTime = millis();
    CarStatus = 0;
    CarSpeed = 0;
    PrevCarSpeed = 0;
#if DEBUG == 1
    Serial.println("Race Start!");
#endif
  }
  else if (str.equals("STOP"))
  {
    GameStatus = 0;
    
    CarSpeed = 0;
    Move(MOVE_STOP, CarSpeed);
#if DEBUG == 1
    Serial.println("Race Stop!");
#endif
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
          CarHandle = n;
        }
      }
    }
  }
  
  str = "";
}

void GameRun()
{
  // 라인센서에 300 보다 작은 값이 들어오면 트랩 가동
  for (int i = 0; i < 2; i++)
  {
    if (analogRead(A0 + i) < LINE_TRAP)
    {
      TrapTime = millis();
      CarStatus = 0;
      CarSpeed = 100;
#if DEBUG == 1
      Serial.println("Trap On");
#endif
      break;
    }
  }
  
  if (CarStatus == 0)
  {
    // 트랩 가동중 트랩을 벗어난지 0.3초가 지나면 트랩 해제
    if (millis() - TrapTime >= 300)
    {
      CarStatus = 1;
#if DEBUG == 1
      Serial.println("Trap Off");
#endif
    }
  }
  else if (CarStatus == 1)
  {
    // 트랩 해제된 상태에서 천천히 가속
    int s = (millis() - TrapTime) / 30;
    // 최대 가속 상태이므로 더이상 계산하지 않기 위함
    if (s >= 50)
    {
      CarStatus = 2;
      s = 50;
    }
    
    CarSpeed = 100 + s;
  }
}

void Move(int8_t _dir, int _speed)
{
  // 속도 최대값과 최소값을 넘길시 조정
  if (_speed > 255) _speed = 255;
  else if (_speed < 0) _speed = 0;
  
  // 이동방향
  switch (_dir)
  {
    case MOVE_STOP:
      _dir = 0;
      _speed = 0;
      break;
      
    case MOVE_FORWARD:
      _dir = 0;
      _speed = _speed;
      break;
      
    case MOVE_REVERSE:
      _dir = 1;
      _speed = 255 - _speed;
      break;
  }
  
  digitalWrite(MOTOR_A_DIR, _dir);
  analogWrite(MOTOR_A_PWM, _speed);
}
