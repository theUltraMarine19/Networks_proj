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
#include "user.h"

#define PORT "9034"   // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
std::vector<User> activeUsers;

void addNewUser(char buf[256])
{
    std::string userId;
    std::string password;
    std::fstream f("~/networksproj/userDetails.txt", std::ios::in | std::ios::app);
    if(!f)
    {
        while(!f.eof())
        {
            f>>userId;
            if(f.eof() || userId.length() == 0)
            {
                break;
            }
            f>>password;
            int i;
            for (i = 0; i < userId.length(); ++i) {
                if(userId.c_str()[i] != buf[5+i]) {
                    break;
                }
            }
            if(userId.length() != 0 && i == userId.length())
                std::cout<<"already exists";
        }
        const int MAX_LEN = 15;
        std::cout<<"writing username\n";
        for (int i = 0; i < MAX_LEN; ++i)
        {
            f<<buf[5+i];
        }
        std::cout<<"writing password\n";
        f<<'\n';
        for (int i = 0; i < MAX_LEN; ++i)
        {
            f<<buf[20+i];
        }
        f<<'\n';
    }
    else
    {
        std::cout<<"could not open file\n";
    }
    f.close();
}

void loginUser(char buf[256], int fd) {
    std::string userId;
    std::string password;
    bool userExists = false;
    std::fstream f("~/networksproj/userDetails.txt", std::ios::in | std::ios::app);
    if(!f) {
        while(!f.eof()) {
            f>>userId;
            if(f.eof()) {
                break;
            }
            f>>password;
            int i;
            for (i = 0; i < userId.length(); ++i) {
                if(userId.c_str()[i] != buf[5+i]) {
                    break;
                }
            }
            if(i == userId.length())
            {
                userExists = true;
                for (i = 0; i < password.length(); ++i) {
                    if(password.c_str()[i] != buf[5+i]) {
                        break;
                    }
                }
                if(i == password.length()) {
                    std::cout<<"Login Success\n";
                    activeUsers.push_back(User(userId, fd));
                }
                else {
                    std::cout<<"Wrong Password\n";
                }
                break;
            }
        }
        if(!userExists) {
            std::cout<<"Username does not exists\n";
        }
    }
    else
    {
        std::cout<<"could not open file\n";
    }
    f.close();
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
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        char len[4] = {buf[0], buf[1], buf[2], buf[3]};
                        int packet_size = atoi(len);
                        std::cout<<"packet_size="<<packet_size<<" nbytes="<<nbytes<<'\n';
                        if (nbytes == packet_size) {
                            switch (buf[4])
                            {
                                case 'R':
                                    std::cout<<"adding User\n";
                                    addNewUser(buf);
                                case 'L':
                                    std::cout<<"logging in User\n";
                                    loginUser(buf, i);
                                    break;
                            };
                            // we got some data from a client
                            for(j = 0; j <= fdmax; j++) {
                                // send to everyone!
                                if (FD_ISSET(j, &master)) {
                                    // except the listener and ourselves
                                    if (j != listener && j != i) {
                                        if (send(j, buf, nbytes, 0) == -1) {
                                            perror("send");
                                        }
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}
