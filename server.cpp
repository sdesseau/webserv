/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stan <stan@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/08 14:44:27 by stan              #+#    #+#             */
/*   Updated: 2023/03/15 02:00:50 by stan             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "utils.cpp"
#include "config.hpp"

// Fonction pour analyser la requête HTTP reçue
ft::Request& ft::Server::parse_request() {
    std::stringstream request_stream;                                                                                   
    request_stream << _buffer;                                                                      
    std::string line;                                                                               
    bool headers_done = false;                                                                      
    while (std::getline(request_stream, line) && !line.empty()) {                                   
        if (!headers_done) {                                                                        
            if (line.find(":") != std::string::npos) {                                              
                std::string header_name = line.substr(0, line.find(":"));                           
                std::string header_value = line.substr(line.find(":") + 1);                         
                _request.headers[header_name] = header_value;                                       
            } else {                                                                                
                headers_done = true;                                                                
                _request.method = line.substr(0, line.find(" "));                                   
                _request.uri = line.substr(line.find(" ") + 1, line.rfind(" ") - line.find(" ") - 1); 
            }
        } else {                                                                                    
            _request.body += line + "\n";                                                           
        }
    }
    return (_request);                                                                                  
}

std::string search_content_type(std::string filename)
{
    std::string content_type;
    int i = filename.length();
    if (filename[i - 1] == 'l' && filename[i - 2] == 'm' && filename[i - 3] == 't' && filename[i - 4] == 'h' && filename[i - 5] == '.')
        content_type = "text/html";
    else if (filename[i - 1] == 'g' && filename[i - 2] == 'p' && filename[i - 3] == 'j' && filename[i - 4] == '.')
        content_type = "image/jpeg";
    else if (filename[i - 1] == 's' && filename[i - 2] == 's' && filename[i - 3] == 'l' && filename[i - 4] == 'e' && filename[i - 5] == 'c' && filename[i - 6] == '.')
        content_type = "text/css";
    else if (filename[i - 1] == 's' && filename[i - 2] == 's' && filename[i - 3] == 'e' && filename[i - 4] == 'r' && filename[i - 5] == 't' && filename[i - 6] == '.')
        content_type = "text/javascript";
    else if (filename[i - 1] == 'p' && filename[i - 2] == 'h' && filename[i - 3] == 'p')
        content_type = "application/x-httpd-php";
    else if (filename[i - 1] == 'f' && filename[i - 2] == 'i' && filename[i - 3] == 'g')
        content_type = "image/x-icon";
        // else if pdf
    else if (filename[i - 1] == 'f' && filename[i - 2] == 'd' && filename[i - 3] == 'p')
        content_type = "application/pdf";
   else
        content_type = "text/plain";
    return (content_type);
}

std::string body_test = "<h1>Ceci est un test</h1>";

std::string ft::Server::process_request(const ft::Request& request) 
{
    std::cout << "Method: " << request.method << std::endl;
    std::string response;
    if (request.method == "GET")
    {
        // Traitement de la requête GET
        if (request.uri == "/")
        {
            // std::cout << "URI: " << request.uri << std::endl;
            std::string body = read_file("index.html");
            std::string body_size = std::to_string(body.size());
            response = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nContent-Length: " + body_size + "\r\n\r\n" + body;
        }
        else
        {
            std::cout << "URI: " << request.uri << std::endl;
            // Si l'URI est différent de "/", renvoyer le fichier correspondant
            std::string filename = request.uri.substr(1); // Supprimer le premier caractère "/"
            std::string body = read_file(filename);
            if (body == "") {
                // Si le fichier n'existe pas, renvoyer une réponse 404
                std::cout << "Check filename: " << filename << std::endl;
                response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            }
            else
            {
                std::string content_type = search_content_type(filename);
                std::string body_size = std::to_string(body.size());
                response = "HTTP/1.1 200 OK\r\nContent-Type:";
                response += content_type;
                response += "\r\nContent-Length: " + body_size + "\r\n\r\n" + body;
            }
        }
    }
    else if (request.method == "DELETE")
    {
        // Traitement de la requête DELETE
        // check authorisations
        response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    }
    else if (request.method == "POST")
    {
        // Traitement de la requête POST
        response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    }
    else if (request.method == "PUT")
    {
        // Traitement de la requête PUT
        response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
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
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
        std::cout << "Failed to set SO_REUSEADDR. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket created" << std::endl;
    if ((ioctl(_socket, FIONBIO, (char *)&optval)) < 0) {
        std::cout << "Failed to ioctl. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    if (bind(_socket, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
        std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Bind done" << std::endl;
    // Start listening. Hold at most 10 connections in the queue
    if (listen(_socket, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Waiting for incoming connections..." << std::endl;
}

void ft::Server::response()
{
    std::cout << "Waiting for incoming connections..." << std::endl;
    // Grab a connection from the queue
    size_t addrlen = sizeof(_address);
    _connect = accept(_socket, (struct sockaddr*)&_address, (socklen_t*)&addrlen);
    if (_connect < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
    std:: cout << "Connection accepted" << std::endl;
    // Parse the request
    // parse_request();
    std::cout << "Method: " << _request.method << std::endl;
    std::cout << "URI: " << _request.uri << std::endl;
    std::cout << "Body: " << _request.body << std::endl;
    
    // Process the request
    std::string response_str = process_request(_request);

    // Send the response to the client
    send(_connect, response_str.c_str(), response_str.size(), 0);
    close(_connect);

}

int multi_send(int fd, std::string response)
{
    while ((response.size() / 4096) > 1)
    {
        std::cout << "Sending multi send response" << std::endl;
        if ((send(fd, response.substr(0, 4096).c_str(), 4096, 0)) < 0)
            return (-1);
        response = response.substr(4096);
    }
    return (1);
}

// create poll instance
void ft::Server::create_poll()
{
    int end_server = 0, compress_array = 0;
    int new_sd = -1;
    int close_conn;
    int current_size, rc, nfds = 1;
    struct pollfd fds[200];
    
    memset(fds, 0, sizeof(fds));
    fds[0].fd = _socket;
    fds[0].events = POLLIN;
    int timeout = (3 * 60 * 1000);
    do
    {
        printf("Waiting on poll()...\n");
        if ((rc = poll(fds, nfds, timeout)) < 0)
        {
            perror("  poll() failed");
            break;
        }
        if (rc == 0)
        {
            printf("  poll() timed out.  End program.\n");
            break;
        }
        current_size = nfds;
        for (int i = 0; i < current_size; i++)
        {
            if (fds[i].revents == 0)
                continue;
            if (fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
            //     end_server = 1;
                // break;
            }
            if (fds[i].fd == _socket)
            {
                printf("  Listening socket is readable\n");
                do
                {
                    new_sd = accept(_socket, 0, 0);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            end_server = 1;
                        }
                        break;
                    }
                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } while (new_sd != -1);
            }
            else
            {
                printf("  Descriptor %d is readable\n", fds[i].fd);
                close_conn = 0;
                do
                {
                    rc = recv(fds[i].fd, _buffer, sizeof(_buffer), 0);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  recv() failed");
                            close_conn = 1;
                        }
                        break;
                    }
                    if (rc == 0)
                    {
                        printf("  Connection closed\n");
                        close_conn = 1;
                        break;
                    }
                    _request = parse_request();
                    // Process the request
                    std::string response_str = process_request(_request);
                    // Send the response to the client
                    std::cout << "response_str len : " << response_str.size() << std::endl;
                    if (response_str.size() > 4096)
                    {
                        if ((multi_send(fds[i].fd, response_str)) < 0)
                        {
                            perror("  send() failed");
                            close_conn = 1;
                            break;
                        }
                    }
                    else if (send(fds[i].fd, response_str.c_str(), response_str.size(), 0) < 0)
                    {
                        perror("  send() failed");
                        close_conn = 1;
                        break;
                    }
                } while (1);
                if (close_conn)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = 1;
                }
            }
        }
        if (compress_array)
        {
          compress_array = 0;
          for (int i = 0; i < nfds; i++)
          {
            if (fds[i].fd == -1)
            {
              for(int j = i; j < nfds; j++)
              {
                fds[j].fd = fds[j+1].fd;
              }
              i--;
              nfds--;
            }
          }
        }    
        
        
    } while ( end_server == 0);
    for (int i = 0; i < nfds; i++)
    {
      if(fds[i].fd >= 0)
        close(fds[i].fd);
    }
}

void ft::Server::run(int argc, char **argv)
{
    std::string tmp;
    if (argc == 1)
    {
        tmp = "config/default.conf";
        std::cout << "[⊛] => " <<"Using default config file: " << tmp << std::endl;
    }
    else
        tmp = argv[1];
    // check if argv is a valid file
    if (access(tmp.c_str(), F_OK) == -1)
    {
        std::cout << "Config file not found" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Checking config file..." << std::endl;
    create_socket();
    std::cout << "Socket created" << std::endl;
    listen_server();
    std::cout << "Server listening" << std::endl;
    create_poll();
    // while (1)
    //     response();
    std::cout << "Response sent" << std::endl;

    // Close the connections
    // close(_connect);
    close(_socket);
}

int ft::Server::getSock() const { return (_socket); }

int ft::Server::getConnect() const { return (_connect); }

struct sockaddr_in ft::Server::getAddress() const { return (_address); }


int main(int argc, char **argv)
{
    ft::Server serv;
    serv.run(argc, argv);
    return 0;
}