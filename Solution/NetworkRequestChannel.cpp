#include "NetworkRequestChannel.h"
#include <iostream>

using namespace std;

// client-side constructor
NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const string _port_no){	
    struct addrinfo hints;
    int rv;
	server = false;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(_server_host_name.c_str(), _port_no.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and make a socket
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		perror("talker: socket");
		exit(1);
	}

	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)<0)
	{
		perror ("connect error\n");
		exit(1);
	}
}

// server-side constructor
NetworkRequestChannel::NetworkRequestChannel(const int _backlog, const string _port_no, void * (*connection_handler)(void *)){	
	struct addrinfo hints;
    int rv;
	
	server = true;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, _port_no.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		perror("listener: socket");
		exit(1);
	}

	if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) 
	{
        close(sockfd);
        perror("listener: bind");
		exit(1);
    }
	
	freeaddrinfo(servinfo);

	if (listen(sockfd, _backlog) == -1) {
        perror("listen");
        exit(1);
    }
	
    while(true) {
        addr_len = sizeof their_addr;
    	int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_len);
    	if (new_fd == -1) {
    		perror("accept");
    		continue;
    	}
    	NetworkRequestChannel * newchan = new NetworkRequestChannel(new_fd);
    	pthread_t pid;
    	pthread_create(&pid, NULL, connection_handler, newchan);
    }
}

NetworkRequestChannel::NetworkRequestChannel(int _sockfd) {
	sockfd = _sockfd;
}

NetworkRequestChannel::~NetworkRequestChannel(){
	close(sockfd);
}

string NetworkRequestChannel::cread(){
	int numbytes;
    char buf[MAXBUFLEN];
	
	recv (sockfd, buf, MAXBUFLEN, 0);
	
	printf("listener: packet is %d bytes long\n", numbytes);
	//buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);
	

	string ret(buf);
	
	return ret;
}

int NetworkRequestChannel::cwrite(string _request){
	int numbytes;
		
	if (send(sockfd, _request.c_str(), MAXBUFLEN, 0) == -1){
        perror("send");
    }
	
	printf("talker: sent %s\n", _request.c_str());
	
	return 0;
}