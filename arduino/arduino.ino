#include <LiquidCrystal.h>

const byte builtinLED = 13;
const byte coin_input = 2;

volatile byte state = LOW;
boolean coin_flag = false;

volatile int start =0;
volatile long long int revs = 0;
volatile int counter = 0;

volatile float balance = 0;
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;

const int GREEN1 = 7;
const int RED1 = 8;
const int GREEN2 = 9;
const int RED2 = 10;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void clearLCDrow(int row) {
  lcd.setCursor(0, row);
  for(int i=0; i<20; i++) lcd.print(" ");
}

void blinkGreenIN() {
    digitalWrite(RED1, LOW);
    digitalWrite(GREEN1, HIGH);
    delay(7000);
    digitalWrite(GREEN1, LOW);
    digitalWrite(RED1, HIGH);
}

void blinkGreenOUT() {
    digitalWrite(RED2, LOW);
    digitalWrite(GREEN2, HIGH);
    delay(7000);
    digitalWrite(GREEN2, LOW);
    digitalWrite(RED2, HIGH);
}


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
  lcd.print(String("Balance: ") + balance);
  //lcd.print(String("Added ") + String(nv));
  Serial.print(String("NEW_BALANCE: ") + String(balance)+ ","+ String(nv));
}

void receive_message() {
  if (Serial.available() > 0) {
    // read the incoming byte
    String incomingString = Serial.readString();

    if (String(incomingString) == "MESSAGE: FINISHED PAYING") {
      String m = incomingString;
      m.replace("MESSAGE: ", "");
      clearLCDrow(1);
      lcd.setCursor(0, 1);
      lcd.print(m);
      blinkGreenOUT();
      delay(500);
      clearLCDrow(0);
      clearLCDrow(1);
      balance = 0;
      digitalWrite(RED1, HIGH);
      digitalWrite(RED2, HIGH);
      lcd.setCursor(0, 0);
      lcd.print(String("Balance: ") + balance);
    } else if (String(incomingString).startsWith("MESSAGE: ")) {
      String m = incomingString;
      m.replace("MESSAGE: ", "");
      lcd.setCursor(0, 1);
      lcd.print(m);
    } else if (String(incomingString) == "NEW_SESSION") {
      blinkGreenIN();
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(coin_input, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(coin_input), CH926, FALLING);
  start = millis();

  pinMode(GREEN1, OUTPUT); // GREEN LED GREEN WIRE
  pinMode(GREEN2, OUTPUT); // GREEN LED NORMAL WIRE

  pinMode(RED1, OUTPUT); // RED LED GREEN WIRE
  pinMode(RED2, OUTPUT); // RED LED NORMAL WIRE

  digitalWrite(RED1, HIGH);
  digitalWrite(RED2, HIGH);
  lcd.begin(16, 2);
  lcd.print(String("Balance: ") + balance);
}

void loop() {
  receive_message();
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
    }

    revs = (fin - start);
  } else {
    long fin = millis();
    long a = ((fin - start) -  revs);
    if(a > 1000 && counter > 1) {
      add_balance(counter);
      counter = 1;
    }
  }
}

void CH926() {
  coin_flag = true;
}
