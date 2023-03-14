all: server client 
client : client.cpp
	g++  client.cpp -o client
server : server.cpp
	g++  server.cpp -o server
.PHONY: clean
clean:
	rm server client