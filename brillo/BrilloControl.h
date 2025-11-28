// BrilloControl.h
#ifndef BRILLO_CONTROL_H
#define BRILLO_CONTROL_H

// Incluye la librería estándar de Arduino para funciones como min/max
#include <Arduino.h>

// =======================================================
// CONSTANTES DE CONFIGURACIÓN DEL MODELO
// =======================================================
// Ajusta estos valores para calibrar el comportamiento de la luz
#define UMBRAL_OSCURIDAD_LDR 2000 // Lectura LDR por encima de la cual se considera 'oscuro' (0-4095)
#define NIVEL_BASE_OSCURIDAD 50   // Brillo mínimo cuando está oscuro sin movimiento (0-255)
#define IMPULSO_MOVIMIENTO   205  // Brillo adicional por detección PIR (50 + 205 = 255 total)
#define PESO_VECINO_FACTOR   0.35 // Factor (0.0 a 1.0) que modera la contribución del brillo vecino
#define TEMPERATURA_ALTA     30.0 // Umbral de temperatura para iniciar la reducción de brillo (ahorro/protección)
#define REDUCCION_POR_GRADO  0.01 // Reducción de 1% por cada grado sobre el UMBRAL_ALTA

/**
 * @brief Calcula el nivel de brillo de la luz de la calle (0 a 255) basado en los sensores.
 * * @param pir_estado Estado del PIR: 1 (Movimiento) o 0 (No Movimiento). (Obligatorio)
 * @param ldr_lectura Lectura del LDR: 0 (Mucha luz) a 4095 (Oscuridad). (Obligatorio)
 * @param dht_temperatura Temperatura en C (0 a 50). Usar 0.0 si no está disponible (Opcional).
 * @param vecino1_brillo Brillo del Vecino 1 (0 a 255). Usar -1 si no está disponible (Opcional).
 * @param vecino2_brillo Brillo del Vecino 2 (0 a 255). Usar -1 si no está disponible (Opcional).
 * @return int Nivel de brillo de salida (0 a 255).
 */
int calcularNivelBrillo(int pir_estado, int ldr_lectura, float dht_temperatura, int vecino1_brillo, int vecino2_brillo) {
    int nivelBrillo = 0;
    const int NIVEL_MAXIMO_SALIDA = 255;

    // 1. EVALUACIÓN OBLIGATORIA: LUZ AMBIENTAL (LDR)
    bool esOscuro = (ldr_lectura >= UMBRAL_OSCURIDAD_LDR);

    if (!esOscuro) {
        return 0; // Apagado si es de día
    }

    // Es de noche/oscuro: Iniciar con el brillo base.
    nivelBrillo = NIVEL_BASE_OSCURIDAD;

    // 2. EVALUACIÓN OBLIGATORIA: MOVIMIENTO (PIR)
    if (pir_estado == 1) {
        nivelBrillo = NIVEL_BASE_OSCURIDAD + IMPULSO_MOVIMIENTO; 
    }
    
    nivelBrillo = min(NIVEL_MAXIMO_SALIDA, nivelBrillo);

    // 3. EVALUACIÓN OPCIONAL: MÓDULOS VECINOS
    int vecinos_activos = 0;
    int brillo_acumulado = 0;

    if (vecino1_brillo >= 0 && vecino1_brillo <= NIVEL_MAXIMO_SALIDA) {
        brillo_acumulado += vecino1_brillo;
        vecinos_activos++;
    }

    if (vecino2_brillo >= 0 && vecino2_brillo <= NIVEL_MAXIMO_SALIDA) {
        brillo_acumulado += vecino2_brillo;
        vecinos_activos++;
    }

    if (vecinos_activos > 0) {
        int promedio_vecinos = brillo_acumulado / vecinos_activos;
        int ajuste_vecino = (int)(promedio_vecinos * PESO_VECINO_FACTOR);
        nivelBrillo += ajuste_vecino;
    } 

    // 4. EVALUACIÓN OPCIONAL: TEMPERATURA (DHT11)
    if (dht_temperatura > 0.0 && dht_temperatura > TEMPERATURA_ALTA) {
        float reduccion_grados = dht_temperatura - TEMPERATURA_ALTA;
        float factor_ajuste = 1.0 - (reduccion_grados * REDUCCION_POR_GRADO);
        
        factor_ajuste = max(0.85, factor_ajuste); 
        
        nivelBrillo = (int)(nivelBrillo * factor_ajuste);
    }

    // 5. AJUSTE FINAL Y SALIDA
    nivelBrillo = max(0, min(NIVEL_MAXIMO_SALIDA, nivelBrillo));

    return nivelBrillo;
}

#endif