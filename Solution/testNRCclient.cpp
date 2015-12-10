#include <iostream>
#include "NetworkRequestChannel.h"

int main(){
	NetworkRequestChannel *client = new NetworkRequestChannel("build.tamu.edu", "4950");
	client->cwrite("Test");
	cout<<"RESPONSE:"<<client->cread()<<endl;
}