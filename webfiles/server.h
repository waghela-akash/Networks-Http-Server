#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;

FILE *fsocket;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

int bindSocket(int PORT){
	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd == -1){
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(server_addr.sin_zero),'0',8);

	// Check if could bind the specified port
	if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr))==-1){
		perror("Unable to bind");
		exit(1);
	}

	// Limit on number of sequential connects
	if(listen(sockfd,10)==-1){
		perror("Listen");
		exit(1);
	}

	printf("\nListening on Port %d \n\n",PORT);
	fflush(stdout);
	return sockfd;
}

int acceptConnection(int sockfd){
	unsigned int sin_size = sizeof(struct sockaddr_in);
	int newfd = accept(sockfd,(struct sockaddr *)&client_addr,&sin_size);
	//fsocket = fdopen(newfd,"w+");

	printf("\nNew Client CONNECTED %s %d\n\n",
		inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
	return newfd;
}

int closeConnection(int sockfd){
	//printf("\nCONNECTION CLOSED\n");
	close(sockfd);
	printf("\nCONNECTION CLOSED %s %d\n\n",
	inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
}