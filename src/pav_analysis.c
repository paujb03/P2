#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include "pav_analysis.h"
#define M_PI 3.14159265358979323846


float compute_power(const float *x, unsigned int N) {
    // Calcular la suma de los cuadrados de los primeros N valores en el array
    float suma_cuadrados = 0.0;
    for (unsigned int i = 0; i < N; ++i) {
        suma_cuadrados += x[i] * x[i];
    }

    // Dividir la suma de los cuadrados por N
    float promedio = suma_cuadrados / N;

    // Convertir el promedio a dB utilizando 10 * log10()
    float resultado_dB = 10.0 * log10(promedio);

    return resultado_dB;
}

float compute_am(const float *x, unsigned int N) {
    // Calcular la suma de los valores absolutos de los primeros N elementos en el array
    float suma_absolutos = 0.0;
    for (unsigned int i = 0; i < N; ++i) {
        suma_absolutos += fabs(x[i]);
    }

    // Dividir la suma absoluta por N
    float resultado = suma_absolutos / N;

    return resultado;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    // Inicializar la suma de cambios de signo
    int suma_cambios_signo = 0;

    // Calcular la suma de cambios de signo para n que va desde 0 hasta N-1
    for (unsigned int n = 1; n < N; ++n) {
        if ((x[n] >= 0 && x[n - 1] < 0) || (x[n] < 0 && x[n - 1] >= 0)) {
            // Si el signo de los valores consecutivos es diferente, incrementar la suma
            suma_cambios_signo++;
        }
    }

    // Calcular el resultado final
    float resultado = (float)suma_cambios_signo * (fm) / (2 * (N - 1));

    return resultado;
}

#define EPSILON 1e-12

// Función para calcular la potencia con la ventana de Hamming precalculada
float compute_power2(const float *x, unsigned int N) {

    // AMPLIACIÓN 2 HAMMING //

    // Variable para almacenar temporalmente el valor de la ventana de Hamming
    float hamming_value;  

    // Arreglo para almacenar los valores de la ventana de Hamming    
    float w[N]; 

    // Variable para almacenar la suma de los cuadrados de los valores de la ventana              
    float denominador = 0;    

    // Bucle para calcular la constante denominador
    for (int i = 0; i < N; i++) {
        // Calcula el valor de la ventana de Hamming para la posición actual (i)
        hamming_value = hamming_window(i, N);

        // Almacena el valor de la ventana de Hamming en el arreglo w
        w[i] = hamming_value;

        // Acumula la suma de los cuadrados de los valores de la ventana de Hamming
        denominador += hamming_value * hamming_value;
    }

    double numerador = EPSILON; // Evita divisiones por cero

    // Calcula el numerador de la potencia ponderada
    for(unsigned int i = 0; i < N; i++) {
        numerador += pow(x[i] * w[i], 2.0);
    }

    // Calcula y devuelve la potencia en decibelios
    return (10 * log10(numerador / denominador));
}

float hamming_window(int n, int M) {
    float a0 = HAMMING_A0;
    float a1 = 1 - a0;
    return a0 - a1 * cos((2 * M_PI * n) / (M - 1));
}