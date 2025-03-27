#include <math.h>
#include "pav_analysis.h"

void hamming(float *w, unsigned int N) {
    for (int n = 0; n < N; n++) {
        w[n] = 0.54 - 0.46 * cos(2 * M_PI * n / (N - 1));
    }
}

float compute_power(const float *x, unsigned int N) {
    float pot = 1e-12;
    float w[N];
    float sum = 0.0;
    hamming(w, N);
    for (int i = 0; i < N; i++) {
        pot += x[i] * w[i] * x[i] * w[i];
        sum += w[i] * w[i];
    }
    return 10 * log10(pot / sum);
    
    
}


float compute_am(const float *x, unsigned int N) {
    float suma = 0.0;
    for (int i = 0; i < N; i++)
     {
        suma += fabs(x[i]);
    }
    return suma/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float zero_crossings = 0;
    for (int  i = 1; i < N+1; i++) {
        zero_crossings += (x[i] * x[i-1] < 0);
    }
    return zero_crossings / (N - 1) * fm / 2;;
}
