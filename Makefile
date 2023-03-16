all: Client Server
Server: server.o date.o responses.o my_utils.o json_utils.o my_utils.o logger.o 
	g++ -pthread server.o date.o responses.o  json_utils.o my_utils.o logger.o -lstdc++ -o Server.exe

Client: client.o my_utils.o json_utils.o logger.o
	g++ -pthread client.o my_utils.o json_utils.o logger.o  -lstdc++ -o Client.exe

client.o: client.cpp logger.h json_utils.h my_utils.h 
	g++ -c client.cpp -o client.o -lstdc++

server.o: server.cpp responses.h server.h date.h logger.h json_utils.h my_utils.h 
	g++ -c server.cpp -o server.o -lstdc++

respones.o: responses.cpp responses.h
	g++ -c respones.cpp  -lstdc++

date.o: date.cpp date.h my_utils.h
	g++ -c date.cpp  -lstdc++

logger.o : logger.cpp logger.h
	g++ -c logger.cpp -lstdc++

json_utils.o: json_utils.cpp json_utils.h
	g++ -c json_utils.cpp -o json_utils.o -lstdc++

my_utils.o: my_utils.cpp my_utils.h
	g++ -c my_utils.cpp -o my_utils.o -lstdc++


.PHONY: clean
clean:
	rm *.o 
	rm *.exe