#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"   // port we're connecting to

#define STDIN 0

int main(){
	struct sockaddr_in serveraddr;
	struct hostent *he;

	fd_set master;
	fd_set readfds;
	int fdmax;

	FD_ZERO(&master);
	FD_ZERO(&readfds);


	if((he=gethostbyname("localhost"))==NULL){
		perror("gethostbyname");
		exit(0);
	}


	int sockfd;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("socket");
		exit(0);
	}



	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9034);
	serveraddr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(serveraddr.sin_zero), 8);

	if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr))==-1){
		perror("connect");
		exit(0);
	}

	FD_SET(sockfd,&master);
	FD_SET(STDIN,&master);

	fdmax = sockfd;

	

	while(1){
		readfds=master;
		if(select(fdmax+1,&readfds,NULL,NULL,NULL)==-1){
			perror("select");
			exit(0);
		}

		if(FD_ISSET(STDIN,&readfds)){
			char buf[100];
			int numBytes = read(STDIN,buf,100);
			buf[numBytes]='\0';
			if((numBytes=send(sockfd,buf,strlen(buf),0))==-1){
				perror("send");
				exit(0);
			}
		}

		if(FD_ISSET(sockfd,&readfds)){
			int numBytes;
			char buf[100];
			if((numBytes=recv(sockfd,buf,sizeof buf,0))<0){
				perror("recv");
			}
			else{
				buf[numBytes]='\0';
				printf("%s",buf);
			}
		}

	}
	close(sockfd);

	return 0;
}