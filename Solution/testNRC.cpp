#include "NetworkRequestChannel.h"

int main(){
	string port = "4950";
	NetworkRequestChannel server(20,port);
	
	//printf("Recieved : %s\n", server.cread().c_str());
	//server.cwrite("This is a response");
	
	printf("EXIT CWRITE");
}