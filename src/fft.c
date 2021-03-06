/* 
 * Free FFT and convolution (C)
 * 
 * Copyright (c) 2014 Project Nayuki
 * http://www.nayuki.io/page/free-small-fft-in-multiple-languages
 * 
 * (MIT License)
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fft.h"


// Private function prototypes
static size_t reverse_bits(size_t x, unsigned int n);

#define SIZE_MAX ((size_t)-1)


/**
 * int fft_2signals(int n)
 * 
 * @brief this function implements the complex fast-fourier transform of 2 real-valued signal at the same time.
 * This is achieved by packing one signal in the real domain and the other in the imaginary part.
 * @param signal1, the first signal.
 * @param signal2, the second signal.
 * @param X1_real, the real part of the fft of the first signal.
 * @param X1_imag, the imaginary part of the fft of the first signal.
 * @param X2_real, the real part of the fft of the second signal.
 * @param X2_imag, the imaginary part of the fft of the second signal.
 * @param n, the length of the fft to be computed.
 * @return 1 if success, 0 otherwise
 */ 
int fft_2signals(double* signal_1, double* signal_2, 
                 double* X1_real, double* X1_imag, 
                 double* X2_real, double* X2_imag,
                 size_t n){
	
	int status;
	register int i,k;
	double *X_real = (double*)malloc(n * sizeof(double));
	double *X_imag = (double*)malloc(n * sizeof(double));
	
	for(i=0;i<n;i++){
		X_real[i] = signal_1[i];
		X_imag[i] = signal_2[i];
	}
	
	/*Compute the fourier transform of the two signals at once*/
	if(!transform(X_real, X_imag, n)){
		status = 0;
		goto cleanup;
	}
	
	/*compute the split operation to recover X1(k) and X2(k)*/
	/* 
	 * X1(k) = 1/2*[X(k)+X*(N-k)]
	 * X2(k) = 1/(j2)*[X(k)-X*(N-k)]
	 */
	X1_real[0] = X_real[0];
	X1_imag[0] = 0;

	X2_real[0] = X_imag[0];
	X2_imag[0] = 0;

	X1_real[n/2] = X_imag[n/2];
	X2_real[n/2] = X_imag[n/2];
	X1_imag[n/2] = 0;
	X2_imag[n/2] = 0;	
	
	for(k=1;k<n/2;k++){
		X1_real[k] = 0.5*(X_real[k]+X_real[n-k]);
		X2_real[k] = 0.5*(X_imag[k]+X_imag[n-k]);
		
		/*make use of the symmetry*/
		X1_real[n-k] = X1_real[k];
		X2_real[n-k] = X2_real[k];
		
		X1_imag[k] = 0.5*(X_imag[k]-X_imag[n-k]);
		X2_imag[k] = -0.5*(X_real[k]-X_real[n-k]);
		
		/*make use of the symmetry*/
		X1_imag[n-k] = -X1_imag[k];
		X2_imag[n-k] = -X2_imag[k];
	}
	
	status = 1;
	
cleanup:
	free(X_real);
	free(X_imag);
	
	return status;
}

/**
 * int abs_fft_2signals(int n)
 * 
 * @brief this function implements the fast-fourier transform of 2 real-valued signal at the same time
 * and returns the absolute values of the one-sided fft transform of each signals.
 * @param signal1 (in), the first signal.
 * @param signal2 (in), the second signal.
 * @param X1 (out), abs value of one-sided fft of signal 1 (length n/2+1).
 * @param X2 (out), abs value of one-sided fft of signal 2 (length n/2+1).
 * @param n, the length of the fft to be computed.
 * @return 1 if success, 0 otherwise
 */       
int abs_fft_2signals(double* signal_1, double* signal_2, 
                 double* X1, 
                 double* X2,
                 size_t n){
				
	int i, status;
					 
	double* X1_real = (double*)malloc(sizeof(double)*n);
	double* X1_imag = (double*)malloc(sizeof(double)*n);
	double* X2_real = (double*)malloc(sizeof(double)*n);
	double* X2_imag = (double*)malloc(sizeof(double)*n);					 
					 
	/*compute the complex fft of both signals*/				 
	if(!fft_2signals(signal_1, signal_2, 
                 X1_real, X1_imag, 
                 X2_real, X2_imag,
                 n)){
		status = 0;	
		goto cleanup;			
	}
	
	/*compute the absolute values of the one-sided fft of the signal*/
	for(i=0;i<(n/2+1);i++){
		X1[i] = 2*(sqrt(X1_real[i]*X1_real[i] + X1_imag[i]*X1_imag[i])/n);
		X2[i] = 2*(sqrt(X2_real[i]*X2_real[i] + X2_imag[i]*X2_imag[i])/n);
	}			
	
	status = 1;			
				
cleanup:
				
	free(X1_real);	
	free(X1_imag);	
	free(X2_real);	
	free(X2_imag);	 
				 
	return status;				 
}


