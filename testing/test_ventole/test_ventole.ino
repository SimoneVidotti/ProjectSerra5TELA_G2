int pin_ventola1 = 49;
int pin_ventola2 = 48;

int Lettura1, Lettura2;

void setup() {
  Serial.begin(9600);
  pinMode(pin_ventola1, OUTPUT);
  pinMode(pin_ventola2, OUTPUT);
  digitalWrite(pin_ventola1, LOW);
  digitalWrite(pin_ventola2, LOW);
}

void Stampa(int L, int sens) {
  L = digitalRead(sens);
  Serial.println(L);
}

void Linea() {
  Serial.println("- - - - - - - - -");
}

void loop() {
  digitalWrite(pin_ventola1, LOW);
  digitalWrite(pin_ventola2, LOW);
  delay(5000);
  Stampa(Lettura1, pin_ventola1);
  Stampa(Lettura2, pin_ventola2);
  Linea();

  digitalWrite(pin_ventola1, HIGH);
  digitalWrite(pin_ventola2, HIGH);
  delay(5000);
  Stampa(Lettura1, pin_ventola1);
  Stampa(Lettura2, pin_ventola2);
  Linea();
}
