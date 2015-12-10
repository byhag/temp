/* 
    File: dataserver.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/16

    Dataserver main program for MP3 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#ifndef PORT
#define PORT "4601"
#endif

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "NetworkRequestChannel.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;
static int backlog = 20;
static string port = "4950";

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void handle_process_loop(NetworkRequestChannel & _channel);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

void * handle_data_requests(void * args) {

  NetworkRequestChannel * data_channel =  (NetworkRequestChannel*)args;

  // -- Handle client requests on this channel. 
  
  handle_process_loop(*data_channel);

  // -- Client has quit. We remove channel.
 
  delete data_channel;
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(NetworkRequestChannel & _channel, const string & _request) {
  _channel.cwrite("hello to you too");
}

void process_data(NetworkRequestChannel & _channel, const string &  _request) {
  usleep(1000 + (rand() % 5000));
  //_channel.cwrite("here comes data about " + _request.substr(4) + ": " + int2string(random() % 100));
  stringstream ss(_request);
  string ignore, name;
  ss >> ignore;
  ss.get();
  getline(ss, name);
  _channel.cwrite(int2string(rand() % 100) + " " + name);
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(NetworkRequestChannel & _channel, const string & _request) {

  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(_channel, _request);
  }
  else if (_request.compare(0, 4, "data") == 0) {
    process_data(_channel, _request);
  }
  else {
    _channel.cwrite("unknown request");
  }

}

void handle_process_loop(NetworkRequestChannel & _channel) {

	for(;;) {
	
		string request = _channel.cread();

		if (request.compare("quit") == 0) {
			_channel.cwrite("bye");
			usleep(10000);          // give the other end a bit of time.
			break;                  // break out of the loop;
		}
		
		process_request(_channel, request);
	}
	
}

void print_usage() {
  cout << "Usage: ./dataserver -p <port number> ";
  cout << "-b <backlog size>\n";
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

	if (argc > 5 || argc%2 == 0) {
		print_usage();
		return 1;
	}
	for (int i = 1; i < argc-1; i++) {
		if (string(argv[i]) == "-p") {
			port = argv[++i];
			if (stoi(port) < 1) {
				cout << "Invalid port number.\n";
				return 1;
			}
		}
		else if (string(argv[i]) == "-b") {
			backlog = atoi(argv[++i]);
			if (backlog < 1) {
				cout << "Invalid backlog size.\n";
				return 1;
			}
		}
		else {
			cout << "Invalid flag.\n";
			print_usage();
			return 1;
		}
	}

  //  cout << "Establishing control channel... " << flush;
  NetworkRequestChannel control_channel(backlog, port, handle_data_requests);
  //  cout << "done.\n" << flush;


}
