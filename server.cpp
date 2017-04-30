/*
 ** selectserver.cpp -- a cheezy multiperson chat server
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include "user.h"
using namespace std;
#define PORT "9034"   // port we're listening on

vector<User> activeUsers;
vector<string> registeredUsers;
map<string, vector<string> > inbox;



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void addNewUser(string buf, int fd)
{
    string response;
    char resType = 'I';
    string userId;
    string password;
    bool alreadyExists = false;
    string userIdInp = buf.substr(5,buf.find("\n")-5);
    string passwordInp = buf.substr(buf.find("\n")+1,buf.length()-buf.find("\n")-2);
    ifstream f("./userDetails.txt", ios::in );
    if(f)
    {
        while(!f.eof())
        {
            f>>userId;
            if(f.eof() || userId.length() == 0)
            {
                break;
            }
            f>>password;
            if(userId.compare(userIdInp)==0) {
                alreadyExists = true;
            }
        }
    }
    else
    {
        cout<<"could not open file\n";
    }
    f.close();
    ofstream f1("./userDetails.txt", ios::app);
    if(!alreadyExists) {
        cout<<"writing username\n"<<userIdInp;
        f1<<"\n"<<userIdInp;
        registeredUsers.push_back(userIdInp);
        cout<<"writing password\n"<<passwordInp;
        f1<<" "<<passwordInp;
        response = "Successfully registered :)\n";
        resType = 'I';
    }
    else {
        response = "User id already exists :( Try some new id\n";
        resType = 'F';
    }
    f1.close();
    string from_addr("server");
    readdata(response, resType, from_addr, userIdInp);
    if (send(fd, response.c_str(), response.length(), 0) == -1) {
        perror("send");
    }
}

void loginUser(string buf, int fd) {
    string response;
    char resType = 'I';
    string userId;
    string password;
    string userIdInp = buf.substr(5,buf.find("\n")-5);
    string passwordInp = buf.substr(buf.find("\n")+1,buf.length()-buf.find("\n")-2);
    bool userExists = false;
    ifstream f("./userDetails.txt", ios::in);
    if(f) {
        while(!f.eof()) {
            f>>userId;
            if(f.eof()) {
                break;
            }
            f>>password;
            if(userId.compare(userIdInp)==0) {
                userExists = true;
                vector<User>::iterator iter;
                for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++) {
                    if(userId.compare(iter->id) == 0) {
                        break;
                    }
                }
                if (iter != activeUsers.end()) {
                    response = "Already Logged in -_-\n";
                    resType = 'F';
                }
                else if(password.compare(passwordInp)==0) {
                    response = "Login Success\n";
                    activeUsers.push_back(User(userId, fd));
                    resType = 'I';
                }
                else {
                    response = "Wrong Password :(\n";
                    resType = 'F';
                }
            }
        }
        if(!userExists) {
            response = "Username does not exists\n";
            resType = 'F';
        }
    }
    else
    {
        cout<<"could not open file\n";
    }
    string from_addr("server");
    readdata(response, resType, from_addr, userIdInp);
    if (send(fd, response.c_str(), response.length(), 0) == -1) {
        perror("send");
    }
    f.close();
}

void logoutUser(int fd) {
    string response;
    char resType = 'I';
    vector<User>::iterator iter;
    string user;
    for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++)
    {
        if (iter->fd == fd)
        {
            user = iter->id;
            response = "Logged out. Visit Again :)\n";
            resType = 'F';
            activeUsers.erase(iter);
            break;
        }
    }
    string from_addr("server");
    readdata(response, resType, from_addr, user);
    if (send(fd, response.c_str(), response.length(), 0) == -1) {
        perror("send");
    }
}
void sendMessage(string packet) {
    int fd=1;
    string data = packet.substr(5);
    string from_addr = data.substr(0,data.find("\n"));
    data = data.substr(data.find("\n") + 1);
    string to_addr = data.substr(0,data.find("\n"));
    data = data.substr(data.find("\n") + 1);
    vector<User>::iterator iter;
    bool active = false,registered = false;
    for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++) {
        if(from_addr.compare(iter->id) == 0) {
            fd = iter->fd;
            active = true;
            break;
        }
    }

    if(!active){
        int i;
        for(i=0;i<registeredUsers.size();i++){
            if(from_addr.compare(registeredUsers[i])==0){
                registered=true;
                break;
            }
        }
        if(registered){
            inbox[from_addr].push_back(packet);
        }
    }

    if (send(fd, packet.c_str(), packet.length(), 0) == -1) {
        perror("send");
    }
}
void sendActiveList(int fd)
{
    char resType = 'I';
    string user;
    string reply("Active Users:\n");
    vector<User>::iterator iter;
    for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++) {
        reply += iter->id + "\n";
        if (iter->fd == fd) {
            user = iter->id;
        }
    }
    string from_addr("server");
    readdata(reply, resType, from_addr, user);
    if (send(fd, reply.c_str(), reply.length(), 0) == -1) {
        perror("send");
    }
}

void populateRegisteredUsers(){
    ifstream f("./userDetails.txt", ios::in );
    if(f){
        string userId,password;
        while(!f.eof())
        {
            f>>userId;
            if(f.eof() || userId.length() == 0)
            {
                break;
            }
            f>>password;
            registeredUsers.push_back(userId);
        }   
    } 
    else{
        cerr<<"Unable to open users.txt\n";
        exit(0);
    }
}

void requestInbox(int i){
//    cerr<<"Entering req inbox\n";
    vector<User>::iterator iter;
    string user;
    string reply;
    for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++) {
        if(iter->fd==i){
            user = iter->id;
            break;
        }
    }
    map<string,vector<string> >::iterator it;
    string from_addr("server");
    if((it=inbox.find(user))==inbox.end()){
        reply = "No messages left\n";
        readdata(reply,'I',from_addr,user);
        int numBytes;
        if((numBytes=send(i,reply.c_str(),reply.length(),0))==-1){
            perror("send");
        }
    }
    else{

        for(int index = 0;index<(it->second).size();it++){
            reply = (it->second)[index];           
            int numBytes;
            if((numBytes=send(i,reply.c_str(),reply.length(),0))==-1){
                perror("send");
            }
        }
        inbox.erase(it);
    }
 

/*    else{
        inbox.erase(user);
    }*/

}

