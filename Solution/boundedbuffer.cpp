#include "boundedbuffer.h"
#include <iostream>
using namespace std;

BoundedBuffer::BoundedBuffer(int _size) :
	full(0), empty(_size), mutex(1)
{
	buffer = std::queue<std::string>();
	back = 0;
}

BoundedBuffer::~BoundedBuffer() {
	//delete buffer;
}

void BoundedBuffer::push(std::string item) {
	empty.P();	// check that buffer has empty space
	mutex.P();	// acquire lock
	buffer.push(item);	// puts item in buffer, then incs back
	mutex.V();	// release lock
	full.V();	// signal that item has been added
}

std::string BoundedBuffer::pull() {
	full.P();	// check that buffer has item
	mutex.P();	// acquire lock
	std::string item = buffer.front();	// decs back, then takes item
	buffer.pop();
	mutex.V();	// release lock
	empty.V();	// signal that item has been removed
	return item;
}

// test code for BoundedBuffer

/*
void * produce(void * buf) {
	BoundedBuffer * buffer = (BoundedBuffer *) buf;
	cout << "Pushing 74 onto buffer...\n";
	buffer->push(74);
}

void * consume(void * buf) {
	BoundedBuffer * buffer = (BoundedBuffer *) buf;
	cout << "Pulling from buffer...\n";
	int item = buffer->pull();
	cout << "Got " << item << endl;
}

int main() {
	pthread_t ptid[5];
	pthread_t ctid[5];
	BoundedBuffer buffer(3);	// buffer can hold 3 items
	for (int i = 0; i < 5; ++i) {
		pthread_create(&ptid[i], NULL, &produce, &buffer); 
		pthread_create(&ctid[i], NULL, &consume, &buffer);
	}
	for (int i = 0; i < 5; ++i) {
		pthread_join(ptid[i], NULL); 
		pthread_join(ctid[i], NULL);
	}
}
*/