/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stan <stan@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 14:44:27 by stan              #+#    #+#             */
/*   Updated: 2023/03/08 15:08:02 by stan             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

ft::Server::Server()
{
    _socket = 0;
    _connect = 0;
}

ft::Server::~Server() {}

void ft::Server::create_socket()
{
    // Create a socket (IPv4, TCP)
    if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ft::Server::listen_server()
{
    // Listen to port 9999 on any address
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(9999); // htons is necessary to convert a number to network byte order
    if (bind(_socket, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
        std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start listening. Hold at most 10 connections in the queue
    if (listen(_socket, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Grab a connection from the queue
    size_t addrlen = sizeof(_address);
    _connect = accept(_socket, (struct sockaddr*)&_address, (socklen_t*)&addrlen);
    if (_connect < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ft::Server::response()
{
    // Read from the connection
    read(_connect, _buffer, 100);
    std::cout << "The message was: " << _buffer;

    // Send a message to the connection
    std::string response = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 16\n\n<h1>testing</h1>";
    send(_connect, response.c_str(), response.size(), 0);
}

void ft::Server::run()
{
    create_socket();
    listen_server();
    response();

    // Close the connections
    close(_connect);
    close(_socket);
}

int main()
{
    ft::Server serv;
    serv.run();
    return 0;
}