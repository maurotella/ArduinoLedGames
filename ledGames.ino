#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define N 4
#define CALIBRATION_TIME 1000
#define CLEAR_LCD "                "

int leds[] = {8,9,10,11};
int sensors[] = {A3,A2,A1,A0};
int min[N];

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.print("Calibrazione");
  Serial.begin(9600);
  for (int i=0; i<N; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(sensors[i], INPUT);
    min[i] = init_light(sensors[i]);
  }
  lcd.clear();
}

void loop() {
  lcd.print("Copri la 1");
  while(analogRead(sensors[0])>min[0]) {}
  lcd.clear();
  lcd.print("INIZIAMO");
  delay(1000);
  while(analogRead(sensors[0])<min[0]) {}
  delay(300);
  HTLhitThat();
}

// UTILS

/*
Pulisce la linea del display line da tutti i caratteri
*/
void clearLcdLine(int line) {
  lcd.setCursor(0,line);
  lcd.print(CLEAR_LCD);
}

/**
 * Scansiona per CALIBRATION_TIME millisecondi la fotoresistenza con id e
 * restituisce il valore minimo raggiunto
 * @param id: id della fotoresistenza da calibrare
 * @return  : il valore minimo raggiunto dalla fotoresistenza
 */
int  init_light(int id) {
  int n = 32, min = 1024, read;
  for (int i=0; i<n; i++) {
    read = analogRead(id);
    if (read<min)
      min = read;
    lcd.setCursor(((sensors[0]-id)*4+i/8),1);
    lcd.print("X");
    delay(CALIBRATION_TIME/n);
  }
  return min-10;
}

//HIT THAT LED

/**
 * Fa partire il gioco
 */
void HTLhitThat() {
  int points = 0;
  int time = 1000;
  while (true) {
    lcd.setCursor(0,1);
    lcd.print("Punti: ");
    lcd.print(points);
    if (HTLcheckHit(time)) {
      points++;
      time-= time>200? 100:50 ;
      Serial.println(time);
    } else {
      clearLcdLine(0);
      lcd.home();
      lcd.print("GAME OVER");
      delay(3000);
      lcd.home();
      return;
    }
  }
}

/**
 * Entra in modalità colpisci:
 * il gioco proporrà di colpire una fotoresistenza
 * in maniera casuale facendo variare i led ogni 
 * wait millisecondi.
 * @param wait: tempo di variazione tra un led e l'altro
 * @return    : true se viene coperta la fotoresistenza corretta
 *              false altrimenti
 */
bool HTLcheckHit(int wait) {
  int goal = random(0,N);
  lcd.setCursor(0,0);
  lcd.print("Colpisci ");
  lcd.print(goal+1);
  while ( true ) {
    for (int j=0; j<N; j++) {
      digitalWrite(leds[j],HIGH);
      long init = millis();
      while(millis()<init+wait) {
        for (int i=0; i<N; i++) {
          if (analogRead(sensors[i])<min[i]) {
            clearLcdLine(0);
            lcd.setCursor(0,0);
            lcd.print(i==j&&i==goal?"Ottimo":"Che scarso");
            delay(2500);
            digitalWrite(leds[j],LOW);
            return i==j&&i==goal;
          }
        }
      }
      digitalWrite(leds[j],LOW);
    }
  }
}