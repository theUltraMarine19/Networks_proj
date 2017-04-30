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

map<string, string> inbox;
map<string, string> outbox;


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
std::vector<User> activeUsers;

void addNewUser(std::string buf, int fd)
{
    std::string response;
    char resType = 'I';
    std::string userId;
    std::string password;
    bool alreadyExists = false;
    std::string userIdInp = buf.substr(5,buf.find("\n")-5);
    std::string passwordInp = buf.substr(buf.find("\n")+1,buf.length()-buf.find("\n")-2);
    std::ifstream f("./userDetails.txt", std::ios::in );
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
        std::cout<<"could not open file\n";
    }
    f.close();
    std::ofstream f1("./userDetails.txt", std::ios::app);
    if(!alreadyExists) {
        std::cout<<"writing username\n"<<userIdInp;
        f1<<"\n"<<userIdInp;
        std::cout<<"writing password\n"<<passwordInp;
        f1<<" "<<passwordInp;
        response = "Successfully registered :)\n";
        resType = 'I';
    }
    else {
        response = "User id already exists :( Try some new id\n";
        resType = 'F';
    }
    f1.close();
    std::string from_addr("server");
    readdata(response, resType, from_addr, userIdInp);
    if (send(fd, response.c_str(), response.length(), 0) == -1) {
        perror("send");
    }
}

void loginUser(std::string buf, int fd) {
    std::string response;
    char resType = 'I';
    std::string userId;
    std::string password;
    std::string userIdInp = buf.substr(5,buf.find("\n")-5);
    std::string passwordInp = buf.substr(buf.find("\n")+1,buf.length()-buf.find("\n")-2);
    bool userExists = false;
    std::ifstream f("./userDetails.txt", std::ios::in);
    if(f) {
        while(!f.eof()) {
            f>>userId;
            if(f.eof()) {
                break;
            }
            f>>password;
            if(userId.compare(userIdInp)==0) {
                userExists = true;
                std::vector<User>::iterator iter;
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
        std::cout<<"could not open file\n";
    }
    std::string from_addr("server");
    readdata(response, resType, from_addr, userIdInp);
    if (send(fd, response.c_str(), response.length(), 0) == -1) {
        perror("send");
    }
    f.close();
}

void logoutUser(int fd) {
    std::string response;
    char resType = 'I';
    std::vector<User>::iterator iter;
    std::string user;
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
    std::string from_addr("server");
    readdata(response, resType, from_addr, user);
    if (send(fd, response.c_str(), response.length(), 0) == -1) {
        perror("send");
    }
}
void sendMessage(std::string packet) {
    int fd=1;
    std::string data = packet.substr(5);
    std::string from_addr = data.substr(0,data.find("\n"));
    data = data.substr(data.find("\n") + 1);
    std::string to_addr = data.substr(0,data.find("\n"));
    data = data.substr(data.find("\n") + 1);
    std::vector<User>::iterator iter;
    for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++) {
        if(from_addr.compare(iter->id) == 0) {
            fd = iter->fd;
            break;
        }
    }
    inbox[to_addr] = packet;
    outbox[from_addr] = packet;
    if (send(fd, packet.c_str(), packet.length(), 0) == -1) {
        perror("send");
    }
}
void sendActiveList(int fd)
{
    char resType = 'I';
    std::string user;
    std::string reply("Active Users:\n");
    std::vector<User>::iterator iter;
    for (iter = activeUsers.begin(); iter != activeUsers.end(); iter++) {
        reply += iter->id + "\n";
        if (iter->fd == fd) {
            user = iter->id;
        }
    }
    std::string from_addr("server");
    readdata(reply, resType, from_addr, user);
    if (send(fd, reply.c_str(), reply.length(), 0) == -1) {
        perror("send");
    }
}

int main(void)
{
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
    std::memset(&hints, 0, sizeof hints);
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
                    std::memset(&buf, 0, 256);
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
                        std::string temp(buf);
                        // char len[4] = {buf[0], buf[1], buf[2], buf[3]};
                        int packet_size = atoi(temp.substr(0,4).c_str());
                        std::cout<<"packet_size="<<packet_size<<" nbytes="<<nbytes<<'\n';
                        if (nbytes == packet_size) {
                            switch (buf[4])
                            {
                                case 'R':
                                    std::cout<<"adding User\n";
                                    addNewUser(temp, i);
                                case 'L':
                                    std::cout<<"logging in User\n";
                                    loginUser(temp, i);
                                    break;
                                case 'O':
                                    std::cout<<"loggin out User\n";
                                    logoutUser(i);
                                    break;
                                case 'M':
                                    std::cout<<"Sending Message\n";
                                    sendMessage(temp);
                                    break;
                                case 'U':
                                    std::cout<<"sending active list\n";
                                    sendActiveList(i);
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
