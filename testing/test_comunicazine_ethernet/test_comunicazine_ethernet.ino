#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Configurazione Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 3, 0, 177);          // IP statico del dispositivo
IPAddress gateway(10, 3, 0, 1);        // Gateway della rete
IPAddress subnet(255, 255, 255, 0);    // Subnet mask
IPAddress server(10, 3, 0, 197);       // IP del Raspberry Pi con Node-RED
int serverPort = 9000;                 // Porta del server Node-RED

// Sensori temperatura
#define ONE_WIRE_BUS 46
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

EthernetClient client;
unsigned long lastAttemptTime = 0;
const unsigned long retryInterval = 10000;  // 10 secondi tra i tentativi

void setup() {
  Serial.begin(9600);
  while(!Serial); // Attendere solo su dispositivi con USB

  initNetwork();
  initSensors();
  testNetworkConnection();
}

void loop() {
  if(millis() - lastAttemptTime > retryInterval) {
    if(attemptConnection()) {
      sendSensorData();
    }
    lastAttemptTime = millis();
  }
}

void initNetwork() {
  Serial.println("Inizializzazione rete...");
  
  // Tentativo DHCP con fallback statico
  if(Ethernet.begin(mac, 10000, 4000) == 0) {  // Timeout 10s, risposta 4s
    Serial.println("DHCP fallito, uso IP statico");
    Ethernet.begin(mac, ip, gateway, subnet);
  }

  printNetworkInfo();
}

void printNetworkInfo() {
  Serial.print("MAC: ");
  for(byte i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if(i < 5) Serial.print(":");
  }
  Serial.println();
  
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Gateway: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("Subnet: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Link: ");
  Serial.println(Ethernet.linkStatus() == LinkON ? "UP" : "DOWN");
}

void initSensors() {
  sensors.begin();
  Serial.print("Trovati ");
  Serial.print(sensors.getDeviceCount());
  Serial.println(" sensori");
}

bool attemptConnection() {
  Serial.println("\nTentativo di connessione...");
  Serial.print("Server: ");
  Serial.print(server);
  Serial.print(":");
  Serial.println(serverPort);

  // Chiudi connessioni precedenti
  if(client.connected()) {
    client.stop();
    delay(200);
  }

  if(client.connect(server, serverPort)) {
    Serial.println("Connesso al server Node-RED!");
    return true;
  } else {
    Serial.println("ERRORE: Connessione fallita");
    Serial.print("Stato: ");
    switch(client.status()) {
      case 0: Serial.println("Idle"); break;
      case 1: Serial.println("Connessione in corso"); break;
      case 2: Serial.println("DNS failed"); break;
      case 3: Serial.println("Connesso"); break;
      case 4: Serial.println("Disconnessione in corso"); break;
      default: Serial.println(client.status()); break;
    }
    return false;
  }
}

void sendSensorData() {
  sensors.requestTemperatures();
  
  String payload = "{";
  for(int i = 0; i < sensors.getDeviceCount(); i++) {
    float temp = sensors.getTempCByIndex(i);
    payload += "\"t_terra_" + String(char('A'+i)) + "\":" + String(temp, 2);
    if(i < sensors.getDeviceCount()-1) payload += ",";
  }
  payload += "}";

  Serial.println("Invio dati: " + payload);

  // HTTP Request per Node-RED
  client.println("POST /api/data HTTP/1.1");
  client.println("Host: 10.3.0.197");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(payload.length());
  client.println();
  client.println(payload);

  // Attendi risposta
  unsigned long start = millis();
  while(!client.available() && millis()-start < 3000) {
    delay(10);
  }

  // Leggi risposta
  while(client.available()) {
    Serial.write(client.read());
  }

  client.stop();
  Serial.println("Connessione chiusa");
}

void testNetworkConnection() {
  Serial.println("\nTest di connettività...");
  Serial.println("Ping al gateway:");
  if(Ethernet.gatewayIP() != INADDR_NONE) {
    if(client.connect(Ethernet.gatewayIP(), 80)) {
      Serial.println("Gateway raggiungibile");
      client.stop();
    } else {
      Serial.println("Gateway non raggiungibile");
    }
  } else {
    Serial.println("Gateway non configurato");
  }

  Serial.println("Ping al server Node-RED:");
  if(client.connect(server, serverPort)) {
    Serial.println("Server Node-RED raggiungibile");
    client.stop();
  } else {
    Serial.println("Server Node-RED non raggiungibile");
  }
}