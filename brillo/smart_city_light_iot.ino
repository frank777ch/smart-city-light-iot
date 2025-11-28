// smart_city_light_iot.ino
#include "BrilloControl.h"

// =======================================================
// DEFINICIÓN DE PINES
// =======================================================
#define PIN_PIR         34 // Pin de entrada para el sensor PIR
#define PIN_LDR         35 // Pin de entrada para el sensor LDR
#define PIN_SALIDA_LUZ  23 // Pin de salida PWM para la luz (Luz LED)

void setup() {
  // Inicialización de la comunicación serial
  Serial.begin(115200);

  // Configuración de pines de entrada y salida
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_SALIDA_LUZ, OUTPUT);

  Serial.println("--- Smart City Light IoT: Sistema Inicializado ---");
  Serial.println("PIR | LDR | TEMP | V1 | V2 | -> BRIGHTNESS");
  Serial.println("----------------------------------------------");
}

void loop() {
  // -----------------------------------------------------------------
  // 1. LECTURA DE SENSORES Y MÓDULOS
  // -----------------------------------------------------------------
  // Lecturas Obligatorias
  int pir_val = digitalRead(PIN_PIR);      
  int ldr_val = analogRead(PIN_LDR);      

  // Lecturas Opcionales: Sustituir por tu lógica de lectura (DHT/Red)
  // Usar 0.0 para DHT no disponible. Usar -1 para Vecino no disponible.
  float temp_val = 25.5; // Simulación: Temperatura normal
  int vecino1_val = 0;   // Simulación: Vecino apagado
  int vecino2_val = -1;  // Simulación: Vecino no disponible

  // --- EJEMPLOS PARA PRUEBA (DESCOMENTAR UNO PARA SIMULAR ESCENARIO) ---
  /*
  // Caso 1: DÍA - La luz debe estar apagada (0)
  pir_val = 0; 
  ldr_val = 500; // Mucha luz
  temp_val = 22.0;
  vecino1_val = 255; // Los vecinos no importan si es de día
  vecino2_val = 255;
  */

  /*
  // Caso 2: NOCHE + MOVIMIENTO - Máximo Brillo (255)
  pir_val = 1; 
  ldr_val = 4000; // Oscuridad total
  temp_val = 20.0;
  vecino1_val = 0; 
  vecino2_val = 0;
  */

  // Caso 3: NOCHE + SIN MOVIMIENTO + VECINOS ACTIVOS - Brillo medio
  pir_val = 0; 
  ldr_val = 3000; // Oscuro
  temp_val = 32.0; // Hace calor (activará la reducción)
  vecino1_val = 255; // Vecino 1 encendido
  vecino2_val = 200; // Vecino 2 a medio encender
  
  // -----------------------------------------------------------------
  // 2. LLAMADA A LA FUNCIÓN DE CÁLCULO
  // -----------------------------------------------------------------
  int brilloFinal = calcularNivelBrillo(pir_val, ldr_val, temp_val, vecino1_val, vecino2_val);

  // -----------------------------------------------------------------
  // 3. APLICAR Y DEPURAR LA SALIDA
  // -----------------------------------------------------------------
  analogWrite(PIN_SALIDA_LUZ, brilloFinal);
  
  // Depuración por Serial
  Serial.print(pir_val);
  Serial.print("   | "); Serial.print(ldr_val);
  Serial.print(" | "); Serial.print(temp_val);
  Serial.print(" | "); Serial.print(vecino1_val);
  Serial.print(" | "); Serial.print(vecino2_val);
  Serial.print(" | -> "); Serial.println(brilloFinal);

  delay(2000); 
}