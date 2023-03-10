/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stan <stan@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 14:44:27 by stan              #+#    #+#             */
/*   Updated: 2023/03/10 17:04:14 by stan             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "utils.cpp"

// Fonction pour analyser la requête HTTP reçue
ft::Request& ft::Server::parse_request() {
    std::stringstream request_stream;                                                                   // création d'un flux pour stocker la requête reçue
    ssize_t n = recv(_connect, _buffer, sizeof(_buffer), 0);                                              // réception des données depuis la socket
    if (n > 0) {                                                                                        // si des données ont été reçues
        request_stream << _buffer;                                                                      // stockage des données dans le flux
        std::string line;                                                                               // création d'une variable pour stocker chaque ligne de la requête
        bool headers_done = false;                                                                      // variable pour indiquer si les en-têtes ont été traitées ou non
        while (std::getline(request_stream, line) && !line.empty()) {                                   // parcours de chaque ligne de la requête
            if (!headers_done) {                                                                        // si les en-têtes n'ont pas encore été traités
                if (line.find(":") != std::string::npos) {                                              // si la ligne correspond à un en-tête
                    std::string header_name = line.substr(0, line.find(":"));                           // extraction du nom de l'en-tête
                    std::string header_value = line.substr(line.find(":") + 1);                         // extraction de la valeur de l'en-tête
                    _request.headers[header_name] = header_value;                                       // stockage de l'en-tête dans la structure Request
                } else {                                                                                // si la ligne correspond à la première ligne de la requête (méthode, URI, protocole)
                    headers_done = true;                                                                // on indique que les en-têtes ont été traités
                    _request.method = line.substr(0, line.find(" "));                                   // extraction de la méthode HTTP
                    _request.uri = line.substr(line.find(" ") + 1, line.rfind(" ") - line.find(" ") - 1); // extraction de l'URI
                }
            } else {                                                                                    // si les en-têtes ont été traités
                _request.body += line + "\n";                                                           // on stocke le corps de la requête
            }
        }
    }
    return (_request);                                                                                  // on retourne la structure Request remplie avec les informations de la requête
}


std::string body_test = "<h1>Ceci est un test</h1>";

std::string ft::Server::process_request(const ft::Request& request) 
{
    std::string response;
    if (request.method == "GET")
    {
        // Traitement de la requête GET
        if (request.uri == "/")
        {
            std::string body = read_file("index/index.html");
            std::string body_size = std::to_string(body.size());
            response = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nContent-Length: " + body_size + "\r\n\r\n" + body;
        }
        else
        {
            // Si l'URI est différent de "/", renvoyer le fichier correspondant
            std::string filename = request.uri.substr(1); // Supprimer le premier caractère "/"
            std::string body = read_file(filename);
            if (body == "") {
                // Si le fichier n'existe pas, renvoyer une réponse 404
                response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            }
            else
            {
                if (filename == "lol")
                {
                    body = read_file("index/image.jpeg");
                    std::string body_size = std::to_string(body.size());
                    response = "HTTP/1.1 200 OK\r\nContent-Type:image/jpeg\r\nContent-Length: " + body_size + "\r\n\r\n" + body;
                }
                else if (filename == "subject")
                {
                    body = read_file("index/subject.pdf");
                    std::string body_size = std::to_string(body.size());
                    response = "HTTP/1.1 200 OK\r\nContent-Type:application/pdf\r\nContent-Length: " + body_size + "\r\n\r\n" + body;
                }
                else
                {
                    std::string body_size = std::to_string(body_test.size());
                    response = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nContent-Length: " + body_size + "\r\n\r\n" + body_test;
                    body_test += "test";
                }
            }
        }
    }
    else
    {
        // Requête non prise en charge
        response = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
    }
    return (response);
}

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

    // Set SO_REUSEADDR option on the socket
    int optval = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cout << "Failed to set SO_REUSEADDR. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
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
    // Parse the request
    parse_request();
    std::cout << "Method: " << _request.method << std::endl;
    std::cout << "URI: " << _request.uri << std::endl;
    std::cout << "Body: " << _request.body << std::endl;
    
    // Process the request
    std::string response_str = process_request(_request);

    // Send the response to the client
    send(_connect, response_str.c_str(), response_str.size(), 0);

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

int ft::Server::getSock() const { return (_socket); }

int ft::Server::getConnect() const { return (_connect); }

struct sockaddr_in ft::Server::getAddress() const { return (_address); }


int main()
{
    ft::Server serv;
    while (1)
        serv.run();
    return 0;
}