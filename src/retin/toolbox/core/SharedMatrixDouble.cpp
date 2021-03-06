/*
 * MatrixDouble.cpp
 *
 *  Created on: 9 oct. 2013
 *      Author: jfellus
 */

#include "SharedMatrixDouble.h"
#include "sharedmem/shared_mem.h"
#include "sharedmem/matrix_reader.h"
#include "sharedmem/matrix_writer.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>


using namespace std;
using namespace retin;


///////////
// UTILS //
///////////

static string str_replace(string subject, const string& search, const string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

static void touch(const char* file) {
	FILE* f = fopen(file,"w"); if(f) fclose(f);
}

static key_t create_key(const string& file) {
	touch(file.c_str());
	return ftok(file.c_str(), 1);
}

static const char* getcwd() {
	static char* c = NULL;
	if(c==NULL) c = new char[512];
	if(!getcwd(c, 512)) {};
	return c;
}

static string realpath(const char* file) {
	string realfile = getcwd();
	if(file[0]=='/') realfile = file;
	else {realfile += "/"; realfile += file;}
	char* rp = realpath(realfile.c_str(),NULL);
	if(rp) {
		realfile = rp;
		free(rp);
	}
	return realfile;
}

static string create_id(const char* file) {
	if(system("mkdir --parents /var/tmp/shared_matrices/")){}
	return string("/var/tmp/shared_matrices/") + str_replace(realpath(file),"/","__");
}

namespace shared_matrices {

////////////
// MATRIX //
////////////

/** Creates a new shared matrix from a file */
void MatrixDouble::load(const char* file, int flags) {
	if((flags & MATRIX_LOCAL) && (flags & MATRIX_CREATE))
		throw "MATRIX_LOCAL and MATRIX_CREATE are incompatible";

	if((flags & MATRIX_ATTACH) && attach(file)) {
		this->bShared = true;
		return;
	}

	this->bShared = ((flags & MATRIX_LOCAL)==0);

	if(flags & MATRIX_CREATE) key = create_key(id);
	else key = -1;

	if(flags & MATRIX_LOCAL || flags & MATRIX_CREATE) {
		read(file);
		if(!data) throw "Unrecognized Matrix file format";
	} else data = NULL;
}

/** Attach to an already existing shared matrix */
bool MatrixDouble::attach(const char* file) {
	width=height=0;
	data = 0;
	bOwner = false;

	this->id = create_id(file);

	key = ftok(id.c_str(), 1);
	if(key==-1) return false;

	size_t* _data = NULL;
	try{ _data = (size_t*)attach_shared_mem(key, &_shmid); }catch(...){}
	if(!_data) return false;

	width = *_data++;
	height = *_data++;
	data = (double*)_data;
	bShared = true;

	cout << "Found shared matrix : " << file << "\n";
	return true;
}

void MatrixDouble::detach() {
	 detach_shared_mem(((size_t*)data)-2);
}

void MatrixDouble::delete_shared() {
	delete_shared_mem(((size_t*)data)-2, _shmid, this->id.c_str());
}

void MatrixDouble::create_ref(double* data, size_t width, size_t height) {
	this->width = width;
	this->height = height;
	this->data = data;
}

/** Creates a new empty shared matrix of dimensions  width x height */
void MatrixDouble::create(size_t width, size_t height, const char* shared_id) {
	if(shared_id) {
		bShared = true;
		if(attach(shared_id)) {
			if(this->width==width && this->height==height) 	return;
			cout << "But matrix dimensions does not agree : reshaping ... ok\n";
			delete_shared();
		}
		key = create_key(id);
		try {
			size_t* _data = (size_t*)create_shared_mem(key, width*height*sizeof(double) + 2*sizeof(size_t), &_shmid);
			*_data++ = width;
			*_data++ = height;
			data = (double*)_data;
			//this->clear();
		} catch(const char* e) {
			data = 0;
			cerr << "Couldn't allocate shared matrix " << shared_id << " with size [" << width << "x" << height << "] (" << e << ")\n";
			unlink(id.c_str());
			throw "Couldn't allocate shared matrix";
		}
	} else {
		bShared = false;
		data = new double[width*height];
		//this->clear();
	}

	this->width = width; this->height = height;
}

/** if this instance is the owner of the shared content, delete the whole shared matrix
 *  else destroy only the local reference to the shared matrix */

/** if this instance is the owner of the shared content, delete the whole shared matrix
 *  else destroy only the local reference to the shared matrix */
MatrixDouble::~MatrixDouble() {
	dealloc();
}

void MatrixDouble::dealloc() {
	fflush(stdout);
	if(!data) return;
	if(!bShared) {
		fflush(stdout);
		delete data;
	} else {
		if(bOwner) delete_shared();
		else detach();
	}
}

void MatrixDouble::realloc(size_t w, size_t h, const char* shared_id) {
	dealloc();
	create(w,h,shared_id);
}



bool MatrixDouble::read(const char* file) {
	matrix_reader<float>* reader = 0;
	std::string file_name = file;
	if (string_has_file_ext(file_name,".fvec")) reader = new fvec_reader(file);
    else if (string_has_file_ext(file_name,".hvec8")) reader = new hvec8_reader(file);
    else if (string_has_file_ext(file_name,".csv") || string_has_file_ext(file_name,".m")) reader = new csv_reader<float>(file);
    else if (string_has_file_ext(file_name,".txt")) reader = new txt_reader<float>(file);
    else if (string_has_file_ext(file_name,".bin16")) reader = new bin_reader(file);
    else if (string_has_file_ext(file_name,".peter8")) reader = new peter8_reader(file);
    else if (string_has_file_ext(file_name,".idx3-ubyte")) reader = new idx3_ubyte_reader(file);

	if(!reader) {
		cout << "Can't open " << file << "\n";
		throw "No reader for this file format";
	}

	height = reader->get_height();
	width = reader->get_width();

	if(bShared) {
		size_t* _data = (size_t*)create_shared_mem(key, width*height*sizeof(double) + 2*sizeof(size_t), &_shmid);
		*_data++ = width;
		*_data++ = height;
		data = (double*)_data;
	} else {
		data = new double[width*height];
	}

	fflush(stdout);

	float* tmpdata = new float[width*height];
    reader->read(tmpdata);
    for(int i=0; i<width*height; i++) data[i] = tmpdata[i];
    delete tmpdata;

	delete reader;

	return true;
}

bool MatrixDouble::write(const char* file) const {
	matrix_writer<float>* writer = 0;
	std::string file_name = file;
	if (string_has_file_ext(file_name,".fvec")) writer = new fvec_writer(file);
	else if (string_has_file_ext(file_name,".hvec8")) writer = new hvec8_writer(file);
	else if (string_has_file_ext(file_name,".csv") || string_has_file_ext(file_name,".m")) writer = new csv_writer<float>(file);
	else if (string_has_file_ext(file_name,".txt")) writer = new txt_writer<float>(file);
	else if (string_has_file_ext(file_name,".ivecs")) writer = new ivecs_writer(file);
	else if (string_has_file_ext(file_name,".peter8")) writer = new peter8_writer(file);

	if(!writer) throw "No writer for this file format";

	float* tmpdata = new float[width*height];
	for(int i=0; i<width*height; i++) tmpdata[i] = data[i];
	writer->write(tmpdata, width, height);
	delete tmpdata;
	delete writer;
	return true;
}


void MatrixDouble::dump(size_t nbrows, size_t nbcols) {
	if(nbrows==0) nbrows = height;
	if(nbcols==0) nbcols = width;

	for(size_t i=0; i<nbrows; i++) {
		if(i) putc('\n',stdout);
		for(size_t j=0; j<nbcols; j++) {
			if(j) putc(' ',stdout);
			printf("%f", data[i*width+j]);
		}
		fflush(stdout);
	}
	putc('\n',stdout);
}

bool MatrixDouble::free_shared(const char* file) {
	bool b = false;
	MatrixDouble* m = new MatrixDouble(file, MATRIX_ATTACH);
	if(*m) {b=true; cout << "Free shared matrix : " << file << endl;}
	m->set_owner();
	delete m;
	return b;
}

void MatrixDouble::clear() {
	if(!data) return;
	memset(data, 0, width*height*sizeof(double));
}

void MatrixDouble::operator=(const MatrixDouble& m) {memcpy(this->data, m.data, sizeof(double)*width*height);}


}
