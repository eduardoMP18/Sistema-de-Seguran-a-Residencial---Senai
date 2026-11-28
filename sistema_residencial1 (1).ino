
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Keypad.h>

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

String textoLCD[2] = {"", ""};

// PINOS
const int PINO_SERVO = A0;
const int PINO_BUZZER = 3;

const int LED_VERMELHO = A3;
const int LED_AZUL = A2;
const int LED_VERDE = A1;

// ESTADOS
const int DESARMADO = 0;
const int ARMADO = 1;
const int DISPARADO = 2;

int estado = ARMADO;

// SERVO
const int POS_TRANCADO = 90;
const int POS_ABERTO = 0;

Servo servo;

// TECLADO 3X3
const byte LINHAS = 3;
const byte COLUNAS = 3;

char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

byte pinosLinhas[LINHAS] = {13, 12, 11};
byte pinosColunas[COLUNAS] = {10, 9, 8};

Keypad teclado = Keypad(
  makeKeymap(teclas),
  pinosLinhas,
  pinosColunas,
  LINHAS,
  COLUNAS
);

// SENHA
const String SENHA = "1235";
String entrada = "";

// TENTATIVAS
int tentativas = 0;

bool telaDesarmar = false;

// TEMPORIZADOR DA PORTA
unsigned long tempoAbertura = 0;
const unsigned long TEMPO_ABERTO = 5000;

bool portaAberta = false;

// MENSAGENS
String mensagem = "";
unsigned long tempoMensagem = 0;


// SETUP
void setup() {

  lcd.init();
  lcd.backlight();

  servo.attach(PINO_SERVO);

  pinMode(PINO_BUZZER, OUTPUT);

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  // INICIA COM A PORTA TRANCADA
  servo.write(POS_TRANCADO);

  atualizarSaidas();
}


// LOOP
void loop() {

  lerTeclado();

  // FECHA A PORTA E REARMA APOS 5 SEGUNDOS
  if (portaAberta &&
      millis() - tempoAbertura >= TEMPO_ABERTO) {

    portaAberta = false;
    estado = ARMADO;
  }

  // APAGA MENSAGEM TEMPORARIA
  if (mensagem != "" &&
      millis() - tempoMensagem >= 1200) {

    mensagem = "";
  }

  atualizarSaidas();
}


// LEITURA DO TECLADO
void lerTeclado() {

  char tecla = teclado.getKey();

  if (tecla != NO_KEY) {

    mensagem = "";

    if (estado == DISPARADO) {
      telaDesarmar = true;
    }

    entrada += tecla;

    if (entrada.length() == 4) {

      if (entrada == SENHA) {

        tentativas = 0;

        mensagem = "Senha correta";
        tempoMensagem = millis();

        // DESARMA O ALARME
        if (estado == DISPARADO) {

          estado = DESARMADO;
          telaDesarmar = false;
          portaAberta = false;
        }

        // DESATIVA E ABRE A PORTA
        else if (estado == ARMADO) {

          estado = DESARMADO;

          portaAberta = true;
          tempoAbertura = millis();
        }

        // REARMA O SISTEMA
        else {

          estado = ARMADO;
          portaAberta = false;
        }

      }

      else {

        tentativas++;

        mensagem = "Senha errada";
        tempoMensagem = millis();

        // ALARME APOS 3 ERROS
        if (tentativas >= 3) {

          estado = DISPARADO;
          telaDesarmar = false;
          portaAberta = false;
        }
      }

      entrada = "";
    }
  }
}


// ATUALIZA AS SAIDAS
void atualizarSaidas() {

  mostrarStatus();

  if (estado == DESARMADO) {

    if (portaAberta) {

      abrirPorta();
      mostrarCor(0, 255, 0);
    }

    else {

      trancarPorta();
      mostrarCor(255, 0, 0);
    }

    desligarAlarme();
  }

  else if (estado == ARMADO) {

    trancarPorta();
    mostrarCor(255, 0, 0);

    desligarAlarme();
  }

  else if (estado == DISPARADO) {

    trancarPorta();
    mostrarCor(255, 0, 0);

    ligarAlarme();
  }
}


// LCD
void escreverLinha(int linha, String texto) {

  while (texto.length() < 16) {
    texto += " ";
  }

  if (texto != textoLCD[linha]) {

    lcd.setCursor(0, linha);
    lcd.print(texto);

    textoLCD[linha] = texto;
  }
}


// MOSTRA STATUS
void mostrarStatus() {

  String linha1 = "Senha: ";

  for (int i = 0; i < entrada.length(); i++) {
    linha1 += "*";
  }

  if (mensagem != "") {

    escreverLinha(0, mensagem);
  }

  else if (estado == DISPARADO && !telaDesarmar) {

    escreverLinha(0, "CHAMANDO GUARDA");
  }

  else if (estado == DISPARADO && telaDesarmar) {

    escreverLinha(0, "Digite a senha");
  }

  else {

    escreverLinha(0, linha1);
  }

  if (estado == DESARMADO) {

    escreverLinha(1, "DESTRANCADO");
  }

  else if (estado == ARMADO) {

    escreverLinha(1, "TRANCADO");
  }

  else {

    if (telaDesarmar) {
      escreverLinha(1, "Desarmar: " + entrada);
    }

    else {
      escreverLinha(1, "***INVASOR***");
    }
  }
}


// LED RGB
void mostrarCor(int r, int g, int b) {

  analogWrite(LED_VERMELHO, r);
  analogWrite(LED_VERDE, g);
  analogWrite(LED_AZUL, b);
}


// TRAVAR PORTA
void trancarPorta() {

  servo.write(POS_TRANCADO);
}


// ABRIR PORTA
void abrirPorta() {

  servo.write(POS_ABERTO);
}


// LIGAR ALARME
void ligarAlarme() {

  tone(PINO_BUZZER, 500);
}


// DESLIGAR ALARME
void desligarAlarme() {

  noTone(PINO_BUZZER);
}
