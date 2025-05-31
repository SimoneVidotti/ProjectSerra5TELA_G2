// Questo è solo da esempio, se eseguito, non funzionerà

#include <SPI.h>
#include <Ethernet.h>

// Indirizzo MAC e IP Arduino
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 3, 0, 177);
IPAddress gateway(10, 3, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// IP del server (Node-RED)
IPAddress server(10, 3, 0, 175);
int serverPort = 8081;

EthernetClient client;

// Pin sensori (esempio)
#define T_AMB_PIN           A
#define IRRAGGIAMENTO_PIN   A
#define U_AMB_PIN           A
#define T_TERRA_PIN_A       A
#define U_TERRA_PIN_A       A
#define T_TERRA_PIN_B       A
#define U_TERRA_PIN_B       A
#define T_TERRA_PIN_C       A
#define U_TERRA_PIN_C       A

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // ogni 5 secondi

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(3000);
  
  Serial.print("IP Arduino: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  if (millis() - lastSendTime > sendInterval) {
    lastSendTime = millis();

    if (!client.connected()) {
      Serial.println("Connessione al server...");
      if (client.connect(server, serverPort)) {
        Serial.println("Connesso!");
      } else {
        Serial.println("Connessione fallita.");
        return;
      }
    }

    // Letture dai sensori (esempi)
    int raw_t_terra = analogRead(TEMP_TERRA_PIN);
    float t_terra = raw_t_terra * (5.0 / 1023.0) * 100.0; // esempio conversione

    int raw_u_terra = analogRead(UMID_TERRA_PIN);
    int u_terra = map(raw_u_terra, 0, 1023, 0, 100);

    int raw_t_am = analogRead(TEMP_AMB_PIN);
    float t_am = raw_t_am * (5.0 / 1023.0) * 100.0;

    int ir_am = analogRead(IRRAGGIAMENTO_PIN); // raw

    int raw_u_am = analogRead(UMID_AMB_PIN);
    int u_am = map(raw_u_am, 0, 1023, 0, 100);

    // Costruzione messaggio JSON
    String json = "{";
    json += "\"t_terra\":" + String(t_terra, 1) + ",";
    json += "\"u_terra\":" + String(u_terra) + ",";
    json += "\"t_am\":" + String(t_am, 1) + ",";
    json += "\"ir_am\":" + String(ir_am) + ",";
    json += "\"u_am\":" + String(u_am);
    json += "}";

    Serial.println("Invio dati:");
    Serial.println(json);

    // Invio al server
    client.println(json);
  }

  // Lettura risposta (opzionale)
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // Riconnessione se persa, debug 
  if (!client.connected()) {
    Serial.println("Disconnesso dal server.");
    client.stop();
  }
}
