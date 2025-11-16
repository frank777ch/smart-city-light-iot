// --- CONFIGURACIÓN DEL SENSOR PIR ---
#define PIR_PIN 13  // Pin donde conectaste la señal "OUT" del PIR

void setup() {
  Serial.begin(115200);
  
  // Configurar el pin del PIR como ENTRADA
  pinMode(PIR_PIN, INPUT);

  Serial.println("Iniciando sensor PIR...");
  Serial.println("Calibrando... (no te muevas por 30 segundos)");
  delay(30000); // Esperamos 30 segundos a que el sensor se estabilice
  Serial.println("¡Sensor listo!");
}

void loop() {
  
  // 1. Leer el valor del pin
  int valor = digitalRead(PIR_PIN);

  // 2. Imprimir el estado ACTUAL
  Serial.print("Estado del sensor: ");
  Serial.println(valor); // Imprimirá '0' (LOW) o '1' (HIGH)

  // 3. Comprobamos si hay movimiento para mostrar el mensaje
  if (valor == HIGH) {
    Serial.println("===> ¡MOVIMIENTO DETECTADO! <===");
  }
  
  // Pequeña pausa para no saturar el monitor
  delay(500); // Imprime el estado 2 veces por segundo
}