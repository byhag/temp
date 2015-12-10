#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN 100

using namespace std;

class NetworkRequestChannel{	
	private:
		int sockfd;
		struct addrinfo *servinfo;
		struct sockaddr_storage their_addr;
		socklen_t addr_len;
		bool server;
	public:
		

		NetworkRequestChannel(const string _server_host_name, const string _port_no);
		NetworkRequestChannel(const int _backlog, const string _port_no, void * (*connection_handler) (void *));
		NetworkRequestChannel(int _sockfd);	// for newthreads
		~NetworkRequestChannel();
		
		string cread();
		int cwrite(string _msg);
		int sock_fd() { return sockfd; }
		string send_request(string _request){
			cwrite(_request);
			return cread();
		};
};