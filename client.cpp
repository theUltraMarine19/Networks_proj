#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "user.h"
#include "guiInitial.h"

#define PORT "9034"   // port we're connecting to

#define STDIN 0



int main(){
    struct sockaddr_in serveraddr;
    struct hostent *he;
    int curr_state;
    fd_set master;
    fd_set readfds;
    int fdmax;
    int display;
    cin>>display;
    std::string msg="Username and password must be alphanumeric";
    while(true)
    {

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

        string packet,temp1,temp2;
        int num;
        // packet=temp1+"\n"+packet+"\n"
        do
        {
            if (display == 0)
            {
                cin>>curr_state;
                if(curr_state==1)
                    readdata(packet,'R',temp1,temp2);
                else if(curr_state==2)
                    readdata(packet,'L',temp1,temp2);
                else
                    curr_state = 0;
            }
            else if (display == 1)
            {
                init_x();
                std::string uname, passw;
                curr_state = loginPage(uname, passw, msg);
                if(curr_state==1)
                    readdata(packet,'R',temp1,temp2,true,uname,passw);
                else if(curr_state==2)
                    readdata(packet,'L',temp1,temp2,true,uname,passw);
                close_x();
            }
        }while(curr_state == 0);

        if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr))==-1){
            perror("connect");
            exit(0);
        }

        FD_SET(sockfd,&master);
        FD_SET(STDIN,&master);

        fdmax = sockfd;
        if((num=send(sockfd,packet.c_str(),packet.length(),0))==-1){
            perror("send");
            exit(0);
        }
        bool loggedOut = false;
        while(!loggedOut){
            readfds=master;
            if(select(fdmax+1,&readfds,NULL,NULL,NULL)==-1){
                perror("select");
                exit(0);
            }

            if(FD_ISSET(STDIN,&readfds)){
                char buf[100];
                int numBytes = read(STDIN,buf,1000);
                buf[numBytes]='\0';
                if(buf[0]=='3') {
                    curr_state=3;
                    temp2 = "";
                }
                else if(buf[0] == '4') {
                    string buffer;
                    readdata(buffer,'U',temp1,temp2);
                    if((numBytes=send(sockfd,buffer.c_str(),buffer.length(),0))==-1){
                        perror("send");
                        exit(0);
                    }
                }
                else if(buf[0] == '6') {
                    exit(0);
                }
                else if(buf[0] == '5') {
                    string buffer;
                    readdata(buffer,'O',temp1,temp2);
                    if((numBytes=send(sockfd,buffer.c_str(),buffer.length(),0))==-1){
                        perror("send");
                        exit(0);
                    }
                }
                else if (buf[0]== '7'){
                    string buffer;
                    readdata(buffer,'I',temp1,temp2);
                    if((numBytes=send(sockfd,buffer.c_str(),buffer.length(),0))==-1){
                        perror("send");
                        exit(0);
                    }
                else if (buf[0]== '8'){
                    string buffer;
                    readdata(buffer,'S',temp1,temp2);
                    if((numBytes=send(sockfd,buffer.c_str(),buffer.length(),0))==-1){
                        perror("send");
                        exit(0);
                    }

                }
                else if (curr_state==3 and temp2.length()!=0) {
                    string buffer(buf);
                    readdata(buffer,'M',temp1,temp2);
                    if((numBytes=send(sockfd,buffer.c_str(),buffer.length(),0))==-1){
                        perror("send");
                        exit(0);
                    }
                }
                else
                    temp2 = string(buf);
            }

            if(FD_ISSET(sockfd,&readfds)){
                int numBytes;
                char buf[256];
                if((numBytes=recv(sockfd,buf,sizeof buf,0))<0){
                    perror("recv");
                }
                else{
                    buf[numBytes]='\0';
                    int i=0, n=0;
                    switch(buf[4])
                    {
                        case 'F':
                            loggedOut = true;
                        case 'I':
                        case 'M':
                            while (buf[i] != '\0' and n < 2)
                            {
                                if (buf[i] == '\n') {
                                    n++;
                                    break;
                                }
                                i++;
                            }
                            if (buf[4] == 'F' and display == 1) {
                                std::string message(&buf[i]);
                                msg = message;
                            }
                            else
                            {
                                printf("%s", &buf[i]);
                            }
                            break;
                    };
                }
            }
        }
        close(sockfd);

    }

    return 0;
}
