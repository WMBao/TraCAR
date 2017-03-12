#define AHEAD 105
#define RIGHT 130
#define LEFT 80
#define BRIGHT 130
#define BLEFT 80
#define ENER 200
#define DISEN 10
#define DELAY 50
int i, j, val;
char a[3];
boolean display, ahead;
#include <Servo.h>
Servo myservo;

void setup(){
  Serial.begin(9600);
  myservo.attach(3);
  pinMode(11,OUTPUT);//左电机阳极
  pinMode(9,OUTPUT);//左电机阴极
  pinMode(6,OUTPUT);//右电机阳极
  pinMode(5,OUTPUT);//右电机阴极
  analogWrite(11, 0);
  analogWrite(9, 0);
  analogWrite(6, 0);
  analogWrite(5, 0);
  myservo.write(AHEAD);
  //val = AHEAD;
}

void loop(){
  if (Serial.available()){        //如果有数据输入.....
    delay(30);                    //等待30毫秒让所有输入数据从串口传输完毕.....
    if (Serial.available() <=3){ //如果输入数据位数'<=3'.....
      while (Serial.available()){ //开始读取数据直到[串口输入缓存被清空]
        a[i++] = Serial.read();   //读取数据到[数组"a"]
      }
      display = 1;                    //数据读取完毕以后'打开'显示输出开关
    }
    else {                        //如果输入数据位数'>3'.....
      Serial.flush();             //刷新串口输入缓存
    }
  }
//========================判断及修正输入数据位数模块======================
if (display)                   
  {
   if (!a[2]){ //如果输入数据为两位数(最后一位空)
   if (!a[1]){ //如果输入数据为一位数(最后两位空)
   a[2] = a[0];
   a[1] = 48;
   a[0] = 48;
   }
   else { 
   a[2] = a[1];
   a[1] = a[0];
   a[0] = 48;
   }
   }
//===========================判断是否是前进指令===========================
  if(a[2] == 'F'){
    myservo.write(AHEAD);
    delay(DELAY);
    analogWrite(11, ENER);//将左边电机的正极置为高，占空比为ENER
    analogWrite(9,0);//保持左边电机的负极为低
    analogWrite(6, ENER);//将右边电机的正极置为高，占空比为ENER
    analogWrite(5,0);//保持右边电机的负极为低
    //display = 0;
    //Serial.flush();
  }
  else if(a[2] == 'B'){
    myservo.write(AHEAD);
    delay(DELAY);
    analogWrite(11, 0);
    analogWrite(9,ENER);
    analogWrite(6, 0);
    analogWrite(5,ENER);
  }
  else if(a[2] == 'S'){
    myservo.write(AHEAD);
    delay(DELAY);
    analogWrite(11, 0);
    analogWrite(9,0);
    analogWrite(6, 0);
    analogWrite(5,0);
    //display = 0;
    //Serial.flush();
  }
  else if(a[2] == 'L'){
    myservo.write(LEFT);
    delay(DELAY);
    analogWrite(11,ENER);
    analogWrite(9,0);
    analogWrite(6,ENER);
    analogWrite(5,0);
  }
  else if(a[2] == 'R'){
    myservo.write(RIGHT);
    delay(DELAY);
    analogWrite(11,ENER);
    analogWrite(9,0);
    analogWrite(6,ENER);
    analogWrite(5,0);
  }
  else if(a[2] == 'Z'){
    myservo.write(BLEFT);
    delay(DELAY);
    analogWrite(11,0);
    analogWrite(9,ENER);
    analogWrite(6,0);
    analogWrite(5,ENER);
  }
  else if(a[2] == 'X'){
    myservo.write(BRIGHT);
    delay(DELAY);
    analogWrite(11,0);
    analogWrite(9,ENER);
    analogWrite(6,0);
    analogWrite(5,ENER);
  }
    
//==============转换变量类型后输出给舵机且通过串口返回结果值==============
  else{
  for (i=0;i<=3;i++){ //变量类型:char to int (48为0的ASCII)
  a[i] -= '0';
  }
  val = 100*a[0] + 10*a[1] + a[2];
  myservo.write(val);
  if (val > AHEAD){
    analogWrite(11,ENER);
    analogWrite(9,0);
    analogWrite(6,DISEN);
    analogWrite(5,0);
  }
  else {
    analogWrite(11,DISEN);
    analogWrite(9,0);
    analogWrite(6,ENER);
    analogWrite(5,0);
  }
  }
  
  display = 0;                  //显示完毕'关闭'显示输出开关
  Serial.flush();               //刷新串口输入缓存
  for (i = 0; i <= 3; i++)      //重置[数组"a"]
  {
    a[i] = 0;
  }
  i = 0;                        //重置"计数变量"[i]
//  val = 0;
}
}


