# makefile

all: dataserver client

test: nrc.o
	g++ -std=c++11 -g -o testServ testNRC.cpp nrc.o
	g++ -std=c++11 -g -o testClient testNRCclient.cpp nrc.o

nrc.o: NetworkRequestChannel.h NetworkRequestChannel.cpp
	g++ -std=c++11 -c -g NetworkRequestChannel.cpp -o nrc.o

semaphore.o: semaphore.cpp semaphore.h
	g++ -std=c++11 -c -g semaphore.cpp -lpthread

boundedbuffer.o: boundedbuffer.cpp boundedbuffer.h
	g++ -std=c++11 -c -g boundedbuffer.cpp

dataserver: dataserver.cpp nrc.o 
	g++ -std=c++11 -g -o dataserver dataserver.cpp nrc.o -lpthread

client: client.cpp nrc.o boundedbuffer.o semaphore.o
	g++ -std=c++11 -g -o client client.cpp nrc.o boundedbuffer.o semaphore.o -lpthread

clean:
	rm *.o dataserver client
