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

#define PORT "9034"   // port we're connecting to

#define STDIN 0

using namespace std;

void readdata(string &temp, char pac_type, string to_addr, string from_addr){
    string user;
    string header;
    int numbytes;
    string data;
    if(pac_type=='R' || pac_type=='L') {
        cin>>user;
        if(user.length()>14)
            cout<<"ERROR";
        else {
            user=user+"\n";
            cin>>data;
            if(data.length()>14)
                cout<<"ERROR";
            else {
                data=data+"\n";
                numbytes=5+user.length()+data.length();
                header = to_string(numbytes);
                while(header.length()<4)
                header="0"+header;
                header=header+pac_type;
                temp=header+user+data;
            }
        }
    }
    else {
        temp=temp+"\n";
        numbytes=7+from_addr.length()+to_addr.length()+temp.length();
        header = to_string(numbytes);
        while(header.length()<4)
        header="0"+header;
        header=header+pac_type+from_addr+"\n"+to_addr+"\n";
        temp=header+temp;
    }   
}

int main(){
    struct sockaddr_in serveraddr;
    struct hostent *he;
    int curr_state;
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
    string packet,temp1,temp2;
    int num;
    cin>>curr_state;
    if(curr_state==1)    
        readdata(packet,'R',temp1,temp2);
    else
        readdata(packet,'L',temp1,temp2);
    if((num=send(sockfd,packet.c_str(),packet.length(),0))==-1){
        perror("send");
        exit(0);
    }
    while(1){
        readfds=master;
        if(select(fdmax+1,&readfds,NULL,NULL,NULL)==-1){
            perror("select");
            exit(0);
        }

        if(FD_ISSET(STDIN,&readfds)){
            char buf[100];
            int numBytes = read(STDIN,buf,1000);
            buf[numBytes]='\0';
            if(buf[0]=='3')
                curr_state=3;
            if (curr_state==3) {
                string buffer(buf);
                temp1 = "From";
                temp2 = "To";
                readdata(buffer,'M',temp1,temp2);
                if((numBytes=send(sockfd,buffer.c_str(),buffer.length(),0))==-1){
                perror("send");
                exit(0);
                }
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
