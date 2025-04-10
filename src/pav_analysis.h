#ifndef PAV_ANALYSIS_H
#define PAV_ANALYSIS_H
#define HAMMING_A0 0.54

float compute_power(const float *x, unsigned int N);
float compute_power2(const float *x, unsigned int N); 
float compute_am(const float *x, unsigned int N);
float compute_zcr(const float *x, unsigned int N, float fm);
float hamming_window(int n, int M);

#endif	/* PAV_ANALYSIS_H	*/
