/*
 * utils.cpp
 *
 *  Created on: 22 nov. 2013
 *      Author: jfellus
 */



#include "utils.h"
#include <string.h>

string stringprintf(const char* fmt, ...) {
	char* s = new char[strlen(fmt)+512];
	va_list vl;
	va_start(vl, fmt);
	vsprintf(s, fmt, vl);
	va_end(vl);
	string ss(s);
	delete s;
	return ss;
}

void fappend(const string& filename, const string& line) {
	FILE* f = fopen(filename.c_str(), "a");
	if(!f) {
		cout << "Cant' open : " << filename << "\n";
		perror("blabla");
		throw "Can't create file";
	}
		fputs(line.c_str(), f);
	fclose(f);
}

void fappend_n(const string& filename, const string& line) {
	FILE* f = fopen(filename.c_str(), "a");
		fprintf(f, "%s", line.c_str());
	fclose(f);
}


string get_config_str(const char* what, const char* default_val) {
	FILE* f = fopen("config.properties", "r");
	char line[512];
	char* c = 0;
	string v = default_val;
	while ( fgets (line , 512 , f) != NULL ) {
	      if(!strncmp(what, line, strlen(what))) {
	    	  c = line + strlen(what);
	    	  while(*c==' ' || *c=='=') c++;
	    	  v = c;
	      }
	}
	fclose(f);
	v.erase(v.find_last_not_of(" \n\r\t")+1);
	return v;
}

bool str_ends_with(const char* s, const char* ew) {
	string ss = s;
	string eww = ew;
	if (ss.length() >= eww.length())
		return (0 == ss.compare (ss.length() - eww.length(), eww.length(), eww));
	return false;
}

string rtrim(string s) {
	s.erase(s.find_last_not_of(" \n\r\t")+1);
	return s;
}
