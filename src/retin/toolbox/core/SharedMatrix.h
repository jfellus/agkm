/*
 * Matrix.h
 *
 *  Created on: 9 oct. 2013
 *      Author: jfellus
 */

#ifndef SHAREDMATRIX_H_
#define SHAREDMATRIX_H_

#include <stdlib.h>
#include <string>
#include <string.h>
#include "string_retin.h"

using namespace std;

// MATRIX LOADING FLAGS
#define MATRIX_CREATE 0x0001
#define MATRIX_LOCAL 0x0010
#define MATRIX_ATTACH 0x0100

namespace shared_matrices {

class Matrix {
public:
	size_t width, height;
private:
	bool bOwner, bShared;
	std::string id;

	int _shmid;
	key_t key;
public:
	float* data;

public:
	Matrix() {width=height=0; data=0; bOwner=bShared=false; _shmid=-1;}
	Matrix(const string& file, int flags = MATRIX_LOCAL | MATRIX_ATTACH) {load(file.c_str(), flags);}
	Matrix(const char* file, int flags = MATRIX_LOCAL | MATRIX_ATTACH) {load(file, flags);}
	Matrix(size_t w, size_t h, const char* shared_id = NULL) {create(w,h,shared_id);}
	Matrix(size_t w, size_t h, const string& shared_id) {create(w,h,shared_id.c_str());}

	/** if this instance is the owner of the shared content, delete the whole shared matrix
	 *  else destroy only the local reference to the shared matrix */
	virtual ~Matrix();


	void dealloc();
	void realloc(size_t w, size_t h, const char* shared_id = NULL);



	size_t get_width() const {return width;}
	size_t get_height() const {return height;}

	bool is_allocated() const {return data!=0;}
	bool is_owner() const {return bOwner;}
	bool is_shared() const {return bShared;}


	float* get_row(size_t i) {return data + i*width;}

	/** Make this instance the owner of the shared matrix (i.e. when destroyed, the shared matrix itself
	 * is deleted */
	void set_owner(bool bOwner = true) {
		this->bOwner = bOwner;
	}

	bool read(const char* file);
	bool write(const char* file) const;
	inline bool save(const char* file) const {return this->write(file);}
	inline bool save(const string& file) const {return this->write(file.c_str());}

	void dump(size_t nbrows=0, size_t nbcols=0);

	void clear();

	operator bool() const { return is_allocated(); }
	operator float*() const {return data; }
	float& operator[](int i) {return data[i]; }
	void operator=(const Matrix& m);
	void operator=(const int* data) {for(int i=0; i<width*height; i++) this->data[i] = data[i];}
	void operator=(float x) {for(int i=0; i<width*height; i++) this->data[i] = x;}


	static bool free_shared(const char* file);
	static bool free_shared(const string& file) {return free_shared(file.c_str());}

	void create(size_t w, size_t h, const char* shared_id = NULL);
	void create_ref(float* data, size_t w, size_t h);
	void load(const char* file, int flags = MATRIX_LOCAL);

protected:
	bool attach(const char* file);
	void detach();
	void delete_shared();
};

}

#endif /* SHAREDMATRIX_H_ */
