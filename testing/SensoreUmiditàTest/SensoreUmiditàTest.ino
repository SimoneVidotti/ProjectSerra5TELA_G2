// Definizione pin sensori
#define SENSOR1_PIN A0
#define SENSOR2_PIN A1
#define SENSOR3_PIN A2

// Calibrazione per ciascun sensore (sostituisci con i tuoi valori)
const int DRY_VALUE = 620;  // Valore in aria secca
const int WET_VALUE = 310;  // Valore in acqua

void setup() {
  Serial.begin(9600);
}

// Funzione per leggere un singolo sensore
int readMoisture(int sensorPin) {
  int value = analogRead(sensorPin);
  int moisturePercent = map(value, DRY_VALUE, WET_VALUE, 0, 100);
  return constrain(moisturePercent, 0, 100);  // Limita tra 0% e 100%
}

void loop() {
  // Leggi tutti i sensori
  int moisture1 = readMoisture(SENSOR1_PIN);
  int moisture2 = readMoisture(SENSOR2_PIN);
  int moisture3 = readMoisture(SENSOR3_PIN);

  // Stampa i risultati
  Serial.print("Sensore 1: ");
  Serial.print(moisture1);
  Serial.print("% | Sensore 2: ");
  Serial.print(moisture2);
  Serial.print("% | Sensore 3: ");
  Serial.print(moisture3);
  Serial.println("%");

  delay(1000);  // Aspetta 1 secondo tra le letture
}