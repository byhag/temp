/* 
    File: simpleclient.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2013/01/31

    Simple client main program for MP3 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <climits>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <vector>
#include <map>
#include <chrono>
#include <sstream>

#include "reqchannel.h"
#include "boundedbuffer.h"
#include "NetworkRequestChannel.h"

#ifndef PORT
#define PORT "4601"
#endif

using namespace std;


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* struct work_thread_data{
	int thread_no;
	string channel;
	BoundedBuffer* in_buffer;
	map<string, BoundedBuffer*> out_buffers;
}; */

struct req_thread_data{
	int thread_no;
	int num_req;
	BoundedBuffer* out_buffer;
	string name;
};

struct stat_thread_data{
	int thread_no;
	int num_req;
	BoundedBuffer* in_buffer;
	string out; //actually meant to be used by stat thread as output
};

struct event_thread_data {
	int num_chans;
	int num_requests;
	BoundedBuffer * req_buffer;
	vector<NetworkRequestChannel*> channels;
	map<string, BoundedBuffer*> out_buffers;
};

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void* event_handler(void* ptr) {
	event_thread_data * data = (event_thread_data *) ptr;
	BoundedBuffer * buffer = data->req_buffer;
	vector<NetworkRequestChannel*> channels = data->channels;
	
	fd_set sockfds;
	FD_ZERO(&sockfds);	// initial clear
	int i;
	//cout<<"In Event Handler"<<endl;
	int big_chan = INT_MIN;
	int req_to_send = data->num_requests, req_to_recieve = data->num_requests;

	
	for (i = 0; i < data->num_chans; ++i) {
		if(req_to_send > 0){
			channels[i]->cwrite("data " + buffer->pull());
			--req_to_send;
		}
		FD_SET(channels[i]->sock_fd(), &sockfds);
		big_chan = (big_chan < channels[i]->sock_fd()) ? channels[i]->sock_fd() : big_chan;		
	}
		
	while (req_to_recieve > 0) {
		int num_ready = select(big_chan+1, &sockfds, NULL, NULL, NULL);
		//cout<<"Selected"<<endl;
		for(auto chan : channels){
			if(FD_ISSET(chan->sock_fd(), &sockfds)){
				//cout<<"Found Response"<<endl;
				--num_ready;
				--req_to_recieve;
				string name, response;
				stringstream ss(chan->cread());
				ss>>response;
				ss.get();
				getline(ss, name);
				data->out_buffers[name]->push(response);
				if(req_to_send > 0){
					string s = "data "+buffer->pull();
					//cout<<s<<endl;
					chan->cwrite(s);
					--req_to_send;
				}
			}else{
				FD_SET(chan->sock_fd(), &sockfds);
				//cout<<"added empty read to fds"<<endl;
			}
		}
	}
	
	
	pthread_exit(0);
	
	return NULL;
}
/* unused worker thread function
void* worker_logic(void* ptr){
	work_thread_data* data = (work_thread_data*) ptr;
	
	RequestChannel chan(data->channel, RequestChannel::CLIENT_SIDE);
	
	for(;;){
		std::string message = data->in_buffer->pull();
		
		if(message == "quit"){
			break;
		}
				
		string reply = chan.send_request("data " + message);
		cout << "Reply to request 'data "+ message +"' from thread[" + to_string(data->thread_no) + "] is '" + reply + "'\n";
		data->out_buffers[message]->push(reply);
	}
	
	string reply = chan.send_request("quit");
	cout << "Reply to request 'quit' from thread[" + to_string(data->thread_no) + "] is '" + reply + "'\n";
	
	pthread_exit(0);
	
	return NULL;
} */

void* request_logic(void* ptr){
	req_thread_data* data = (req_thread_data*) ptr;
	//cout<<"In Request thread"<<endl;
	for(; data->num_req > 0; --data->num_req){
		data->out_buffer->push(data->name);
	}
	
	pthread_exit(0);
	return NULL;
}

void* stat_logic(void* ptr){
	stat_thread_data* data = (stat_thread_data*)ptr;
	map<int, int> histogram;
	
	int div_size = 20;
	
	for(; data->num_req > 0; --data->num_req){
		int reply = stoi(data->in_buffer->pull());
		
		reply /= div_size;
		
		if(histogram.find(reply) == histogram.end())
			histogram[reply] = 0;
		++histogram[reply];
	}
	
	data->out = " Result Histogram :\n";
	for(auto i : histogram){
		data->out += to_string(i.first*div_size) + " - " + to_string((i.first+1)*div_size-1) + " : " + to_string(i.second) + "\n";
	}
	
	pthread_exit(0);
	return NULL;
}

