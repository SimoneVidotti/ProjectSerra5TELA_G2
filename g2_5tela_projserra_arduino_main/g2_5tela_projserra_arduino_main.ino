// G2 5TELA "Smart Serra" 2025
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Wire.h>
#include <MAX44009.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

/*
  Codice sorgente Arduino per il progetto serra.

  Il sistema monitora:
  - Parametri del terreno: umidità (4 zone), temperatura
  - Parametri ambientali: temperatura, umidità, irraggiamento luminoso

  Attuatori controllati:
  - 4 pompe per irrigazione a goccia (una per zona) (Usate realmente: 3)
  - 2 ventole per il ricambio d'aria

  I dati sono visualizzati su LCD locale e inviati via Ethernet a Node-RED
  per il monitoraggio e controllo remoto.
  Modificare i pin e gli indirizzi di rete secondo la configurazione effettiva.

  Pin LCD e sensore luminosità -> sda scl
*/

// Variabili di lettura
int LetturaTempTerraA;
int LetturaTempTerraB;
int LetturaTempTerraC;
int LetturaTempTerraD;

int LetturaHumTerraA;
int LetturaHumTerraB;
int LetturaHumTerraC;
int LetturaHumTerraD;

int LetturaTempAmb;
int LetturaHumAmb;

int LetturaLuce;

String StrJson;

// Numero cicli loop
int N_LOOP = 0;

// Minuti attesa accensione pompe per collaudo versione Beta
int CtrlMinutiPompe = 60;

// Pin Sensori umidita terra
#define PinTerraHumidityA A12
#define PinTerraHumidityB A13
#define PinTerraHumidityC A14
#define PinTerraHumidityD A15

// Pin x4 con indirizzamento, temperatura terra
#define ONE_WIRE_BUS 46   // Specify the digital pin connected to the DS18B20 sensors

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20s(&oneWire);

// Array per salvare gli indirizzi dei sensori
DeviceAddress DS18B20Addresses[4];

// Pin ventole
int PinVentoleA = 48;
int PinVentoleB = 49;

// Pin DHT22, temperatura e umidita ambiente, init
#define DHTPIN 7
#define DHTTYPE DHT22  // Tipo di sensore (DHT22)

DHT dht(DHTPIN, DHTTYPE); // Inizializza il sensore

// Dichiarazione Pompe , pin da modificare
#define PinPompaA 4444 // Pin collegato al relè o al driver della pompa
#define PinPompaB 4444 // Pin collegato al relè o al driver della pompa
#define PinPompaC 4444 // Pin collegato al relè o al driver della pompa
// #define PinPompaD X; // Pin collegato al relè o al driver della pompa, non esiste nel nostro progetto

// Durata ON e OFF in millisecondi (regolabili per ottenere un gocciolamento)
const int POMPA_T_ON = 100;   // pompa accesa per 100ms (breve impulso)
const int POMPA_T_OFF = 300; // TEMPO pompa spenta
const int POMPA_N_CICLI = 5; // Modificabile, numero gocce

// Calibrazione per ciascun sensore (sostituisci con i tuoi valori)
const int DRY_VALUE = 620;  // Valore in aria secca
const int WET_VALUE = 310;  // Valore in acqua
const int HIGH_TEMP = 28;   // Valore di alta temperatura

// Controllo invio dati json
int CtrlMinutiInvioDati = 60; // tempo attesa invio dati

