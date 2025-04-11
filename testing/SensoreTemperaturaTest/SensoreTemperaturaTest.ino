#define SensorTestPin A12

void setup() {
  pinMode(SensorTestPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(SensorTestPin);
  
  // Calculate temperature using the linear equation
  float m = -0; // Slope
  float b = 80; // Intercept
  float temperature = m * sensorValue + b;
  
  Serial.print("Analog Reading: ");
  Serial.print(sensorValue);
  Serial.print(" - Temperature: ");
  Serial.println(temperature);
  
  delay(1000); // Delay for readability
}