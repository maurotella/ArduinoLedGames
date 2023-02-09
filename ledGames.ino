/* Copyright 2022 Mauro Tellaroli */
/*
This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define N                4
#define CALIBRATION_TIME 1500
#define CLEAR_LCD        "                "
#define MSG_GO           "Game Over"
#define HTL_MSG_WIN      "Ottimo"
#define HTL_MSG_LOST     "Nooooo"
#define HTL_CONTROL_TIME 40
#define HTL_START_TIME   1000
#define MG_WATCH_TIME    1000
#define MG_CONTROL_TIME  500
#define MG_MSG_WIN       "Giusto"
#define MG_MSG_LOST      "Sbagliato"
#define MG_MSG_NEXT      "Memorizza"
#define MG_MSG_INPUT     "Indica"
#define AIL_CONTROL_TIME 40
#define AIL_CATCH_MSG    "Acchiappa il led"
#define AIL_MIN_TIME     200
#define AIL_MAX_TIME     800
#define AIL_DURATION     15000

int    leds[] = {8,9,10,11};
int sensors[] = {A3,A2,A1,A0};
int    min[N];

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.print("Calibrazione");
  Serial.begin(9600);
  randomSeed(analogRead(5));
  for (int i=0; i<N; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(sensors[i], INPUT);
    min[i] = init_light(sensors[i]);
  }
  lcd.clear();
}

void loop() {
  MainMenu();
}

// #######################     UTILS    #######################

/**
 *Pulisce la linea del display line da tutti i caratteri
 */
void clearLcdLine(int line) {
  lcd.setCursor(0,line);
  lcd.print(CLEAR_LCD);
}

/**
 * Scansiona per CALIBRATION_TIME millisecondi la fotoresistenza con id e
 * restituisce il valore minimo raggiunto
 * @param id: id della fotoresistenza da calibrare
 * @return  : il valore minimo raggiunto dalla fotoresistenza-10
 */
int init_light(int id) {
  int n = 32, min = 1024, read;
  for (int i=0; i<n; i++) {
    read = analogRead(id);
    if (read<min)
      min = read;
    lcd.setCursor(((sensors[0]-id)*4+i/8),1);
    lcd.print("X");
    delay(CALIBRATION_TIME/n);
  }
  return min-20;
}

/**
 * Entra in modalità input, ovvero resta in attesa di
 * un input da parte dei sensori. L'input sarà valido
 * se la durata sarà di almeno control_time ms. Resistuisce
 * l'id del sensore coperto e validato. Se max_time è
 * diverso da -1, l'attesa di input terminerà dopo 
 * max_time ms.
 *
 * @param control_time: tempo di convalida dell'input
 * @param max_time: tempo massimo di attesa input
 *
 * @return id del sensore coperto e validato,
 *         -1 se eccede max_time
 */
int inputMode(int control_time, int max_time) {
  long start, start_time = millis();
  bool ok;
  while(true) {
    for(int i=0;i<N;i++) { //sensor
      if (analogRead(sensors[i])<min[i]) {
        start = millis();
        ok = true;
        while (millis()<start+control_time) {
          if (analogRead(sensors[i])>min[i]) {
            ok = false;
            break;
          }
        }
        if (ok) {
          //Serial.println(i);
          return i;
        }
      }
    }
    if (max_time!=-1 && millis()>start_time+max_time) {
      return -1;
    }
  }
}

// #######################     MENU     #######################

/**
 * Entra nel menu dove si potrà selezionare tramite sensore
 * qual gioco far partire. Le opzioni sono:
 * 1) HitThatLed
 * 2) Memory Game
 * 3) Acchiappa il led
 */
void MainMenu() {
  int sel;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scegli il gioco:");
  lcd.setCursor(0,1);
  lcd.print("1:HTL 2:MG 3:AIL");
  do {
    sel = inputMode(500,-1);
  } while(sel<0 || sel>2);
  startAnimation();
  switch (sel) {
    case 0:
      HTLstart();
      break;
    case 1:
      MGstart();
      break;
    case 2:
      Serial.println("qui");
      AILstart(AIL_DURATION);
      break;
  }
}

/**
 * Parte un'animazione che mostrerà sulla prima riga del lcd:
 * "Pronti" "Partenza" "Via!"
 */
void startAnimation() {
  int wait = 1500;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PRONTI");
  delay(wait);
  clearLcdLine(0);
  lcd.setCursor(0,0);
  lcd.print("PARTENZA");
  delay(wait);
  clearLcdLine(0);
  lcd.setCursor(0,0);
  lcd.print("VIA!");
  delay(wait);
}

// ####################### HIT THAT LED #######################

/**
 * Fa partire il gioco HitThatLed
 */
void HTLstart() {
  int points = 0;
  int time = HTL_START_TIME;
  while (true) {
    lcd.setCursor(0,1);
    lcd.print("Punteggio: ");
    lcd.print(points);
    if (HTLsingleGame(time,HTL_CONTROL_TIME)) {
      points++;
      time -= time>200? 100:50;
    } else {
      clearLcdLine(0);
      lcd.home();
      lcd.print(MSG_GO);
      delay(3000);
      lcd.home();
      return;
    }
  }
}

