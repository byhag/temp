#include "semaphore.h"
#include <iostream>
using namespace std;

Semaphore::Semaphore(int _count) {
	m = PTHREAD_MUTEX_INITIALIZER;	// default initailizing attributes
	c = PTHREAD_COND_INITIALIZER;
	if (_count >= 0) { 
		counter = _count;
	}
	else {
		cout << "Invalid Semaphore initialization\n";
	}
}

Semaphore::~Semaphore() {
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&c);
}

int Semaphore::P() {
	pthread_mutex_lock(&m);	// lock the crit section
	while (counter <= 0) {
		pthread_cond_wait(&c, &m);	// yield to other threads
	}
	if (counter > 0) {
		--counter;
	}
	pthread_mutex_unlock(&m);
}

int Semaphore::V() {
	pthread_mutex_lock(&m);	// lock the crit section
	++counter;
	pthread_mutex_unlock(&m);
	pthread_cond_signal(&c);  // signal other threads to wake
}

