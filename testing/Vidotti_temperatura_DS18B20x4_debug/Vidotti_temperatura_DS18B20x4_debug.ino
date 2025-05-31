// Vidotti, G2, debug Sensori temperatura DS18B20, standard code example
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 53   // Specify the digital pin connected to the DS18B20 sensors

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Array per salvare gli indirizzi dei sensori
DeviceAddress sensorAddresses[4];

void setup(void) {
  Serial.begin(9600);
  Serial.println("Dallas Temperature Control Library - 4 DS18B20 su un singolo pin");

  sensors.begin();

  int sensorCount = sensors.getDeviceCount();
  Serial.print("Sensori trovati: ");
  Serial.println(sensorCount);

  if (sensorCount < 4) {
    Serial.println("Attenzione: meno di 4 sensori trovati!");
  }

  // Acquisisci gli indirizzi dei sensori
  for (int i = 0; i < sensorCount && i < 4; i++) {
    if (sensors.getAddress(sensorAddresses[i], i)) {
      Serial.print("Sensore ");
      Serial.print(i);
      Serial.print(" indirizzo: ");
      printAddress(sensorAddresses[i]);
    } else {
      Serial.print("Errore nel leggere l'indirizzo del sensore ");
      Serial.println(i);
    }
  }
}

void loop(void) {
  sensors.requestTemperatures();

  for (int i = 0; i < 4; i++) {
    float tempC = sensors.getTempC(sensorAddresses[i]);
    Serial.print("Temperatura sensore ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(tempC);
    Serial.println(" °C");
  }

  delay(2000); // Pausa di 2 secondi tra le letture
}

// Funzione per stampare l'indirizzo di un sensore
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}