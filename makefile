webServer:http.cpp httprequest.cpp threadpool.cpp web.cpp webServer.cpp
	g++ -g -o webServer http.cpp httprequest.cpp threadpool.cpp web.cpp webServer.cpp -lpthread -std=c++11