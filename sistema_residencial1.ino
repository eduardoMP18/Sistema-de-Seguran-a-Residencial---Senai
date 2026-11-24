nesse codigo corrigido completo:
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// constantes para o LCD
const int RS = 2;
const int E = 3;
const int DB4 = 4;
const int DB5 = 5;
const int DB6 = 6;
const int DB7 = 7;
// senha


// constantes para o servo motor
const int SERVO_PIN = A0;

// constantes para o sensor de movimento e LED RGB
const int PIR = A5;
const int ALARM = A4;
const int RED_LED = A3;
const int BLUE_LED = A2;
const int GREEN_LED = A1;

// estados do sistema de segurança
const int UNARMED = 0;
const int ARMED = 1;
const int ALARM_STATUS = 2;

// estado atual do sistema
int systemState = UNARMED;

// sensor de movimento
int pirState;

// LCD
LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);

// posições do servo motor
const int LOCKED_POS = 0;
const int UNLOCKED_POS = 90;

Servo servo;

// teclado
const byte ROWS = 3;
const byte COLS = 3;

const char KEYMAP[ROWS][COLS] =
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
};

// senha do sistema
const String CODE = "1235";

// pinos do teclado
byte ROW_PINS[ROWS] = {13, 12, 11};
byte COL_PINS[COLS] = {10, 9, 8};

// cria o objeto do teclado
Keypad keypad = Keypad(makeKeymap(KEYMAP), ROW_PINS, COL_PINS, ROWS, COLS);

// variáveis de entrada
String input = "";
char key;

// tentativas incorretas
int failedAttempts = 0;

// tela de desativação do alarme
bool disarmScreen = false;

// TEMPORIZADOR DA PORTA
unsigned long tempoAbertura = 0;
const unsigned long TEMPO_ABERTO = 5000;
bool portaAberta = false;


void setup(){

  // configura o LCD
  lcd.begin(16, 2);

  // conecta o servo
  servo.attach(SERVO_PIN);

  // configura os pinos
  pinMode(PIR, INPUT);
  pinMode(ALARM, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  // atualiza as saídas
  updateOutputs();
}


void loop(){

  // verifica o teclado
  readKeypad();

  // verifica o sensor de movimento
  readPIR();

  // verifica se o tempo de abertura terminou
  if (portaAberta && millis() - tempoAbertura >= TEMPO_ABERTO) {
    portaAberta = false;
  }

  // atualiza as saídas
  updateOutputs();
}


/*********************** FUNÇÕES ***********************/

// lê o teclado e controla o sistema
void readKeypad(){

  key = keypad.getKey();

  if (key != NO_KEY) {

    if (systemState == ALARM_STATUS)
    {
      disarmScreen = true;
    }

    input += key;

    // verifica se foram digitados 4 números
    if (input.length() == 4){

      // verifica se a senha está correta
      if (input == CODE) {

        // zera as tentativas incorretas
        failedAttempts = 0;

        // abre a porta por 5 segundos
        if (systemState == UNARMED) {
          portaAberta = true;
          tempoAbertura = millis();
        }

        lcd.setCursor(6, 0);
        lcd.print("CORRECT");
        delay(500);

        // desliga o alarme
        if(systemState == ALARM_STATUS){
          alarmOff();
          systemState = UNARMED;
          disarmScreen = false;
          portaAberta = false;
        }

        // ativa o sistema
        else if(systemState == UNARMED){
          systemState = ARMED;
        }

        // desativa o sistema
        else{
          systemState = UNARMED;
          portaAberta = false;
        }

      }
      else {

        // aumenta as tentativas incorretas
        failedAttempts++;

        lcd.setCursor(6, 0);
        lcd.print("WRONG");

        delay(500);

        // ativa o alarme após 3 erros
        if(failedAttempts >= 3){
          systemState = ALARM_STATUS;
          disarmScreen = false;
          portaAberta = false;
          alarmOn();
        }
      }

      // limpa a entrada
      input = "";
    }
  }
}


// detecta movimento pelo sensor PIR
void readPIR() {

  pirState = digitalRead(PIR);

  // ativa o alarme quando detecta movimento
  if(systemState == ARMED && pirState == HIGH)
  {
    systemState = ALARM_STATUS;
    disarmScreen = false;
    portaAberta = false;
    alarmOn();
  }
}


// atualiza LCD, servo, LED e alarme
void updateOutputs(){

  showStatus();

  if(systemState == UNARMED){

    if(portaAberta){
      unlockDoor();
      displayColor(0, 255, 0); // LED verde
    }
    else{
      lockDoor();
      displayColor(255, 0, 0); // LED vermelho
    }

    alarmOff();

  }
  else if(systemState == ARMED){

    lockDoor();
    displayColor(255, 0, 0); // LED vermelho
    alarmOff();

  }
  else if(systemState == ALARM_STATUS){

    lockDoor();
    displayColor(255, 0, 0); // LED vermelho
    alarmOn();

  }
}


// controla as cores do LED RGB
void displayColor(int r, int g, int b){

  analogWrite(RED_LED, r);
  analogWrite(GREEN_LED, g);
  analogWrite(BLUE_LED, b);
}


// tranca a porta
void lockDoor(){

  servo.write(LOCKED_POS);
}


// destranca a porta
void unlockDoor(){

  servo.write(UNLOCKED_POS);
}


// exibe o estado do sistema no LCD
void showStatus(){

  if(systemState != ALARM_STATUS)
  {
    lcd.setCursor(0,0);
    lcd.print("CODE: ");

    lcd.setCursor(6,0);
    lcd.print("          ");
    lcd.setCursor(6,0);
    lcd.print(input);

    lcd.setCursor(0,1);
  }

  switch(systemState)
  {
    case UNARMED:
      lcd.print("Status: UNARMED");
      break;

    case ARMED:
      lcd.print("Status: ARMED  ");
      break;

    case ALARM_STATUS:

      if (!disarmScreen)
      {
        lcd.setCursor(0,0);
        lcd.print("CALLING SECURITY");
        lcd.setCursor(0,1);
        lcd.print(" ***INTRUDER*** ");
      }
      else{
        lcd.setCursor(0,0);
        lcd.print("Enter Code To   ");

        lcd.setCursor(0,1);
        lcd.print("Disarm: ");
        lcd.print(input);
        lcd.print("        ");
      }

      break;
  }

  lcd.setCursor(6,0);
}


// liga o alarme
void alarmOn(){

  tone(ALARM, 500);
}


// desliga o alarme
void alarmOff(){

  noTone(ALARM);
}
