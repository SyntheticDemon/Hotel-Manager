all : server client

server : server.cpp 
	g++ -o server.o  server.cpp 
client:
	g++ client.cpp -o client.o

.PHONEY: clean

clean:
	rm *.o

