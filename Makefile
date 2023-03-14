all: server client 
utils : utils.cpp 
	g++ utils.cpp -o utils
client : client.cpp utils.cpp
	g++  client.cpp -o client
server : server.cpp utils.cpp
	g++  server.cpp -o server
.PHONY: clean
clean:
	rm server client.