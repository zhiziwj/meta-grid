#ifndef RAND_H
#define RAND_H
#include "core.h"
int rand_dim(){
	return rand()%(state*2);
}
double distur_rand(){
	return (double)rand()/RAND_MAX*2.0-1.0;
}
double rand01(){
	return (double)rand()/RAND_MAX;
}
#endif 