/**
 * int abs_fft(int n)
 * 
 * @brief this function implements the fast-fourier transform of a signal and return the absolute values
 * of the one-sided fft.
 * @param signal (in), the n-long signal.
 * @param abs_onesided_fft (out), abs value of one-sided fft of the signal(length n/2+1).
 * @param n, the length of the signal to be computed.
 * @return 1 if success, 0 otherwise
 */                
int abs_fft(double* signal,
            double* abs_onesided_fft, 
            size_t n){
		
	int i,status = 0;		
	int one_sided_fft_length = (n/2)+1;	
	/*duplicate the signal*/
	double* real = (double*)memdup(signal, n*sizeof(double));
	double* imag = zero_reals(n);
	
	/*compute the complex fft*/
	status = transform(real, imag, n);
	
	/*compute the abs values one-sided fft*/
	if(status){
		for(i=0;i<one_sided_fft_length;i++){
			abs_onesided_fft[i] = 2*(sqrt(real[i]*real[i]+imag[i]*imag[i])/n);
		}
	}
	
	free(real);
	free(imag);
	
	return status;
				
}                 


/**
 * void get_signal_sin(double* signal, int sample_length, double norm_frequency, double diff_factor)
 * @brief Returns customary information about a fft transform, given a signal length and a sampling frequency.
 * @param freq_scale (out), n/2+1-long array that will contain the signal
 * @param df (out), delta between two adjancent frequencies
 * @param n (in), length of the signal that was ffted
 * @param fs (in), sampling frequency
 */ 
void get_fft_infos(double* freq_scale, double* df, int n, double fs){
	
	int i=0;
	int half_spectrum = n/2+1;
	
	*df = fs/(double)n;
	
	for(i=0;i<half_spectrum;i++){
		freq_scale[i] = (*df)*(double)i;
	}
	
}



int transform(double real[], double imag[], size_t n) {
	if (n == 0)
		return 1;
	else if ((n & (n - 1)) == 0)  // Is power of 2
		return transform_radix2(real, imag, n);
	else  // More complicated algorithm for arbitrary sizes
		return transform_bluestein(real, imag, n);
}


int inverse_transform(double real[], double imag[], size_t n) {
	return transform(imag, real, n);
}


int transform_radix2(double real[], double imag[], size_t n) {
	// Variables
	int status = 0;
	unsigned int levels;
	double *cos_table, *sin_table;
	size_t size;
	size_t i;
	
	// Compute levels = floor(log2(n))
	{
		size_t temp = n;
		levels = 0;
		while (temp > 1) {
			levels++;
			temp >>= 1;
		}
		if (1u << levels != n)
			return 0;  // n is not a power of 2
	}
	
    /*precompute the trignometric tables for optimization*/
	if (SIZE_MAX / sizeof(double) < n / 2)
		return 0;
	size = (n / 2) * sizeof(double);
	cos_table = (double*)malloc(size);
	sin_table = (double*)malloc(size);
	if (cos_table == NULL || sin_table == NULL)
		goto cleanup;
	for (i = 0; i < n / 2; i++) {
		cos_table[i] = cos(2 * M_PI * i / n);
		sin_table[i] = sin(2 * M_PI * i / n);
	}
	
	// Bit-reversed addressing permutation
	for (i = 0; i < n; i++) {
		size_t j = reverse_bits(i, levels);
		if (j > i) {
			double temp = real[i];
			real[i] = real[j];
			real[j] = temp;
			temp = imag[i];
			imag[i] = imag[j];
			imag[j] = temp;
		}
	}
	
	// Cooley-Tukey decimation-in-time radix-2 FFT
	for (size = 2; size <= n; size *= 2) {
		size_t halfsize = size / 2;
		size_t tablestep = n / size;
		for (i = 0; i < n; i += size) {
			size_t j;
			size_t k;
			for (j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
				double tpre =  real[j+halfsize] * cos_table[k] + imag[j+halfsize] * sin_table[k];
				double tpim = -real[j+halfsize] * sin_table[k] + imag[j+halfsize] * cos_table[k];
				real[j + halfsize] = real[j] - tpre;
				imag[j + halfsize] = imag[j] - tpim;
				real[j] += tpre;
				imag[j] += tpim;
			}
		}
		if (size == n)  // Prevent overflow in 'size *= 2'
			break;
	}
	status = 1;
	
cleanup:
	free(cos_table);
	free(sin_table);
	return status;
}


