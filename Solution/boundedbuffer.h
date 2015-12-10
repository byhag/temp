#include "semaphore.h"
#include <string>
#include <queue>
using namespace std;

class BoundedBuffer {
	
private:
	Semaphore full;		// keeps track of filled spots
	Semaphore empty;	// keeps track of empty spots
	Semaphore mutex;	// locks crit section

	std::queue<std::string> buffer;
	int back; 		// keeps track of where the last item is

public:
	BoundedBuffer(int _size);
	~BoundedBuffer();

	void push(std::string n);
	std::string pull();
};