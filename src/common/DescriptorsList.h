/*
 * DescriptorsList.h
 *
 *  Created on: 24 janv. 2014
 *      Author: jfellus
 */

#ifndef DESCRIPTORSLIST_H_
#define DESCRIPTORSLIST_H_

#include "utils.h"
#include "matrix_double.h"
#include "matrix_float.h"


class DescriptorsList {
private:
	int n,dim;

	string filename;
	string dir;

	int nbfiles;
public:
	DescriptorsList(const char* filename);
	~DescriptorsList();

	int get_n();
	int get_dim();

	void read_all(MatrixDouble& m);
	void read_all(MatrixFloat& m);

protected:
	void read_props();
};


#endif /* DESCRIPTORSLIST_H_ */
