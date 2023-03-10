/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stan <stan@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 14:44:29 by stan              #+#    #+#             */
/*   Updated: 2023/03/10 16:10:24 by stan             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream> // for cout
#include <sys/socket.h> // for socket
#include <netinet/in.h> // for sockaddr_in
#include <cstdlib> // for exit and EXIT_FAILURE
#include <unistd.h> // for close and read
#include <map> // for map
#include <sstream> // for stringstream
#include <fstream> // for ifstream

namespace ft
{
    // Structure pour stocker les informations de la requête
    struct Request {
        std::string method;                          // méthode HTTP utilisée (GET, POST, etc.)
        std::string uri;                             // URI de la ressource demandée
        std::map<std::string, std::string> headers;  // en-têtes de la requête
        std::string body;                            // corps de la requête
    };
    
    class Server
    {
        private:
            int _socket;
            int _connect;
            struct sockaddr_in _address;
            char _buffer[1024];
            Request _request;
        public:
            Server();
            ~Server();
            
            void run();
            void create_socket();
            void listen_server();
            void response();
            Request& parse_request();
            std::string process_request(const Request& request);
            
            int getSock() const;
            int getConnect() const;
            struct sockaddr_in getAddress() const;
            std::string getMethod() const;
            std::string getUri() const;
            std::string getBody() const;
            std::map<std::string, std::string> getHeaders() const;
    };
}