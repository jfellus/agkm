/*
 * matrix_writer.cpp
 *
 *  Created on: 10 oct. 2013
 *      Author: jfellus
 */

#include "matrix_writer.h"



///////////////////
// peter8_writer //
///////////////////

void peter8_writer::write(const float* buffer, size_t w, size_t h) {
		uint16_t x = 0;

		// header
		x = h;
		out.write((const char*)&x,sizeof(x));
		if (out.fail())
			throw std::runtime_error("Input stream error 1");

		x = w;
		out.write((const char*)&x,sizeof(x));
		if (out.fail())
			throw std::runtime_error("Input stream error 2");

		// descriptors
		for (size_t i=0;i<h;i++) {
			const float* row = buffer + i*w;
			x = row[0];
			out.write((const char*)&x,sizeof(x));
			x = row[1];
			out.write((const char*)&x,sizeof(x));
			for (size_t k=2;k<w;k++) {
				uint8_t b = row[k];
				out.write((const char*)&b,sizeof(b));
			}
			if (out.fail())
				throw std::runtime_error("Input stream error 3");
		}

}


//////////////////
// hvec8_writer //
//////////////////

void hvec8_writer::write(const float* buffer, size_t w, size_t h) {
	uint8_t* temp = 0;
	size_t n = w*h;
	temp = new uint8_t[n];
	for (size_t i=0;i<n;i++) {
		float x = buffer[i];
		if (x < 0 || x > 255) {
			std::cout << "Index: " << i << ", value: " << x << std::endl;
			throw std::runtime_error("Values outside [0,255]");
		}
		temp[i] = (uint8_t)x;
	}
	writer.write(temp, w, h);
	delete[] temp;
}


//////////////////
// ivecs_writer //
//////////////////

void ivecs_writer::write(const float* buffer, size_t w, size_t h) {
    uint32_t* temp = NULL;
    size_t n = w*h;
    temp = new uint32_t[n];
    for (size_t i=0;i<n;i++) {
        float x = buffer[i];
        if (x < 0 || x > 0xFFFFFFFF) {
            std::cout << "Index: " << i << ", value: " << x << std::endl;
            throw std::runtime_error("Values outside range");
        }
        temp[i] = (uint32_t)x;
    }
    writer.write(temp, w, h);
    delete[] temp;
}

