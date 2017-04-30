Authors
=======
- Aadhavan Nambhi M
- Arijit Pramanik
- Hari Prasad V
- Jeyasoorya S

Requirements
============
c++11, openLDAP, X11 library

Citations
=========
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html - For client and server code
http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/hi.c - Sample X11 code for GUI
https://access.redhat.com/documentation/en-US/Red_Hat_Directory_Server/8.2/html/Administration_Guide/Examples-of-common-ldapsearches.html - LDAP authentication
http://pubs.opengroup.org/onlinepubs/009695399/functions/popen.html - popen


Files
=====

client.cpp
----------
This file contains the code relating to the client.
It first establishes a socket and then sends the login/registration details to the server, running on a localhost as an example.
The login has two options, either a terminal based one or a GUI based one.
The GUI has been written using the X11 library.
STDIN has also been added to the list of file descriptors, apart from the socket file descriptor(fd).
Using FDSET function, we get to know the fd on which there is data.
Suppose the fd is STDIN, then the input is read and parsed accordingly and the made into a packet using the readdata fn, and sent to the server.
Suppose the fd is the SocketFD, then the data from the server is displayed to the user after parsing.
The whole procedure is repeated until the user exits.

server.cpp
----------
This file contains the code relating to the client.
First we listen for connections on a port, and as soon as there is a new connection, the user and his port is added to the list of active users.
We also listen for data on a FD, if there is some data, then it is parsed and an associated function is called to handle it.
The associated functions are addNewUser, throughLDAP, loginUser, logoutUser, sendActiveList, populateRegisteredUsers and requestInbox.
Validation is done during login and registration.
Login has two options, either through a local database or through LDAP. LDAP query is sent using popen to cs252lab.cse.iitb.ac.in
The server also stores the messages to a user who is not active, and sends it to him when he is active and requests for his inbox.

guiInitial.h
------------
The file has the code for GUI.
It is a very basic modification of the sample code of X11.
We have created our own textbox, capable of taking in backspaces, by handling the KeyPress event to our advantage.
We also created a very simple button, by handling buttonPress and its location.
The fonts and their sizes have been changed to make it more legible.
Currently it consists only the login page.

user.h
------
The file consists of the User class used for storing user details, and the readdata function.
The readdata fn is a multipurpose function. It can create a packet with the given packet type, from addr, to addr and the data.
It can also create Login/Registration packets by taking username and password as input from the user.

userDetails.txt
---------------
This is the database of registered users for the server.
It is assumed to be accessible only to the server.
It contains a list of user entries, where each row is "<username> <password>"

Packet Formats
==============

Client to Server:

login packet
------------
<TotalSize>L<Username>
<Password>

registration packet
-------------------
<TotalSize>U<Username>
<Password>

message packet
--------------
<TotalSize><PacketType><fromAddr><toAddr>
<message>
PacketType can take:
'U' - ActiveUsers
'M' - Message
'O' - Logout
'I' - Inbox

Server to Client:

message packet
--------------
<TotalSize><PacketType><fromAddr><toAddr>
<message>
PacketType can take:
'I' - Information
'F' - Failure
