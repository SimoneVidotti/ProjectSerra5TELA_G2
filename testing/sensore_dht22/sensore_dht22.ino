#include <DHT.h>

#define DHTPIN 2          // Pin digitale collegato al DHT22
#define DHTTYPE DHT22     // Tipo di sensore (DHT22 o AM2302)

DHT dht(DHTPIN, DHTTYPE); // Inizializza il sensore

void setup() {
  Serial.begin(9600);
  dht.begin();            // Avvia il sensore
  Serial.println("DHT22 Test");
}

void loop() {
  delay(2000); // Attendere 2 secondi tra le letture (il DHT22 è lento)

  float humidity = dht.readHumidity();     // Leggi umidità (%)
  float temperature = dht.readTemperature(); // Leggi temperatura (°C)

  // Stampa i dati
  Serial.print("Umidità: ");
  Serial.print(humidity);
  Serial.print("% | Temperatura: ");
  Serial.print(temperature);
  Serial.println("°C");
}