void print_usage() {
	cout << "Usage: ./client -n <number of data requests per person> ";
	cout << "-b <bounded buffer size> -w <number of request channels> ";
	cout << "-p <port number>\n";
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
	int num_requests = 1;
	int buf_size = 256;
	int num_channels = 2;
	string port_no = "4950";
	string host_name = "build.tamu.edu";

	if (argc > 11 || argc%2 == 0) {
		print_usage();
		return 1;
	}
	for (int i = 1; i < argc-1; i++) {
		if (string(argv[i]) == "-n") {
			num_requests = atoi(argv[++i]);
			if (num_requests < 1) {
				cout << "Invalid number of data requests.\n";
				return 1;
			}
		}
		else if (string(argv[i]) == "-b") {
			buf_size = atoi(argv[++i]);
			if (buf_size < 1) {
				cout << "Invalid buffer size.\n";
				return 1;
			}
		}
		else if (string(argv[i]) == "-w") {
			num_channels = atoi(argv[++i]);
			if (num_channels < 1) {
				cout << "Invalid number of worker threads.\n";
				return 1;
			}
		}
		else if (string(argv[i]) == "-h") {
			host_name = argv[++i];
		}
		else if (string(argv[i]) == "-p") {
			port_no = argv[++i];
			if (stoi(port_no) < 1) {
				cout << "Invalid port number.\n";
				return 1;
			}
		}
		else {
			cout << "Invalid flag.\n";
			print_usage();
			return 1;
		}
	}

	cout << "num_channels = " << num_channels << endl;
	cout << "buf_size = " << buf_size << endl;
	cout << "num_requests = " << num_requests << endl;

	
	sleep(1);
	
	auto start = chrono::high_resolution_clock::now();

	/* -- Initial Variables -- */
		BoundedBuffer *req_buffer = new BoundedBuffer(buf_size);
		map<string, BoundedBuffer*> stat_buffers;
		
		std::vector<std::string> names;
		names.push_back("Joe Smith");
		names.push_back("Jane Doe");
		names.push_back("Samwise Gamgee");
		
		for(string i : names)
			stat_buffers[i] = new BoundedBuffer(buf_size);
		
		pthread_t r_threads[names.size()];
		req_thread_data r_data[names.size()];
		
		pthread_t ehtid;
		
		pthread_t s_threads[names.size()];
		stat_thread_data s_data[names.size()];
	/* -- End variables -- */
	
	
	
	//Establish control connection 
	cout << "CLIENT STARTED:" << endl;

	// cout << "Establishing control channel... " << flush;
	// NetworkRequestChannel chan(host_name, port_no);
	// cout << "done." << endl;;
	
	/* -- Set up stat buffers -- */
	cout << "Setting up stat buffers...";
	for(int i = 0; i < names.size(); ++i){
		s_data[i].thread_no = i;
		s_data[i].num_req = num_requests;
		s_data[i].in_buffer = stat_buffers[names[i]];
		
		pthread_create(&s_threads[i], NULL, stat_logic, (void*)&s_data[i]);
	}
	cout << "done\n";
	
	/* -- Start sending a sequence of requests */
	
	cout << "Creating request channels...";
	// create request channels
	vector<NetworkRequestChannel*> channels;
	for (int i = 0; i < num_channels; ++i) {
		//string reply = chan.send_request("newthread");
		channels.push_back(new NetworkRequestChannel(host_name, port_no));
	}
	cout << "done\n";
	
	// create event handler thread
	event_thread_data e_data;
	e_data.channels = channels;
	e_data.req_buffer = req_buffer;
	e_data.num_chans = num_channels;
	e_data.num_requests = num_requests*3;
	e_data.out_buffers = stat_buffers;

	pthread_create(&ehtid, NULL, event_handler, (void*)&e_data);
	
	//create request threads
	for(int i = 0; i < names.size(); ++i){
		r_data[i].thread_no = i;
		r_data[i].out_buffer = req_buffer;
		r_data[i].num_req = num_requests;
		r_data[i].name = names[i];
		
		pthread_create(&r_threads[i], NULL, request_logic, (void*)&r_data[i]);
	}

	//wait for requests to end
	for(int i = 0; i < names.size(); ++i){
		pthread_join(r_threads[i], NULL);
	}
	
	//wati for event handler to close
	pthread_join(ehtid, NULL);
	
	// close request channels
	for (int i = 0; i < num_channels; ++i) {
		channels[i]->send_request("quit");
	}	
	cout << "Closed request channels\n";
	
	//close primary communication
	// string reply = chan.send_request("quit");
	// cout << "Reply to request 'quit' FROM MAIN THREAD is '" << reply << "'" << endl;		
	
	//wait for stat threads
	for(int i = 0; i < names.size(); ++i){
		pthread_join(s_threads[i], NULL);
	}
	
	for(int i = 0; i < names.size(); ++i){
		cout<<names[i]<<s_data[i].out;
	}

	auto end = chrono::high_resolution_clock::now();
	cout << "Time elapsed = " << chrono::duration_cast<chrono::microseconds>(end-start).count();
	cout << " microseconds\n";

}
