/*
 * math.h
 *
 *  Created on: 8 nov. 2013
 *      Author: jfellus
 */

#ifndef GOSSIP_MATH_H_
#define GOSSIP_MATH_H_


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "matrix.h"
#include "matrix_double.h"
#include "matrix_float.h"


float frand();
float frand(float min, float max);

float rand_gaussian(float mu, float sigma);
float rand_exp(float lambda);

void randvec(float* v, int n, float min, float max);
void randvec(double* v, int n, double min, double max);

void randvec_gaussian(float* v, int d);
void randvec_gaussian(float* v, MatrixFloat& mu, MatrixFloat& sigma);
void rand_covariance(MatrixFloat& cov, float s);

void randvec_gaussian(double* v, int d);
void randvec_gaussian(double* v, MatrixDouble& mu, MatrixDouble& sigma);
void rand_covariance(MatrixDouble& cov, double s);

#endif /* GOSSIP_MATH_H_ */
