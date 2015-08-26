/*
 * DescriptorsList.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jfellus
 */

#include "DescriptorsList.h"
#include "utils.h"
#include "../retin/toolbox/core/sharedmem/matrix_reader.h"


DescriptorsList::DescriptorsList(const char* filename) {
	n=dim=0;
	this->filename = filename;

	char _dir[255];
	strcpy(_dir, filename);
	this->dir = dirname(_dir);
}

DescriptorsList::~DescriptorsList() {
}

int DescriptorsList::get_n() {
	if(n==0) read_props();
	return n;
}

int DescriptorsList::get_dim() {
	if(dim==0) read_props();
	return dim;
}

void DescriptorsList::read_props() {
	char line[255];

	n = dim = nbfiles = 0;

	FILE* f = fopen(filename.c_str(), "r");
	while(fgets(line, 255, f)) {
		string fn = rtrim(fmt("%s/%s",dir.c_str(),line));
		matrix_reader<float>* reader = new hvec8_reader(fn.c_str());
		n += reader->get_height();
		dim = reader->get_width();
		delete reader;
		nbfiles++;
	}
	fclose(f);
}

void DescriptorsList::read_all(MatrixDouble& m) {
	if(this->dim==0 || this->n==0) read_props();

	char line[255];
	int n=0;

	DBG("Loading ... ");

	FILE* f = fopen(filename.c_str(), "r");
	while(fgets(line, 255, f)) {
		DBG_PERCENT(n,this->n);
		string fn = rtrim(fmt("%s/%s",dir.c_str(),line));
		matrix_reader<float>* reader = new hvec8_reader(fn.c_str());

		float* tmp = new float[dim * reader->get_height()];
		reader->read(tmp);
		for(int i=0; i<reader->get_height()*dim; i++) m.data[n*dim+i] = tmp[i];
		delete tmp;

		n += reader->get_height();
		delete reader;
	}
	fclose(f);

	DBG("ok");
}

void DescriptorsList::read_all(MatrixFloat& m) {
	if(this->dim==0 || this->n==0) read_props();

	char line[255];
	int n=0;

	DBG("Loading ... ");

	FILE* f = fopen(filename.c_str(), "r");
	while(fgets(line, 255, f)) {
		DBG_PERCENT(n,this->n);
		string fn = rtrim(fmt("%s/%s",dir.c_str(),line));
		matrix_reader<float>* reader = new hvec8_reader(fn.c_str());

		float* fff = &m.data[n*dim];
		reader->read(fff);

		n += reader->get_height();
		delete reader;
	}
	fclose(f);

	DBG("ok");
}

