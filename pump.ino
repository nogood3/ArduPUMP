#include <LiquidCrystal.h>  // Лобавляем необходимую библиотеку
LiquidCrystal lcd(19, 18, 17, 16, 15, 14); // (RS, E, DB4, DB5, DB6, DB7) 
int lcd_1[6]={14, 15, 16, 17, 18, 19}; //A0-A5

unsigned long timer=0;
unsigned long timer_1 = 0;
volatile int res=0;
volatile int pump=12; //насос
volatile int res_l=0;
int button=11; //кнопка
unsigned long emerg_t=400000;
unsigned long pump_t[4]={0,0,0,0};
int level[4] = {10,9,8,7};//уровень
int pump_times=0;
int disp_times=0; // пусков за последние сутки
boolean pump_onoff=0;
boolean autorun=0;
  int level_calc=0;
int emerg=0;
int stop_b=6; //кнопка стоп
int stop_task=0;
boolean bt=0;
double timer_step=360000;
double inert=0;

boolean block=0;
int block_b=5;

void setup() {
  // put your setup code here, to run once:

attachInterrupt(0, blink_d, RISING); //Д2 пин манометра
pinMode(2, INPUT_PULLUP);
Serial.begin(9600);
pinMode(button, INPUT_PULLUP);
pinMode(stop_b, INPUT_PULLUP);
pinMode(block_b, INPUT_PULLUP);
pinMode(pump, OUTPUT);
 for (int i = 0; i < 4; i++) {
    pinMode(level[i], INPUT);}
for (int i = 0; i < 6; i++) {
    pinMode(lcd_1[i], OUTPUT);
    }
    
lcd.begin(16, 2);
}

void loop() {
  // put your main code here, to run repeatedly:
level_calc=0;
for (int i=0 ;i<4;i++) {                         //уровень воды
  level_calc = digitalRead(level[i])+level_calc;
  }
  switch (level_calc) {
    case 1: level_calc = 25; break;
    case 2: level_calc = 75; break;
    case 3: level_calc = 110; break;
    case 4: level_calc = 180; break;
    default : break;
    }

  if ( millis() > 500){
    if (digitalRead(stop_b) == LOW) { //STOP кнопка
      stop_task =1;
      digitalWrite(pump,LOW);
      bt=0;
      timer_step=360000;
      }
if (level_calc <1 && emerg == 0 && pump_times < 4 && digitalRead(pump) == LOW && level_calc< 180 && stop_task ==0) { pump_on(); //Мало воды , не включён насос и  не сработала аварийка
                                                  Serial.println("PUMP ON auto");
                                                  autorun=1;
                                                   }
if (digitalRead(button)==LOW /*&& emerg_t > 303000 */&& digitalRead(pump) == LOW ) {pump_on (); //НАЖАТА КНОПКА , не включён насос и  не сработала аварийка
                                                  Serial.println("PUMP ON"); 
                                                  stop_task = 0;
                                                  bt=1;
                                                  }
if (millis()-timer_1 >timer_step && digitalRead(pump) == HIGH) { // кончился таймер
                                Serial.println("end of timer");
                                digitalWrite(pump,LOW);
                                bt=0;
                                autorun=0;
                                timer_step=360000;}
                                
if (level_calc>= 180){ 
        digitalWrite(pump, LOW);  
        autorun=0; 
         bt=0; 
         timer_step=360000;}

                                  
if (res_l!=res  && block == 0) { // сработал датчик холла и время после пуска больше 5 сек
                                           Serial.println("Drill IS empty"); 
                                            digitalWrite(pump,LOW); 
                                            emerg_t=millis(); 
                                            bt=0;
                                            res_l=res;
                                            emerg=1;
                                            autorun=0;
                                            timer_step=360000;
                                           }

if (millis()-timer >500) {if (res_l!=res) Serial.println("drill empty"); // отладка
                          timer=millis(); 
                          if (digitalRead(pump) == HIGH) Serial.println("PUMPING");
                          Serial.print("LEVEL  ");
                          Serial.println(level_calc);   
                          Serial.print("times run  ");
                          times();
                          Serial.println(disp_times);
                          if (digitalRead(block_b)== LOW) {block=!block;} //Блокировка


                          if (digitalRead(button)==LOW &&  digitalRead(pump) == HIGH && millis()-timer_1 > 3000) { timer_step+=360000; }
                          display_out();
                          }

 if (millis() - emerg_t > 28800000) emerg=0; //снятие блокировки через 8 часов
 if (millis()-timer_1>86400000) pump_times=0; //обнуление включений через 24 часа
 if (millis() - timer_1 > 2400000) block =0; //Обнуление блокировки 
  }
  else res_l=res;
}

void blink_d() {
    if (block==1 || digitalRead(pump) == LOW) return;
    res_l=res;
   res++;
   if (digitalRead(pump) == HIGH) {
    if ( millis()-timer_1 < 5000) {res--;}
    }
}

void times(){
  disp_times=0;
  for (int i=0; i<4; i++) {
    if (pump_t[i] < millis()-86400000 && pump_t[i]>0 ) {
      disp_times++;
      } 
    }
  }

void pump_on (){
  digitalWrite(pump, HIGH);
  pump_times++;
  pump_t[0]=pump_t[1];
  pump_t[1]=pump_t[2];
  pump_t[2]=pump_t[3];
  pump_t[3]=millis();
  timer_1=millis();
  pump_onoff=1;
  }

void display_out(){
lcd.setCursor(0, 0);  
lcd.print("LEVEL");
lcd.setCursor(6, 0);  
lcd.print("   ");
lcd.setCursor(6, 0);  
lcd.print(level_calc);
lcd.setCursor(0, 1);
lcd.print("Times");
lcd.setCursor(6, 1);
lcd.print(disp_times);
lcd.print("/");
lcd.print(pump_times);
lcd.setCursor(10, 1);
if (digitalRead(pump) == HIGH) lcd.print("ON ");
else lcd.print("       ");
lcd.setCursor(9, 0);
lcd.print("      ");
lcd.setCursor(9, 0);
if (emerg==1 && digitalRead(pump) ==LOW) {lcd.print("EMPTY"); if(300000 > (millis() - emerg_t)) {lcd.setCursor(10, 1); lcd.print((300000 - (millis() - emerg_t))/1000);}}
if (autorun ==1 && digitalRead(pump) == HIGH && bt==0) lcd.print("auto");
if (bt==1 && digitalRead(pump)==HIGH && block ==0) {lcd.print("BUTTON");}
if (block ==1) {lcd.print("BLOCK");}



lcd.setCursor(13, 1);
lcd.print("      ");
lcd.setCursor(13, 1);
if (digitalRead(pump) == HIGH) { lcd.print((timer_step-(millis()-timer_1)  )/60000);}
}
