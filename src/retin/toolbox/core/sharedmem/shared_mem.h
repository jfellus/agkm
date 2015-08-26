/*
 * shared_mem.h
 *
 *  Created on: 9 oct. 2013
 *      Author: jfellus
 */

#ifndef SHARED_MEM_H_
#define SHARED_MEM_H_

#include <sys/shm.h>
#include <iostream>
#include <unistd.h>


void* create_shared_mem(key_t key, unsigned long size, int* _shmid_out);
void delete_shared_mem(void* ptr, int _shmid, const char* id);
void* attach_shared_mem(key_t key, int* _shmid_out);
void detach_shared_mem(void* ptr);



#endif /* SHARED_MEM_H_ */
