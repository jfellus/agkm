/*
 * shared_mem.h
 *
 *  Created on: 9 oct. 2013
 *      Author: jfellus
 */

#include "shared_mem.h"

void* create_shared_mem(key_t key, unsigned long size, int* _shmid_out) {
	*_shmid_out = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
	if(*_shmid_out < 0) {
		std::cerr << "Maybe too large..." << size << "\n";
		throw "Unable to allocate shared memory";
	}

	void* data = shmat(*_shmid_out, 0, 0);
	if(data == (char*)-1) throw "Unable to attach to shared memory";

	return data;
}

void delete_shared_mem(void* ptr, int _shmid, const char* id) {
	detach_shared_mem(ptr);
	unlink(id);
	if(shmctl(_shmid, IPC_RMID, 0) < 0) throw "Unable to delete shared memory";
}

void* attach_shared_mem(key_t key, int* _shmid_out) {
	*_shmid_out = shmget(key, 0, 0666);
	if(*_shmid_out < 0) throw "No shared memory found with this id";

	void* data = shmat(*_shmid_out, 0, 0);
	if(data == (char*)-1) throw "Unable to attach to shared memory";

	return data;
}

void detach_shared_mem(void* ptr) {
	if(shmdt(ptr)<0) throw "Unable to detach shared memory (given pointer does not point to shared segment";
}


