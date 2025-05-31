#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

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

void setup() {
  Serial.begin(9600);
  while (!Serial) {} ;

  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(5000);

  Serial.print("IP di Arduino: ");
  Serial.println(Ethernet.localIP());

  Serial.println("Connessione al server...");
  if (client.connect(server, serverPort)) {
    Serial.println("Connessione stabilita!");

    String json;
    // Costruzione messaggio JSON manuale
    StaticJsonDocument<200> doc;
    doc["temperatura"] = 24.3;
    doc["umidita"] = 60.1;
    doc["suolo1"] = 350;
    doc["suolo2"] = 402;

    serializeJson(doc, json);

    client.print(json);
  } else {
    Serial.println("Errore nella connessione al server.");
  }
}

void loop() {
  // Lettura risposta dal server
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // Riconnessione se disconnesso
  if (!client.connected()) {
    Serial.println();
    Serial.println("Disconnessione dal server.");
    client.stop();
    delay(1000);
    setup();  // Riprova la connessione
  }
}