int transform_bluestein(double real[], double imag[], size_t n) {
	// Variables
	int status = 0;
	double *cos_table, *sin_table;
	double *areal, *aimag;
	double *breal, *bimag;
	double *creal, *cimag;
	size_t m;
	size_t size_n, size_m;
	size_t i;
	
	// Find a power-of-2 convolution length m such that m >= n * 2 + 1
	{
		size_t target;
		if (n > (SIZE_MAX - 1) / 2)
			return 0;
		target = n * 2 + 1;
		for (m = 1; m < target; m *= 2) {
			if (SIZE_MAX / 2 < m)
				return 0;
		}
	}
	
	// Allocate memory
	if (SIZE_MAX / sizeof(double) < n || SIZE_MAX / sizeof(double) < m)
		return 0;
	size_n = n * sizeof(double);
	size_m = m * sizeof(double);
	cos_table = (double*)malloc(size_n);
	sin_table = (double*)malloc(size_n);
	areal = (double*)calloc(m, sizeof(double));
	aimag = (double*)calloc(m, sizeof(double));
	breal = (double*)calloc(m, sizeof(double));
	bimag = (double*)calloc(m, sizeof(double));
	creal = (double*)malloc(size_m);
	cimag = (double*)malloc(size_m);
	if (cos_table == NULL || sin_table == NULL
			|| areal == NULL || aimag == NULL
			|| breal == NULL || bimag == NULL
			|| creal == NULL || cimag == NULL)
		goto cleanup;
	
	// Trignometric tables
	for (i = 0; i < n; i++) {
		double temp = M_PI * (size_t)((unsigned long long)i * i % ((unsigned long long)n * 2)) / n;
		// Less accurate version if long long is unavailable: double temp = M_PI * i * i / n;
		cos_table[i] = cos(temp);
		sin_table[i] = sin(temp);
	}
	
	// Temporary vectors and preprocessing
	for (i = 0; i < n; i++) {
		areal[i] =  real[i] * cos_table[i] + imag[i] * sin_table[i];
		aimag[i] = -real[i] * sin_table[i] + imag[i] * cos_table[i];
	}
	breal[0] = cos_table[0];
	bimag[0] = sin_table[0];
	for (i = 1; i < n; i++) {
		breal[i] = breal[m - i] = cos_table[i];
		bimag[i] = bimag[m - i] = sin_table[i];
	}
	
	// Convolution
	if (!convolve_complex(areal, aimag, breal, bimag, creal, cimag, m))
		goto cleanup;
	
	// Postprocessing
	for (i = 0; i < n; i++) {
		real[i] =  creal[i] * cos_table[i] + cimag[i] * sin_table[i];
		imag[i] = -creal[i] * sin_table[i] + cimag[i] * cos_table[i];
	}
	status = 1;
	
	// Deallocation
cleanup:
	free(cimag);
	free(creal);
	free(bimag);
	free(breal);
	free(aimag);
	free(areal);
	free(sin_table);
	free(cos_table);
	return status;
}

/**
 * naive_dft(const double *inreal, const double *inimag, double *outreal, double *outimag, int inverse, int n)
 * 
 * @brief this function implements the reference naive fourier transform, by using the true definition.
  *       It is not optimal, but gives the correct result (this function was implemented by the original author)
 * @param inreal, inimag, complex signal to be processed
 * @param outreal, outimag, fourier transform of the signal
 * @param inverse, 1 to compute inverse transform, 0 for the forward transform
 * @param n, length of the signal
 * @return none
 */ 
void naive_dft(const double *inreal, const double *inimag, double *outreal, double *outimag, int inverse, int n) {
	double coef = (inverse ? 2 : -2) * M_PI;
	int k;
	for (k = 0; k < n; k++) {  // For each output element
		double sumreal = 0;
		double sumimag = 0;
		int t;
		for (t = 0; t < n; t++) {  // For each input element
			double angle = coef * ((long long)t * k % n) / n;
			sumreal += inreal[t]*cos(angle) - inimag[t]*sin(angle);
			sumimag += inreal[t]*sin(angle) + inimag[t]*cos(angle);
		}
		outreal[k] = sumreal;
		outimag[k] = sumimag;
	}
}


