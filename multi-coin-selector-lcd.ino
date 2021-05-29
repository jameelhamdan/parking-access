#include <LiquidCrystal.h>

// BRAA HAMADDAN


const byte builtinLED = 13;
const byte coin_input = 2;

volatile byte state = LOW;
boolean coin_flag = false;

volatile int start =0;
volatile long long int revs = 0;
volatile int counter = 0;

volatile float balance = 0;
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void add_balance(int val){
  float nv = 0;
  if (val == 2) {
    nv += 0.5;
  } else if (val == 3) {
    nv += 1;
  } else if (val == 4) {
    nv += 2;
  } else if (val == 5) {
    nv += 5;
  } else if (val == 6) {
    nv += 10;
  }
  balance += nv;
  lcd.begin(16, 2);
  lcd.print(balance);
  lcd.setCursor(0, 1);
  lcd.print(String("Added ") + String(nv));
}

void setup() {
  Serial.begin(9600);
  pinMode(coin_input, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(coin_input), CH926, FALLING);
  start = millis();  


  lcd.begin(16, 2);
  lcd.print(balance);
}

void loop() {

  if (coin_flag == true )
  {
//    delay (1);
    state = !state;
    digitalWrite(builtinLED, state);   // check if the bulit in LED changes

    coin_flag = false ;     // remove the flag
    
    long fin = millis();
    long a = ((fin - start) -  revs);
    if(0 < a && a < 500) {
      counter++;
    } else {
      counter = 1;
      lcd.print("");
    }
 
    revs = (fin - start);    
    Serial.println(counter);
  } else {
    long fin = millis();
    long a = ((fin - start) -  revs);
    if(a > 1000 && counter > 1) {
      Serial.print("Counted ");
      Serial.print(counter);
      Serial.println(" pooolses");
      add_balance(counter);
      counter = 1;
    } 
  }
}

void CH926() {
  coin_flag = true;
}
