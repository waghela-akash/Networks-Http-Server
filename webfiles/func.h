#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "server.h"
using namespace std;

int PORT=3490;
char num[10];
char tempbuffer[4096];
char buffer[4096];
FILE *fsocket;

bool is_file_exist(const char *fileName){
    std::ifstream infile(fileName);		//existence of file
    return infile.good();
}

/*
function for writing on socket the contents of buffer
*/
int write(int msglen){ 
	int off=0;
	while(off<msglen){
		int bytes=fwrite(&buffer[off],1,msglen-off,fsocket);
		if(bytes<1){
			perror("Can't Write");
			exit(1);
		}
		off+=bytes;
	}
	fflush(fsocket);
	return off;
}

/*
function for reading on socket the contents of buffer
*/
int read(int msglen){
	int off=0;
	cout<<msglen<<endl;
	memset(buffer,0,sizeof(buffer));
	while(off<msglen){
		int bytes=fread(&buffer[off],1,msglen-off,fsocket);
		if(bytes<1){
			off=0; break;
		}
		off+=bytes;
	}
	fflush(fsocket);
	return off;
}