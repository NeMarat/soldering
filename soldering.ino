#include "segment_7.h"
#include "PID_v1.h"

int count_delay = 100; //частота опроса датчика паяльника
int count_delay_2 = 1000; //частота считывания переменного резистора, задающего температуру
long actual_count_delay = 0;
long actual_count_delay_2 = 0;
volatile int increment = 0;   //значение на сегментах
byte showDataKind = 0;
//--------------------- переменные паяльника -----------------------------
int nagr = 11; // пин вывода нагревательного элемента(через транзистор)
byte regul = A1; //пин переменного резистора для задания температуры
int tin = 0; // Пин Датчика температуры IN Analog через LM358N
int tdat = 25; // переменная считанной температуры
//int p_tdat = tdat; //предыдущее значение температуры
//byte p_count = 0; //счетчик пропусков значений
volatile int ustt =  0; // Выставленная температура по умолчанию (+ увеличение и уменьшение при нажатии кнопок)
int mintemp = 100; // Минимальная температура
int maxtemp = 360; // Максимальная температура 
int nshim = 0; // Начальное значение шим для нагрузки 
double Setpoint=0.0, Input=0.0, Output=0.0; // переменные pid

double Kp=3.8, Ki=0.00001, Kd=0.9; //настройки pid

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int getNeedTemp () {
  int reg_val = analogRead(regul)>>2;
  reg_val = reg_val + analogRead(regul)>>2;
  reg_val = reg_val / 2;
  reg_val = map(reg_val, 0, 84, mintemp, maxtemp);
  return(reg_val);
}

void getFactTemp () {
  tdat = analogRead(tin)>>2; // Считать состояние датчика температуры и присвоить tdat
  tdat = tdat + analogRead(tin)>>2;
  tdat = tdat / 2;
  tdat =map(tdat,0,80,25,450); // калибровка п умолчанию 0,430,25,310 90
  /*
  if (p_count < 3) {
    if (tdat - 30 < p_tdat) { tdat = p_tdat; p_count++; } //остывание за один такт на 30 градусов считаю невероятным и отбрасываю
     else { if (tdat + 30 > p_tdat) { tdat = p_tdat; p_count++; } //как впрочем и нагрев
      else { p_tdat = tdat; p_count = 0; }}
  }
  */
}


void setup(){  
  delay(1000); //в выдаче на сегменты используются ttl пины, поэтому для легкой прошивки введена задержка
  pinMode(nagr, OUTPUT);     // Порт нагрузки (паяльника) настраиваем на выход
  pinMode(tin, INPUT);
  getFactTemp ();
  Input = tdat;
  Setpoint = getNeedTemp();
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(count_delay);
  myPID.SetOutputLimits(0, 200); // двже при шим=230 транзистор очень сильно греется, лучше не спешить 
  initSegment();
  analogWrite(nagr, 0);     //Вывод шим в нагрузку паяльника (выводим 0 - старт с выключенным паяльником - пока не определим состояние температуры)
  delay(count_delay/3);
}

void loop() {

  showSegment(increment);   // Вывести значение переменной на экран (LED)
  if(millis() - actual_count_delay > count_delay) {  
    actual_count_delay = millis();
    getFactTemp();
    Input = tdat;
  }
  if(millis() - actual_count_delay_2 > count_delay_2) {  
    actual_count_delay_2 = millis();
    ustt = getNeedTemp();
    Setpoint = ustt;
    if (showDataKind == 0) { increment = ustt; showDataKind = 1; } //попеременный показ установленной и фактической температуры
      else { increment = tdat;  showDataKind = 0; }
  }
  myPID.Compute();
  nshim = Output;
  analogWrite(nagr, nshim);
//analogWrite(nagr, 220);
}

