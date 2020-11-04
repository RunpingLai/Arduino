#define S1   3
#define S2   A0
#define S3   A1
#define S4   A2
#define S5   A3
#define S6   2
#define ENA  10
#define ENB  9
#define l1   7
#define l2   6
#define r1   5
#define r2   4
#define kp   0.6
#define ki   0
#define kd   15
#define trs  80

//1 全局变量
int minimal = 1100;
int v1, v2, v3, v4, v5, v6;
int lv1, lv2, lv3, lv4, lv5, lv6;
int speedSet = 55;
int threshold = 55;
int lastcorner = 0;
unsigned long  whitetime = 0, startcornertime, endcornertime;

float ave, left, right;
float toleft, toright;
float previous_left = 0, previous_right = 0;
float pl, pr, il = 0, ir = 0, dl, dr;

struct {
  float toleft, toright;
} pidParas;

//2 Setup设置
void setup()
{
  // put your setup code here, to run once:
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(l1, OUTPUT);
  pinMode(l2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);
  pinMode(S6, INPUT);

  //下降沿触发，触发中断0，调用turnRight函数
  //  attachInterrupt(0, turnRight, RISING);
  Serial.begin(9600);
  Read();
  Set(70, 70);
}

// 3 Set设速
inline void Set(int sa, int sb)
{
  analogWrite(ENA, speedSet + (sa > 0 ? sa : -sa));
  analogWrite(ENB, speedSet + (sb > 0 ? sb : -sb));
  digitalWrite(l2, sa > 0);
  digitalWrite(l1, sa <= 0);
  digitalWrite(r2, sb > 0);
  digitalWrite(r1, sb <= 0);
}

// 4 Read读取传感器数值
inline void Read()
{
  lv1 = v1;
  lv2 = v2;
  lv3 = v3;
  lv4 = v4;
  lv5 = v5;
  lv6 = v6;
  v1 = digitalRead(S1);
  v2 = analogRead(S2);
  v3 = analogRead(S3);
  v4 = analogRead(S4);
  v5 = analogRead(S5);
  v6 = digitalRead(S6);
  if (v2 > 350)v2 = 350;
  if (v3 > 350)v4 = 350;
  if (v4 > 350)v4 = 350;
  if (v5 > 350)v5 = 350;


  if (v2 < minimal)minimal = v2;
  if (v3 < minimal)minimal = v3;
  if (v4 < minimal)minimal = v4;
  if (v5 < minimal)minimal = v5;
  minimal += 8;

}

// 5 pid循线
void pidControl()
{

  Read();
  ave = 60;
  left = (v2 + v3) / 2 - ave;
  right = (v4 + v5) / 2 - ave;

  pl = left;
  pr = right;
  il = il + left;
  ir = ir + right;
  if (ir > 100)ir = 100;
  if (ir < -100)ir = -100;
  dl = left - previous_left;
  dr = right - previous_right;

  previous_left = left;
  previous_right = right;

  pidParas.toleft = (kp * pl) + (ki * il) + (kd * dl);
  pidParas.toright = (kp * pr) + (ki * ir) + (kd * dr);
  Set(speedSet - pidParas.toleft, speedSet - pidParas.toright);

}

// 6 Print
void Print()
{
  Serial.print(v1);
  Serial.print(',');
  Serial.print(v2);
  Serial.print(',');
  Serial.print(v3);
  Serial.print(',');
  Serial.print(v4);
  Serial.print(',');
  Serial.print(v5);
  Serial.print(',');
  Serial.println(v6);
}


// 7 loop
void loop()
{
  Read();

  //cornerJudge();

  if ( (v2<minimal) && (v3<minimal) && (v4<minimal) && (v5<minimal) && \
  (millis()-endcornertime<500) && (minimal<60) && (endcornertime!=0) ) {
    Set(-150, -150);
    delay(50);
    if (lastcorner == 6) {
      Set(20, -20);
    }
    else if (lastcorner == 1) {
      Set(-20, 20);
    }
    do {
      Read();
      if (millis() - endcornertime > 2000)
        break;
    } while ( (v3<minimal && v4<minimal) || (millis()-endcornertime<600) );
  }
  else {
    pidControl();
    delay(5);
  }
}

// 8 cornerjudge
void cornerJudge() {
  unsigned long cha = 0;
  if (v1 && !lv1) {
    startcornertime = millis();
  }
  else if (!v1 && lv1) {
    endcornertime = millis();
    cha = endcornertime - startcornertime;
    if ( cha > 15) //正常弯道
      lastcorner = 1;
    else//细黑线
      endcornertime = 0;
  }
  else if (v6 && !lv6) {
    startcornertime = millis();
  }
  else if (!v6 && lv6) {
    endcornertime = millis();
    cha = endcornertime - startcornertime;
    if ( cha > 15) //正常弯道
      lastcorner = 6;
    else
      endcornertime = 0;
  }
}

// 9 外部中断写法
//void turnRight()
//{
//  detachInterrupt(0);
////  cornertime = millis();
//  Set(-100, -100);
//  delay(50);
//  Read();
//
//  Set(20, -20);
//  do {
//    Set(20, -20);
//  } while (millis() - cornertime < 500);
//  //  attachInterrupt(0, turnRight, RISING);
//  Set(70, 70);
//}

// 10 github?
