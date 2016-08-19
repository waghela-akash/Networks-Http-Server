make:
	g++ -o server server.cpp
	g++ -o client client.cpp
clean:
	rm -f server
	rm -f client
	rm -f a.out