int convolve_real(const double x[], const double y[], double out[], size_t n) {
	double *ximag, *yimag, *zimag;
	int status = 0;
	ximag = (double*)calloc(n, sizeof(double));
	yimag = (double*)calloc(n, sizeof(double));
	zimag = (double*)calloc(n, sizeof(double));
	if (ximag == NULL || yimag == NULL || zimag == NULL)
		goto cleanup;
	
	status = convolve_complex(x, ximag, y, yimag, out, zimag, n);
cleanup:
	free(zimag);
	free(yimag);
	free(ximag);
	return status;
}


int convolve_complex(const double xreal[], const double ximag[], const double yreal[], const double yimag[], double outreal[], double outimag[], size_t n) {
	int status = 0;
	size_t size;
	size_t i;
	double *xr, *xi, *yr, *yi;
	if (SIZE_MAX / sizeof(double) < n)
		return 0;
	size = n * sizeof(double);
	xr = (double*)memdup(xreal, size);
	xi = (double*)memdup(ximag, size);
	yr = (double*)memdup(yreal, size);
	yi = (double*)memdup(yimag, size);
	if (xr == NULL || xi == NULL || yr == NULL || yi == NULL)
		goto cleanup;
	
	if (!transform(xr, xi, n))
		goto cleanup;
	if (!transform(yr, yi, n))
		goto cleanup;
	for (i = 0; i < n; i++) {
		double temp = xr[i] * yr[i] - xi[i] * yi[i];
		xi[i] = xi[i] * yr[i] + xr[i] * yi[i];
		xr[i] = temp;
	}
	if (!inverse_transform(xr, xi, n))
		goto cleanup;
	for (i = 0; i < n; i++) {  // Scaling (because this FFT implementation omits it)
		outreal[i] = xr[i] / n;
		outimag[i] = xi[i] / n;
	}
	status = 1;
	
cleanup:
	free(yi);
	free(yr);
	free(xi);
	free(xr);
	return status;
}


static size_t reverse_bits(size_t x, unsigned int n) {
	size_t result = 0;
	unsigned int i;
	for (i = 0; i < n; i++, x >>= 1)
		result = (result << 1) | (x & 1);
	return result;
}


void *memdup(const void *src, size_t n) {
	void *dest = malloc(n);
	if (dest != NULL)
		memcpy(dest, src, n);
	return dest;
}

/**
 * random_reals(int n)
 * 
 * @brief this function returns a vector of real elements.
  *       (this function was implemented by the original author)
 * @param n, length of the signal
 * @return 1 if success, 0 otherwise
 */ 
double *random_reals(int n) {
	double *result = (double*)malloc(n * sizeof(double));
	int i;
	for (i = 0; i < n; i++)
		result[i] = (rand() / (RAND_MAX + 1.0)) * 2 - 1;
	return result;
}

/**
 * zero_reals(int n)
 * 
 * @brief this function returns a vector of 0s.
 * @param n, length of the signal
 * @return 1 if success, 0 otherwise
 */ 
double *zero_reals(int n) {
	double *result = (double*)malloc(n * sizeof(double));
	int i;
	for (i = 0; i < n; i++)
		result[i] = 0;
	return result;
}

/**
 * void abs_dft_interval(const double *signal, double *abs_power_interval, int n, int interval_start, int interval_stop)
 * @brief This is used in preprocess_core.c in preprocess-daemon
 */
void abs_dft_interval(const double *signal, double *abs_power_interval, int n, int interval_start, int interval_stop){
	
	int k;
	int coef_idx = 0;
	
	/*loop through all coefficients*/
	for (k = interval_start; k < interval_stop; k++) {
		
		/*computing registers*/
		double sumreal = 0;
		double sumimag = 0;
		int t;
		
		/*compute each terms*/
		for(t = 0; t < n; t++) {
			double angle = -2*M_PI * ((long long)t * k % n) / n;
			sumreal += signal[t]*cos(angle);
			sumimag += signal[t]*sin(angle);
		}
		
		/*compute 2*real value, to get abs fft*/
		abs_power_interval[coef_idx] = 2*sqrt(sumreal*sumreal+sumimag*sumimag)/n;
		coef_idx++;
	}
}  

