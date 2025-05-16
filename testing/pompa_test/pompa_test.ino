int PinPompa = 37;

void setup() {
  pinMode(PinPompa, OUTPUT);
  digitalWrite(PinPompa, LOW);
}

void read_pompa() {

}

void write_pompa() {
  digitalWrite(PinPompa, LOW);
  delay(3000);
  digitalWrite(PinPompa, HIGH);
  delay(3000);
}

void loop() {
  write_pompa();
}
