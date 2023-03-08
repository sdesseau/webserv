/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stan <stan@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 14:44:29 by stan              #+#    #+#             */
/*   Updated: 2023/03/08 15:07:49 by stan             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream> // for cout
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <cstdlib> // for exit and EXIT_FAILURE
#include <unistd.h> // for close and read

namespace ft
{
    class Server
    {
        private:
            int _socket;
            int _connect;
            struct sockaddr_in _address;
            char _buffer[100];
        public:
            Server();
            ~Server();
            
            void run();
            void create_socket();
            void listen_server();
            void response();
    };
}