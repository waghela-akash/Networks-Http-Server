#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "server.h"
#include "response.h"
using namespace std;

int PORT=3490;

int main(int argc, char *argv[]){
	
	PORT=atol(argv[1]);
	
	int sockfd=bindSocket(PORT);

	while(1){
		int newfd = acceptConnection(sockfd);
		if(newfd>0 && fork()==0){
			while(getRequest(newfd)){
				sendResponse(newfd);
				printf("Response Sent\n");
			}
			exit(0);			
		}
		sleep(10);		
	}
	return 0;
}
