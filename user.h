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
