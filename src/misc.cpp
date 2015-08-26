/*
 * misc.cpp
 *
 *  Created on: 13 nov. 2013
 *      Author: jfellus
 */
#include "common/utils.h"
#include "retin/toolbox/core/sharedmem/matrix_reader.h"
#include "common/DescriptorsList.h"


void distribute_data() {
	int ndo = 0;
	for(int i=0; i<N-1; i++) {
		node[i].init(X, ndo, n/N);
		ndo += n/N;
	}
	node[N-1].init(X,ndo, n-ndo);

	float mu = 0, var = 0;
	for(int i=0; i<N; i++) mu += node[i].n;
	mu /= N;
	for(int i=0; i<N; i++) var += (node[i].n-mu)*(node[i].n-mu);
	var /= N;
	DBG("Distributed " << n << " points to " << N << " nodes (" << mu << "pts/node±" << sqrt(var) << ')');
}


void init() {
	DBG("INIT");
	DBGV(NBTHREADS);

	srand(RANDOM_SEED==-1 ? time(NULL) : RANDOM_SEED);

	system("rm -rf data/*");
	system("rm -rf plots/*");

	if(system("mkdir -p plots/mat_plotall")) {}
	if(system("mkdir -p data/mat_plotall")) {}
	if(system("mkdir -p data/x")) {}
	if(system("mkdir -p data/stats")) {}
	system(fmt("mkdir -p data/stats/evt/"));

	node = new Node[N];
}

void load_all(const char* datafile) {
	X.load(datafile, MATRIX_ATTACH);
	if(!X) {
		DescriptorsList dl(datafile);
		D = dl.get_dim();
		n = dl.get_n();

		X.create(D,n, datafile);

		dl.read_all(X);
	}
}

void load_data(const char* datafile) {
	if(str_ends_with(datafile, "descriptors.txt")) load_all(datafile);
	else X.load(datafile);
	if(LIMIT_NDATA!=-1 && X.height > LIMIT_NDATA) X.height = LIMIT_NDATA;
	n = X.height;
	D = X.width;

	distribute_data();


	DBGV(LIMIT_NDATA);
	DBGV(N);
	DBGV(D);
	DBGV(n);
}

void generate_data(const char* genstr) {
	X.create(D,n);

	int k = 1;
	sscanf(genstr, "%d",&k);

	Matrix weights(k,1); weights.rand(); weights /= weights.sum();

	Matrix* cov = new Matrix[k];
	Matrix* mu = new Matrix[k];
	for(int kk=0; kk<k; kk++) {
		cov[kk].create(D,D);
		mu[kk].create(D,1);
		rand_covariance(cov[kk], .5);
		randvec(mu[kk],D,-1,1);
	}

	for(int i=0; i<n; i++) {
		float f = frand();
		int kk;	for(kk=0; kk<k && f>0; kk++) f-= weights[kk];
		kk--;
		randvec_gaussian(X.get_row(i), mu[kk], cov[kk]);
	}

	DBG("Generated " << n << " points (D=" << D << ") from " << k << " normal laws");

	distribute_data();
}


void deinit() {
	delete[] node;
}