/**
 * Fa partire una singola giocata fancendo accendere
 * i led in sequenza ogni wait millisecondi e
 * entrando nella modalità input, ovvero resta in attesa
 * di rilevare un input da uno dei sensori. Appena lo rileva
 * controlla se il sensore coperta corrisponde con il led acceso.
 * Avviene anche un controllo: l'input è valido solo se il sensore
 * viene coperto per almeno confirm_time millisecondi.
 *
 * @param confirm_time: tempo in ms di conferma dell'input
 * 
 * @return true se il giocatore ha vinto il gioco, false altrimenti
 */
bool HTLsingleGame(int wait, int confirm_time) {long start_check, start;
  bool ok;
  int goal = random(0,N);
  lcd.setCursor(0,0);
  lcd.print("Colpisci ");
  lcd.print(goal+1);
  while(true) {
    for (int j=0; j<N; j++) { //led
      digitalWrite(leds[j], HIGH);
      start = millis();
      while (millis()<start+wait) {
        for (int i=0; i<N; i++) { //sensori
          if (analogRead(sensors[i])<min[i]){
            start_check = millis();
            ok = true;
            while(millis()<start_check+confirm_time) { // controllo input
              if (analogRead(sensors[i])>min[i]) {
                ok = false;
                break;
              }
            }
            if (ok) {
              clearLcdLine(0);
              lcd.setCursor(0,0);
              lcd.print(i==j && i==goal ? HTL_MSG_WIN:HTL_MSG_LOST);
              delay(2500);
              digitalWrite(leds[j], LOW);
              return i==j && i==goal;
            }
          }
        }
      }
      digitalWrite(leds[j], LOW);
    }
  }
}

// ####################### MEMORY GAMES #######################

/**
 * Fa partire il gioco Memory Game
 */
void MGstart() {
  int n = 0;
  clearLcdLine(1);
  lcd.setCursor(0,1);
  lcd.print("Punteggio: ");
  do {
    clearLcdLine(0);
    lcd.setCursor(0,0);
    lcd.print(MG_MSG_NEXT);
    lcd.setCursor(11,1);
    lcd.print(n);
    n++;
    delay(1500);
  } while (MGsingleGame(n));
  clearLcdLine(0);
  lcd.setCursor(0,0);
  lcd.print(MSG_GO);
  delay(3500);
}

/**
 * Fa partire una singola giocata di memory game.
 * I passaggi sono i seguenti:
 * 1) Mostrerà tramite i led la sequenza che andrà memorizzata lunga n
 * 2) Resterà in attesa dell'input tramite sensore
 * 3) Se la sequenza indicata è corretta restistuisce true, false altrimenti
 *
 * @param n: lunghezza della sequenza da memorizzare
 *
 * @return true se la sequenza indicata è corretta restistuisce true, 
 *         false altrimenti
 *  
 */
bool MGsingleGame(int n) {
  int sequence[n];
  for (int i=0; i<n; i++) {
    delay(MG_WATCH_TIME/2);
    sequence[i] = random(0,N);
    digitalWrite(leds[sequence[i]], HIGH);
    delay(MG_WATCH_TIME/2);
    digitalWrite(leds[sequence[i]], LOW);    
  }
  for (int i=0; i<n; i++) { // sequence check
    clearLcdLine(0);
    lcd.setCursor(0,0);
    lcd.print(MG_MSG_INPUT);
    if (inputMode(MG_CONTROL_TIME,-1)!=sequence[i]) {
      digitalWrite(leds[sequence[i]], HIGH);
      lcd.setCursor(0,0);
      lcd.print(MG_MSG_LOST);
      delay(1200);
      digitalWrite(leds[sequence[i]], LOW);
      return false;
    }
    digitalWrite(leds[sequence[i]], HIGH);
    lcd.setCursor(0,0);
    lcd.print(MG_MSG_WIN);
    delay(1200);
    digitalWrite(leds[sequence[i]], LOW);
  }
  return true;
}

// ####################### ACCHIAPPA IL LED #######################

/**
 * Fa partire il gioco acchiappa il led.
 * I passaggi sono i seguenti:
 * 1) Accende e spegne casualmente i led
 * 2) Se il led è acceso e viene coperta il corrispettivo
 *    sensore, verrà conteggiato un punto e il led si spegnerà
 * 3) La durata sarà di duration ms
 *
 * @param duration: durata del gioco
 */
void AILstart(long duration) {
  int x, selected=-1, wait=1000, points=0;
  long start_time, start;
  bool talpe[N];
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(AIL_CATCH_MSG);
  lcd.setCursor(0,1);
  lcd.print("Punteggio: 0");
  for (int i=0; i<N; i++) {
    talpe[i] = false;
  }
  start_time = millis();
  while (millis()<start_time+duration) {
    x = random(0,N);
    if (x==selected) {
      x = (x+1)%N;
    }
    if (talpe[x]) {
      digitalWrite(leds[x], LOW);
      talpe[x] = false;
    } else {
      talpe[x] = true;
      digitalWrite(leds[x], HIGH);
    }
    start = millis();
    wait = random(AIL_MIN_TIME,AIL_MAX_TIME);
    //si assicura che si aspetti effettivamente wait
    do {
      selected = inputMode(AIL_CONTROL_TIME, wait);
      //Serial.print(selected);
      if (selected!=-1 && talpe[selected]) {
        talpe[selected] = false;
        digitalWrite(leds[selected], LOW);
        lcd.setCursor(11,1);
        lcd.print(++points);
      }      
    } while(millis()<start+wait);
  }
  clearLcdLine(0);
  lcd.setCursor(0,0);
  lcd.print("FINE");
  for (int i=0; i<N; i++) {
    digitalWrite(leds[i],LOW);
  }
  delay(3000);
}
