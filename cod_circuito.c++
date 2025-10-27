#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <DHT.h>  // Incluir a biblioteca DHT

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definir os pinos dos sensores
const int pHPIN = A0;
const int tempPIN = A1;
const int lumiPIN = A2;

// Sensor DHT11
#define DHTPIN A3         // Pino onde o DHT11 está conectado
#define DHTTYPE DHT11    // Definir tipo de sensor (DHT11)
DHT dht(DHTPIN, DHTTYPE); // Inicializar o sensor DHT

// Atuadores
const int buzzerPin = 8;
const int relePin = 7;  // Relé da bomba

// LED RGB
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

const int botaoSilenciarPin = 6;

bool buzzerSilenciado = false;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Iniciado");
  delay(2000);
  lcd.clear();

  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  pinMode(relePin, OUTPUT);  // Configura relé como saída
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(botaoSilenciarPin, INPUT_PULLUP);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(relePin, LOW);  // Bomba ligada sempre

  dht.begin();  // Inicializar o sensor DHT11
}

void loop() {
  // Leitura dos sensores
  int valorSensorPH = analogRead(pHPIN);
  float voltagemPH = valorSensorPH * (4.0 / 1023.0);
  float pH = 7 + (voltagemPH - 2.5);

  int valorSensorTemp = analogRead(tempPIN);
  float voltagemTemp = valorSensorTemp * (5.0 / 1023.0);
  float temperatura = voltagemTemp * 7;

  int valorLuz = analogRead(lumiPIN);
  float porcentagemLuz = (valorLuz / 1023.0) * 100;

  // Leitura do DHT11 (Temperatura e Umidade)
  float tempDHT = dht.readTemperature();  // Temperatura em °C
  float umidadeDHT = dht.readHumidity();  // Umidade em %

  // Exibir no Serial Monitor
  Serial.print("pH: ");
  Serial.println(pH);
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");
  Serial.print("Luminosidade: ");
  Serial.print(porcentagemLuz);
  Serial.println("%");

  Serial.print("Temperatura DHT11: ");
  Serial.print(tempDHT);
  Serial.println(" C");
  Serial.print("Umidade DHT11: ");
  Serial.print(umidadeDHT);
  Serial.println("%");

  // Mostrar no LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(pH, 1);
  lcd.print(" T:");
  lcd.print(temperatura, 1);

  lcd.setCursor(0, 1);
  lcd.print("Luz:");
  lcd.print(porcentagemLuz, 0);
  lcd.print("%");

  delay(3000);

  // Verificação de estado
  bool phCritico = (pH < 5.5 || pH > 9.0);
  bool tempCritica = (temperatura < 15 || temperatura > 30);
  bool luzCritica = (porcentagemLuz < 20); // Menos de 20% = crítico

  bool phAlerta = (pH < 5.8 || pH > 8.0);
  bool tempAlerta = (temperatura < 20 || temperatura > 25);
  bool luzAlerta = (porcentagemLuz < 30 || porcentagemLuz > 80);

  bool estadoCritico = phCritico || tempCritica || luzCritica;
  bool estadoAlerta = (!estadoCritico) && (phAlerta || tempAlerta || luzAlerta);

  int estadoBotao = digitalRead(botaoSilenciarPin);

  // Se botão for pressionado enquanto está crítico, silencia o buzzer
  if (estadoBotao == LOW && estadoCritico) {
    buzzerSilenciado = true;
  }

  if (estadoCritico) {
    // Pisca vermelho
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    digitalWrite(buzzerPin, HIGH);
    delay(200);

    // Pisca azul
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
    digitalWrite(buzzerPin, LOW);
    delay(200);

    // Pisca verde
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
    digitalWrite(buzzerPin, HIGH);
    delay(200);

    // Pisca roxo (vermelho + azul)
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
    digitalWrite(buzzerPin, LOW);
    delay(200);

    // Vermelho aceso
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);

    if (!buzzerSilenciado) {
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(buzzerPin, LOW);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!! CRITICO !!");
    lcd.setCursor(0, 1);
    lcd.print("Press. p/ silenciar");

  } else if (estadoAlerta) {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);

    digitalWrite(buzzerPin, LOW);
    buzzerSilenciado = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Atencao:");
    lcd.setCursor(0, 1);
    lcd.print("Verificar condicoes");

  } else {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);

    digitalWrite(buzzerPin, LOW);
    buzzerSilenciado = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tudo OK :)");
  }

  delay(3000);
}