// Indirizzo MAC della shield Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP statico per Arduino
IPAddress ip(10, 3, 0, 177);
IPAddress gateway(10, 3, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// IP e porta del server (Raspberry Pi con Node-RED)
IPAddress server(10, 3, 0, 197);
int serverPort = 9000;

EthernetClient client;

LiquidCrystal_I2C lcd(0x27, 16, 2); // Inizializzazione lcd

MAX44009 light; // inizializzazione sensore luminoso

// Area dichiarazione Funzioni
void StampaLCD(String s1, String s2) { // stampa lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(s1);
  lcd.setCursor(0, 1);
  lcd.print(s2);
}

void printAddress(DeviceAddress deviceAddress) { // Funzione per stampare l'indirizzo di un sensore
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}

// Funzione per leggere un singolo sensore di umidita terra
int readMoisture(int sensorPin) {
  int value = analogRead(sensorPin);
  int moisturePercent = map(value, DRY_VALUE, WET_VALUE, 0, 100);
  return moisturePercent; //constrain(moisturePercent, 0, 100);  // Limita tra 0% e 100%
}

// Aggiorna lettura Sensori, tutti
void ReadSensori() {
  // update lettura sensori temperatura terra
  DS18B20s.requestTemperatures();

  LetturaTempTerraA = DS18B20s.getTempC(DS18B20Addresses[0]);
  LetturaTempTerraB = DS18B20s.getTempC(DS18B20Addresses[1]);
  LetturaTempTerraC = DS18B20s.getTempC(DS18B20Addresses[2]);
  LetturaTempTerraD = DS18B20s.getTempC(DS18B20Addresses[3]);

  // update lettura sensori umidita
  LetturaHumTerraA = readMoisture(PinTerraHumidityA);
  LetturaHumTerraB = readMoisture(PinTerraHumidityB);
  LetturaHumTerraC = readMoisture(PinTerraHumidityC);
  LetturaHumTerraD = readMoisture(PinTerraHumidityD);

  // update lettura sensori temperatura ambiente
  LetturaTempAmb = dht.readTemperature(); // Leggi temperatura (°C)

  // update lettura sensori umidita ambiente
  LetturaHumAmb = dht.readHumidity();     // Leggi umidità (%)

  // Lettura Sensore Luminosita
  LetturaLuce = light.get_lux();
}

void CtrlVentole() {
  if (LetturaTempAmb >= HIGH_TEMP) {
    digitalWrite(PinVentoleA, HIGH);
    digitalWrite(PinVentoleB, HIGH);
  } else {
    digitalWrite(PinVentoleA, LOW);
    digitalWrite(PinVentoleB, LOW);
  }
}

void CtrlPompe() {
  if (N_LOOP == CtrlMinutiPompe) { // controllo per gocciolamento
    for (int i=0; i <= POMPA_N_CICLI; i++) {
      digitalWrite(PinPompaA, LOW);  // Accende la pompa (logica inversa)
      delay(POMPA_T_ON);
      digitalWrite(PinPompaB, LOW);  // Accende la pompa (logica inversa)
      delay(POMPA_T_ON);
      digitalWrite(PinPompaC, LOW);  // Accende la pompa (logica inversa)
      delay(POMPA_T_ON);
      // digitalWrite(PinPompaD, LOW);  // Accende la pompa (logica inversa), D non esiste
      // delay(POMPA_T_ON);

      digitalWrite(PinPompaA, HIGH); // Spegne la pompa
      delay(POMPA_T_OFF);
      digitalWrite(PinPompaB, HIGH); // Spegne la pompa
      delay(POMPA_T_OFF);
      digitalWrite(PinPompaC, HIGH); // Spegne la pompa
      delay(POMPA_T_OFF);
      // digitalWrite(PinPompaD, HIGH); // Spegne la pompa
      // delay(POMPA_T_OFF);, D non esiste
    }

    digitalWrite(PinPompaA, HIGH); // Spegne la pompa
    digitalWrite(PinPompaB, HIGH); // Spegne la pompa
    digitalWrite(PinPompaC, HIGH); // Spegne la pompa
    // digitalWrite(PinPompaD, HIGH); // Spegne la pompa, D non esiste

    N_LOOP = 0;
  } else {
    digitalWrite(PinPompaA, HIGH); // Spegne la pompa
    digitalWrite(PinPompaB, HIGH); // Spegne la pompa
    digitalWrite(PinPompaC, HIGH); // Spegne la pompa
    // D non esiste digitalWrite(PinPompaD, HIGH); // Spegne la pompa
  }
}

void StampaLCD() {
  StampaLCD(("Temp A: " + String(LetturaTempTerraA)), ("Hum A: " + String(LetturaHumTerraA)));
  delay(1000);

  StampaLCD(("Temp B: " + String(LetturaTempTerraB)), ("Hum B: " + String(LetturaHumTerraB)));
  delay(1000);

  StampaLCD(("Temp C: " + String(LetturaTempTerraC)), ("Hum C: " + String(LetturaHumTerraC)));
  delay(1000);

  StampaLCD(("Temp D: " + String(LetturaTempTerraD)), ("Hum D: " + String(LetturaHumTerraA)));
  delay(1000);

  StampaLCD(("Temp Amb: " + String(LetturaTempAmb)), ("Hum Amb: " +  String(LetturaHumAmb)));
  delay(1000);

  StampaLCD(("Luce: " + String(LetturaLuce)), "");
}

void InvioDatiJSON() {
  if (N_LOOP == CtrlMinutiInvioDati) {
    // Costruzione messaggio JSON manuale
    StaticJsonDocument<256> doc;

    if (client.connect(server, serverPort)) {
      Serial.println("Connessione stabilita!");

      // Costruzione messaggio JSON manuale
      StaticJsonDocument<200> doc; // num è lunghezza buffer
      doc["t_terra_a"] = LetturaTempTerraA;
      doc["t_terra_b"] = LetturaTempTerraB;
      doc["t_terra_c"] = LetturaTempTerraC;
      doc["t_terra_d"] = LetturaTempTerraD;

      doc["h_terra_a"] = LetturaHumTerraA;
      doc["h_terra_b"] = LetturaHumTerraB;
      doc["h_terra_c"] = LetturaHumTerraC;
      doc["h_terra_d"] = LetturaHumTerraD;

      doc["t_amb"] = LetturaTempAmb;
      doc["h_amb"] = LetturaHumAmb;

      doc["luce_amb"] = LetturaLuce;

      serializeJson(doc, StrJson);

      client.print(StrJson);
    } else {
      Serial.println("Errore nella connessione al server.");
    }
  }
}

// config
void setup() {
  Serial.begin(9600);

  Wire.begin();

  if (!light.begin()) {
    Serial.println("Impossibile trovare il sensore MAX44009");
  }

  lcd.init();           // Inizializza il display
  lcd.backlight();      // Accende la retroilluminazione

  // Dallas temperature
  DS18B20s.begin();
  int sensorCount = DS18B20s.getDeviceCount();
  Serial.print("Sensori trovati: ");
  Serial.println(sensorCount);

  if (sensorCount < 4) {
    Serial.println("Attenzione: meno di 4 sensori trovati!");
  }

  // Acquisisci gli indirizzi dei sensori temp terra
  for (int i = 0; i < sensorCount && i < 4; i++) {
    if (DS18B20s.getAddress(DS18B20Addresses[i], i)) {
      Serial.print("Sensore ");
      Serial.print(i);
      Serial.print(" indirizzo: ");
      printAddress(DS18B20Addresses[i]);
    } else {
      Serial.print("Errore nel leggere l'indirizzo del sensore ");
      Serial.println(i);
    }
  }

  // Init Ethernet
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(1000);

  Serial.print("IP di Arduino: ");
  Serial.println(Ethernet.localIP());

  // Avvia il sensore DHT22
  dht.begin();

  // Azzeramento ventole
  pinMode(PinVentoleA, OUTPUT);
  pinMode(PinVentoleB, OUTPUT);
  digitalWrite(PinVentoleA, LOW);
  digitalWrite(PinVentoleB, LOW);

  // Azzeramento pompe
  pinMode(PinPompaA, OUTPUT);
  pinMode(PinPompaB, OUTPUT);
  pinMode(PinPompaC, OUTPUT);
  // pinMode(PinPompaD, OUTPUT); non esiste nel nostro progetto

  digitalWrite(PinPompaA, HIGH); // Spegne la pompa all'inizio (logica inversa)
  digitalWrite(PinPompaB, HIGH); // Spegne la pompa all'inizio (logica inversa)
  digitalWrite(PinPompaC, HIGH); // Spegne la pompa all'inizio (logica inversa)
  // digitalWrite(PinPompaD, HIGH); // Spegne la pompa all'inizio (logica inversa) non esiste nel nostro progetto
}

// main
void loop() {
  ReadSensori();

  StampaLCD();

  InvioDatiJSON();

  CtrlVentole();

  CtrlPompe();
  
  N_LOOP++;
}
