#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

char buffer[4096];
map<string,string> request;
map<string,string> reply;
int debug=1;


bool is_file_exist(const char *fileName){
    std::ifstream infile(fileName);		//existence of file
    return infile.good();
}

void getMethod(string pch){
	if(pch.substr(0,3)=="GET"){
		request["MethodName"]="GET";
		int i=4;
		while(pch[i]!=' ')
			i++;
		if(i>5)
			request["URI"]=pch.substr(5,i-5);
		else
			request["URI"]="index.html";
	}
	if(debug){
		//printf("%s ** %s\n",request["MethodName"],request["URI"] );
		cout<<request["MethodName"]<<" ** "<<request["URI"]<<endl;
	}
}

void parseRequest(){
	char *pch=buffer;
	//pch = buffer;
	pch = strtok(pch,"\r\n");
	getMethod(pch);
	pch = strtok(NULL,"\r\n");
	while(pch!=NULL){
		if(debug)
			printf("LINE %s\n",pch);
		char *tpch=pch,*spch;
		tpch = strtok(tpch,": ");
		spch = tpch;
		tpch = strtok(NULL,"\r\n");
		if(debug)
			printf("%s == %s\n",tpch,spch);
		request[spch]=tpch;
		pch = strtok(NULL,"\r\n");
		//pch = strtok(NULL,"\r\n");
	}
}

int getRequest(int newfd){
		
	memset(buffer,0,sizeof(buffer));
	int bytes_recieved = recv(newfd,buffer,4096,0);
	if(bytes_recieved<=0){
		closeConnection(newfd);
		return 0;
	}
	printf("%s\n",buffer);
	parseRequest();
	if(debug)
		printf("Parsing Done\n");									
	return 1;
}

void sendHeader(int newfd){
	string res;
	res="HTTP/1.1 "+reply["HTTP/1.1"];
	res+="Content-Type: "+reply["Content-Type"];
	res+="Content-Length: "+reply["Content-Length"];
	res+="Connection: "+reply["Connection"];
	res+="\r\n";
	cout<<res<<endl;
	char msg[4096];
	strcpy(msg,res.c_str());
	cout<<msg<<endl;
	send(newfd,msg,strlen(msg),0);
	if(debug)
		printf("Header Sent\n");
}

void sendBody(int newfd, FILE* fp){
	char body[4096];
	int size;
	while((size=fread(body,1,4096,fp))>0){
		send(newfd,body,size,0);
		memset(body,0,sizeof(body));
	}
	printf("Body Sent\n");
}

void addHeader(int newfd,int status){
	reply.clear();
	if(status==200){
		FILE *fp = fopen(buffer,"r");
		fseek(fp,0,SEEK_END);
		int lSize = ftell(fp);
		rewind(fp);
		
		reply["HTTP/1.1"] = "200 OK\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		char *pch=buffer;
		pch=strtok(buffer,".");
		pch=strtok(NULL,".");
		if(debug)
			cout<<pch<<endl;
		if(strcmp(pch,"html")==0 || strcmp(pch,"htm")==0)
			reply["Content-Type"] = "text/html\r\n";
		else if(strcmp(pch,"txt")==0)
			reply["Content-Type"] = "text/plain\r\n";
		else if(strcmp(pch,"jpeg")==0 || strcmp(pch,"jpg")==0)
			reply["Content-Type"] = "image/jpeg\r\n";
		else if(strcmp(pch,"gif")==0)
			reply["Content-Type"] = "image/gif\r\n";
		else if(strcmp(pch,"pdf")==0)
			reply["Content-Type"] = "Application/pdf\r\n";
		else
			reply["Content-Type"] = "application/octet-stream\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		char num[10];
		snprintf(num,10,"%d",lSize);
		reply["Content-Length"] = string(num)+"\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		reply["Connection"] =  "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(debug)
			printf("Header Added\n");
		sendHeader(newfd);
		sendBody(newfd,fp);
		fclose(fp);		
	}
	else if(status==404){
		reply["HTTP/1.1"] = "404 Not Found\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		reply["Content-Type"] = "text/html\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		reply["Content-Length"] = "0\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		reply["Connection"] =  "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(debug)
			printf("Header Added\n");
		sendHeader(newfd);
		//sendBody(newfd,fp);
	}
}

int sendResponse(int newfd){
	if(debug)
		printf("Sending Response...\n");

	memset(buffer,0,sizeof(buffer));
	for(int i=0;i<request["URI"].length();i++)
		buffer[i] = request["URI"][i];

	printf("\nRequested File: (%s)\n",buffer);

	if(is_file_exist(buffer)){
		printf("File Found !\n");
		addHeader(newfd,200);										
	}
	else{
		printf("File Not Found !\n");
		addHeader(newfd,404);
	}	
	return 0;
}