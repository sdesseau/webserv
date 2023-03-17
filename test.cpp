Server() : client_messages_fds(), listen_socket_fd(-1), epoll_fd(-1) { }

void Run()
    {
        setup_connexion_queue(CONNEXION_PORT);
        create_epoll_instance();
        try
        {
            while (true)
                wait_for_events();
        }
        catch(const ClientMessageStopException &e)
        {
            std::cout << "Client commanded server to stop... so the server will stop. This is not a very secure server ._." << std::endl;
        }
}

//Creates a socket, binds it, listens to it and returns the fd of the socket.
    // void setup_connexion_queue(int port)
    // {
    //     //create socket
    //     if ((listen_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    //         throw ServerException("failed to create the connexion socket: ");
    //     //set socket to non blocking
    //     if (fcntl(listen_socket_fd, F_SETFL, O_NONBLOCK) == -1)
    //         throw ServerException("failed fcntl call on the connexion socket: ");
    //     //set sock option SO_REUSEADDR to prevent TCP related error 'Adress already in use' when restartingint
    //     int dump = 1;
    //     if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &dump, sizeof(int)) == -1)
    //         throw ServerException("failed to set connexion_socket SO_REUSEADDR option : ");
    //     //binding of socket to port
    //     sockaddr_in_t sock_addr_for_connexion;
    //     //Using ft_memset before setting the variables we care about abeginllows us to set all the variable we don't care about to zero in one line.
    //     ft_memset(&sock_addr_for_connexion, 0, sizeof(sockaddr_in_t));
    //     sock_addr_for_connexion.sin_family = AF_INET;                       //define adress family, in this case, IPv4
    //     sock_addr_for_connexion.sin_addr.s_addr = htonl(INADDR_ANY);        //define IP adress, in this case, localhost
    //     sock_addr_for_connexion.sin_port = htons(port);                     //define port
    //     if (bind(listen_socket_fd, (const struct sockaddr *)&sock_addr_for_connexion, sizeof(sockaddr_in_t)) == -1)
    //         throw ServerException("failed to bind the queue connexion to port: ");
    //     //start listening for clients connexion requests
    //     if (listen(listen_socket_fd, MAX_QUEUE_SIZE) == -1)
    //         throw ServerException("failed to listen the connexion socket: ");
    //     std::cout << "listening..." << std::endl;
    // }

    void create_epoll_instance()
    {
        if ((epoll_fd = epoll_create1(0)) == -1)
            throw ServerException("failed to create epoll instance: ");
        epoll_event listen_epoll_event;
        listen_epoll_event.events = EPOLLIN;
        listen_epoll_event.data.fd = listen_socket_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket_fd, &listen_epoll_event) == -1)
            throw ServerException("epoll_ctl raised an error: ");
        std::cout << "Created epoll instance." << std::endl;
    }

     void wait_for_events()
    {
        int nb_events = epoll_wait(epoll_fd, events_buffer, MAX_EPOLL_EVENTS_TO_HANDLE_AT_ONCE, -1);
        if (nb_events == -1)
            throw ServerException("failed to wait for epoll events: ");
        for (int i = 0; i < nb_events; i++)
        {
            if (events_buffer[i].data.fd == listen_socket_fd)
                accept_new_connexion();
            else
                read_client_message(events_buffer[i].data.fd);
        }
    }

    //Accepts new connexion, adds the new socket file descriptor to both the open_fds vector and the epoll instance interset list.
    void accept_new_connexion()
    {
        //accept new socket_fd
        epoll_event data_exchage_epoll_event;
        if ((data_exchage_epoll_event.data.fd = accept(listen_socket_fd, NULL, NULL)) == -1)
            throw ServerException("failed to accept new client connexion, error: ");
        //set new element values
        data_exchage_epoll_event.events = EPOLLIN;
        //add new epoll_event to the epoll instance interest list
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data_exchage_epoll_event.data.fd, &data_exchage_epoll_event) == -1)
            throw ServerException("failed to add new client connexion to epoll instance, error: ");
        client_messages_fds.push_back(data_exchage_epoll_event.data.fd);
        //log action
        std::cout << "Accepted new connexion, scoket_fd = " << data_exchage_epoll_event.data.fd << std::endl;
    }

     void read_client_message(int client_message_socket_fd)
    {
        std::string client_msg = recv_all(client_message_socket_fd);
        //if connexion was closed by client, close socket_fd, delete from epoll instance's interest list and erase it from client_messages_fds
        if (client_msg.length() == 0)
            close_connexion(client_message_socket_fd);
        else//read_bytes is neither -1 nor 0, which means that there is an actual message to broadcast.
            handle_client_message(client_message_socket_fd, client_msg);
    }

    void close_connexion(int client_message_socket_fd)
    {
        //make sure to delete socket from interest list before closing the scoket
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_message_socket_fd, NULL) == -1)
            throw ServerException("failed to delete client connexion from epoll instance, error: ");
        if (close(client_message_socket_fd) == -1)
            throw ServerException("failed to close client_message_socket_fd: ");
        std::vector<int>::iterator iterator_in_opened_fds = find(client_messages_fds.begin(), client_messages_fds.end(), client_message_socket_fd);
        client_messages_fds.erase(iterator_in_opened_fds);
        //log action
        std::cout << "Closed connections with client." << std::endl;
    }

    void handle_client_message(int client_message_socket_fd, std::string client_msg)
    {
        //log action
        std::cout << "client message: " << client_msg ;
        //Check for "cariage return"(\r\n), not just \n.
        if (client_msg == "stop\r\n")
            throw ClientMessageStopException();
        //Write message on each sockect in the client_messages_fds vector, except the one that we read from. 
        for (size_t i = 0; i < client_messages_fds.size(); i++)
            if (client_messages_fds[i] != client_message_socket_fd)
                sendall(client_messages_fds[i], client_msg.data(), client_msg.length());
    }

     void sendall(int s, const char *buf, int len)
    {
        int nb_bytes_sent_in_total = 0; // how many bytes we've sent
        int bytesleft = len;            // how many we have left to send
        int n;

        while(nb_bytes_sent_in_total < len)
        {
            if ((n = send(s, buf + nb_bytes_sent_in_total, bytesleft, MSG_DONTWAIT)) == -1 && errno != EWOULDBLOCK && errno != EAGAIN) 
                throw ServerException("failed to send client message: ");
            nb_bytes_sent_in_total += n;
            bytesleft -= n;
        }
    }