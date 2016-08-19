#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

char buffer[4096];
map<string,string> request;
map<string,string> reply;
int dbug=1;


bool is_file_exist(const char *fileName){
    std::ifstream infile(fileName);		//existence of file
    return infile.good();
}

int getMethod(string pch){
	if(pch.substr(0,3)=="GET"){
		request["MethodName"]="GET";
		int i=4;
		while(pch[i]!=' ')
			i++;
		if(pch[i-1]!='/')
			request["URI"]=pch.substr(5,i-5);
		else if(i!=5)
			request["URI"]=pch.substr(5,i-5)+"index.html";
		else
			request["URI"]="index.html";

		struct stat sb;
		stat(request["URI"].c_str(),&sb); 
		request["URItype"]= (S_ISREG(sb.st_mode)) ? "file" : "dir";
	}
	else
		return 501;
	if(dbug)
		cout<<request["MethodName"]<<" ** "<<request["URI"]<<endl;
	return 200;
}

int parseRequest(){
	char *pch=buffer,*savepch,*savetemp;
	pch = buffer;
	pch = strtok_r(pch,"\r\n",&savepch);
	int status=getMethod(pch);
	if(status!=200)
		return status;
	pch = strtok_r(NULL,"\r\n",&savepch);
	while(pch!=NULL){
		char *tpch=pch,*spch;
		tpch = strtok_r(tpch,": ",&savetemp);
		spch = tpch;
		tpch = strtok_r(NULL,"\r\n",&savetemp);
		if(dbug)
			printf("%s == %s\n",spch,tpch);
		request[spch]=tpch;
		pch = strtok_r(NULL,"\r\n",&savepch);
		//pch = strtok(NULL,"\r\n");
	}
	return 200;
}



int getRequest(int newfd){
		
	memset(buffer,0,sizeof(buffer));
	int bytes_recieved = recv(newfd,buffer,4096,0);
	if(bytes_recieved<=0){
		//closeConnection(newfd);
		return 0;
	}
	printf("%s\n",buffer);
	if(dbug)
		printf("Parsing\n");	
	return parseRequest();
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
	if(dbug)
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
	if(status==200 && request["URItype"]=="file"){
		printf("File Found\n");
		FILE *fp = fopen(buffer,"r");
		fseek(fp,0,SEEK_END);
		int lSize = ftell(fp);
		rewind(fp);
		
		reply["HTTP/1.1"] = "200 OK\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		char *pch=buffer;
		pch=strtok(buffer,".");
		pch=strtok(NULL,".");
		if(dbug)
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
		if(request.find("Connection")!=request.end())
			reply["Connection"] =  request["Connection"]+"\r\n";
		else
			reply["Connection"] = "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(dbug)
			printf("Header Added\n");
		sendHeader(newfd);
		sendBody(newfd,fp);
		fclose(fp);		
	}
	if(status==200 && request["URItype"]=="dir"){
		printf("Directory Found\n");
		buffer[strlen(buffer)]='/';
		DIR *d;
		struct dirent *dir;
		d=opendir(buffer);
		string body="<HEAD><TITLE>Directory</TITLE></HEAD><BODY>";
		if(d){
			while((dir=readdir(d))!=NULL){
				printf("%s\n",dir->d_name);
				string s= (string)dir->d_name;
				if(dir->d_type==DT_DIR)
					body+=("<a href=\""+ string(buffer) + s +"\">" + s +"/</a><BR>");
				else
					body+=("<a href=\""+ string(buffer) + s +"\">" + s +"</a><BR>");				
			}
			closedir(d);
		}
		body+=("</body>");
		int lSize=body.length();		
		reply["HTTP/1.1"] = "200 OK\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		reply["Content-Type"] = "text/html\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		char num[10];
		snprintf(num,10,"%d",lSize);
		reply["Content-Length"] = string(num)+"\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(request.find("Connection")!=request.end())
			reply["Connection"] =  request["Connection"]+"\r\n";
		else
			reply["Connection"] = "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(dbug)
			printf("Header Added\n");
		sendHeader(newfd);
		cout<<body<<endl;
		send(newfd,body.c_str(),lSize,0);		
	}
	else if(status==404){
		printf("File Not Found\n");		
		snprintf(buffer,4096,"<HEAD><TITLE>404 File Not Found</TITLE></HEAD><BODY><H1>404 Not Found</H1></BODY>");

		reply["HTTP/1.1"] = "404 Not Found\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		reply["Content-Type"] = "text/html\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		char num[10];
		snprintf(num,10,"%d",(int)strlen(buffer));
		reply["Content-Length"] = string(num)+"\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(request.find("Connection")!=request.end())
			reply["Connection"] =  request["Connection"]+"\r\n";
		else
			reply["Connection"] = "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(dbug)
			printf("Header Added\n");
		sendHeader(newfd);
		send(newfd,buffer,strlen(buffer),0);
	}
	else if(status==400){
		printf("Bad Request\n");
		snprintf(buffer,4096,"<HEAD><TITLE>Bad Request</TITLE></HEAD><BODY><H1>Error:400 Bad Request</H1></BODY>");
		reply["HTTP/1.1"] = "400 Bad Request\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		reply["Content-Type"] = "text/html\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		char num[10];
		snprintf(num,10,"%d",(int)strlen(buffer));
		reply["Content-Length"] = string(num)+"\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(request.find("Connection")!=request.end())
			reply["Connection"] =  request["Connection"]+"\r\n";
		else
			reply["Connection"] = "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(dbug)
			printf("Header Added\n");
		sendHeader(newfd);
		send(newfd,buffer,strlen(buffer),0);
	}
	else if(status==501){
		printf("Not Implemented\n");
		snprintf(buffer,4096,"<HEAD><TITLE>Not Implemented</TITLE></HEAD><BODY><H1>Error:501 Not Implemented</H1></BODY>");
		reply["HTTP/1.1"] = "501 Not Implemented\r\n";
		/*-----------------------------------------------------------------------------------------------*/ 
		reply["Content-Type"] = "text/html\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		char num[10];
		snprintf(num,10,"%d",(int)strlen(buffer));
		reply["Content-Length"] = string(num)+"\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(request.find("Connection")!=request.end())
			reply["Connection"] =  request["Connection"]+"\r\n";
		else
			reply["Connection"] = "Keep-Alive\r\n";
		/*-----------------------------------------------------------------------------------------------*/
		if(dbug)
			printf("Header Added\n");
		sendHeader(newfd);
		send(newfd,buffer,strlen(buffer),0);
	}
}

int sendResponse(int newfd, int stat){
	if(dbug)
		printf("Sending Response...\n");

	memset(buffer,0,sizeof(buffer));
	for(int i=0;i<request["URI"].length();i++)
		buffer[i] = request["URI"][i];

	printf("\nRequested File: (%s)\n",buffer);
	if(stat!=200){
		addHeader(newfd,stat);
	}
	else if(is_file_exist(buffer)){
		addHeader(newfd,200);										
	}
	else{
		addHeader(newfd,404);
	}
	if(request["Connection"]=="close")
		return 0;	
	return 1;
}