int main(void)
{
    populateRegisteredUsers();
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                            (struct sockaddr *)&remoteaddr,
                            &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                                "socket %d\n",
                                inet_ntop(remoteaddr.ss_family,
                                    get_in_addr((struct sockaddr*)&remoteaddr),
                                    remoteIP, INET6_ADDRSTRLEN),
                                newfd);
                    }
                } else {
                    // handle data from a client
                    memset(&buf, 0, 256);
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                            logoutUser(i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        string temp(buf);
                        // char len[4] = {buf[0], buf[1], buf[2], buf[3]};
                        int packet_size = atoi(temp.substr(0,4).c_str());
                        cout<<"packet_size="<<packet_size<<" nbytes="<<nbytes<<'\n';
                        if (nbytes == packet_size) {
                            switch (buf[4])
                            {
                                case 'R':
                                    cout<<"adding User\n";
                                    addNewUser(temp, i);
                                case 'L':
                                    cout<<"logging in User\n";
                                    loginUser(temp, i);
                                    break;
                                case 'O':
                                    cout<<"loggin out User\n";
                                    logoutUser(i);
                                    break;
                                case 'M':
                                    cout<<"Sending Message\n";
                                    sendMessage(temp);
                                    break;
                                case 'U':
                                    cout<<"sending active list\n";
                                    sendActiveList(i);
                                    break;
                                case 'I':
                                    cout<<"Messages you have not seen\n";
                                    requestInbox(i);
                                    break;
                            };
                            // we got some data from a client
                            // for(j = 0; j <= fdmax; j++) {
                            //     // send to everyone!
                            //     if (FD_ISSET(j, &master)) {
                            //         // except the listener and ourselves
                            //         if (j != listener && j != i) {
                            //             if (send(j, buf, nbytes, 0) == -1) {
                            //                 perror("send");
                            //             }
                            //         }
                            //     }
                            // }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}
