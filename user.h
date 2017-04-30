#include <string>
#include <iostream>

using namespace std;

struct User
{
    std::string id;
    int fd;
    User(std::string id_, int fd_)
    {
        id = id_;
        fd = fd_;
    }
};

void readdata(string &temp, char pac_type, string &to_addr, string from_addr, bool guiON=false, string username="", string password=""){
    string user;
    string header;
    int numbytes;
    string data;
    if(pac_type=='R' || pac_type=='L') {
        if(!guiON){
            cerr<<"Enter in the order username and password\n";
            cin>>user;
        }
        else
            user=username;
        if(user.length()>14)
            cout<<"ERROR";
        else {
            if(!guiON)
                cin>>data;
            else
                data=password;
            if(data.length()>14)
                cout<<"ERROR";
            else {
                numbytes=7+user.length()+data.length();
                header = to_string(numbytes);
                while(header.length()<4)
                    header="0"+header;
                header=header+pac_type;
                temp=header+user+"\n"+data+"\n";
                to_addr=user;
            }
        }
    }
    else {
        temp=temp+"\n";
        numbytes=6+from_addr.length()+to_addr.length()+temp.length();
        header = to_string(numbytes);
        while(header.length()<4)
            header="0"+header;
        header=header+pac_type+from_addr+to_addr+"\n";
        temp=header+temp;
    }
}
