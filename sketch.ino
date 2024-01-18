#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "planificador.c"

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int ValuePOT = A0;
Servo Serv;
const int MiniServoAngle = 0;
const int MaxiServoAngle = 180;
const int RangeServoAngle = 150;
int alerta = 0;

tTime LastScheduleTime;
#define TICK 50

int angulo = MiniServoAngle;
bool parpadeo = false;

// Tareas
struct Task *tareaReadPotenciometro;
struct Task *tareaActuralizarServo;
struct Task *tareaParpadeo;
struct Task *tareaActualizarLCD;

// Funciones de las tareas
void lecturaPOT(void *parameter) {
  int potValue = analogRead(ValuePOT);
  angulo = map(potValue, 0, 1023, MiniServoAngle, MaxiServoAngle);
}

void escrituraServo(void *parameter) {
  Serv.write(angulo);
}

void colocarParpadeo(void *parameter) {
  parpadeo = !parpadeo;
}

void escribirLCD(void *parameter) {
  lcd.setCursor(0, 0);
  lcd.print("\xFF"); // El caracter "\xFF" es el bloque solido
  int progressBarLength = map(angulo, MiniServoAngle, MaxiServoAngle, 0, 16);
  for (int i = 1; i < progressBarLength; i++) {
    lcd.print("\xFF");
  }
  for (int i = progressBarLength; i < 16; i++) {
    lcd.print(" ");
  }
  lcd.setCursor(0, 1);
  if (parpadeo) {
    lcd.print("\xDB"); // El caracter "\xDB" es el bloque hueco
  } else {
    lcd.print(' ');
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  tTime now = TimeNow() + 1;
  Serv.attach(9);
  Serv.write(MiniServoAngle);

  // Agregar tareas al planificador
  tareaReadPotenciometro = SchedulerAddTask(TimeNow(), 0, 50, lecturaPOT);
  TaskEnable(tareaReadPotenciometro);
  tareaActuralizarServo = SchedulerAddTask(TimeNow(), 0, 50, escrituraServo);
  TaskEnable(tareaActuralizarServo);
  tareaParpadeo = SchedulerAddTask(TimeNow(), 0, 500, colocarParpadeo);
  TaskEnable(tareaParpadeo);
  tareaActualizarLCD = SchedulerAddTask(TimeNow(), 0, 50, escribirLCD);
  TaskEnable(tareaActualizarLCD);
  LastScheduleTime = now;
  lcd.clear();
}

void loop() {
  if (TimePassed(LastScheduleTime) > TICK) {
    SchedulerRun();
    LastScheduleTime = TimeNow();
  }
}