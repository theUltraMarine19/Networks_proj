all:
	g++ -std=c++11 client.cpp -o client -l X11;
	g++ -std=c++11 server.cpp -o server;
