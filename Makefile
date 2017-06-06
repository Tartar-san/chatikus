all: main.cpp client.cpp
	g++ main.cpp -o server
	g++ client.cpp -o client


clean:
	rm -rf *.o server client
