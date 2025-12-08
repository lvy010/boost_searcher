.PHONY:all
all:parser debug server

parser:parser.cpp
	g++ -o $@ $^ -std=c++11 -lboost_filesystem

debug:debug.cpp
	g++ -o $@ $^ -std=c++11 -ljsoncpp

server:server.cpp
	g++ -o $@ $^ -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -f parser debug server