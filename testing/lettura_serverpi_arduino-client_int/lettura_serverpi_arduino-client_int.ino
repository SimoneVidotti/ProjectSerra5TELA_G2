#include <SPI.h>
#include <Ethernet.h>

// Indirizzo MAC della shield Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP statico per Arduino
IPAddress ip(10, 3, 0, 177);
IPAddress gateway(10, 3, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// IP e porta del server (Raspberry Pi con Node-RED)
IPAddress server(10, 3, 0, 175);
int serverPort = 8081;

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

    // Invia un dato intero (es: 42)
    int numero = 20;
    client.println(numero);  // oppure client.print(numero); per non inviare \n
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
    delay(5000);
    setup();  // Riprova la connessione
  }
}
