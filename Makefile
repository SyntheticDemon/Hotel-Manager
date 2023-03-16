all: Client Server
Server: server.o date.o responses.o my_utils.o json_utils.o my_utils.o logger.o 
	g++ -std=c++11 -I ./include server.o date.o responses.o  json_utils.o my_utils.o logger.o -lstdc++ -o Server.exe

Client: client.o my_utils.o json_utils.o logger.o
	g++ -std=c++11 client.o my_utils.o json_utils.o logger.o  -lstdc++ -o Client.exe

client.o: client.cpp logger.h json_utils.h my_utils.h 
	g++ -std=c++11 -c client.cpp -o client.o -lstdc++

server.o: server.cpp responses.h server.h date.h logger.h json_utils.h my_utils.h 
	g++ -std=c++11 -c server.cpp -o server.o -lstdc++

respones.o: responses.cpp responses.h
	g++ -std=c++11 -c respones.cpp  -lstdc++

date.o: date.cpp date.h my_utils.h
	g++ -std=c++11 -c date.cpp  -lstdc++

logger.o : logger.cpp logger.h
	g++ -std=c++11 -c logger.cpp -lstdc++

json_utils.o: json_utils.cpp json_utils.h
	g++ -std=c++11 -c json_utils.cpp -o json_utils.o -lstdc++

my_utils.o: my_utils.cpp my_utils.h
	g++ -std=c++11 -c my_utils.cpp -o my_utils.o -lstdc++


.PHONY: clean
clean:
	rm *.o 
	rm *.exe