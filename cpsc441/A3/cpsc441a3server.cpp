#include <iostream>
#include <cstring>
#include <unistd.h> // for read()
#include <sys/socket.h>
#include <netinet/in.h>

char buffer[1024] = {0};

// this uses ascii to finds the vowels, if is not vowel put that in consonants and count++, if it's vowel reset count to 0 and put that in vowel string
std::pair<std::string, std::string> splitTextAdvancedEncoding(const std::string& text) {
    std::string consonants;
    std::string vowels;
    int count = 0;
    for (char c : text) {
        char lowerC = std::tolower(c);
        if (lowerC == 97 || lowerC == 101 || lowerC == 105 || lowerC == 111 || lowerC == 117) {
            vowels += std::to_string(count);
            vowels += c;
            count = 0;
        } else {
            consonants += c;
            count++;
        }
    }
    return {consonants, vowels};
}

// if it's number in vowel, with ascii the number in vowel - 0 then we get the number from it
// else we get the skip value from vowel number that we had from previous if statement, if size of text is less than 
// j then put that vowel in the text string then reset the skip to 0.
std::string mergeTextAdvancedEncoding(const std::string& consonants, const std::string& vowels) {
    std::string text = consonants;
    int i = 0;
    int j = 0;
    int skip = 0;
    while (i < vowels.size()) {
        if (isdigit(vowels[i])) {
            skip = vowels[i++] - '0';
        } else {
            j+= skip;
            if (j <= text.size()) {
                text.insert(++j, 1, vowels[i]);
            }
            skip = 0;
            i++;
        }
    }
    return text;
}

// in the text, loop the size of text and if it's vowel it adds in vowels and adds space for consonant
std::pair<std::string, std::string> splitTextBasicEncoding(const std::string& text) {
    std::string consonants;
    std::string vowels;
    for (char c : text) {
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
            vowels += c;
            consonants += ' ';
        } else {
            consonants += c;
            vowels += ' ';
        }
    }
    return {consonants, vowels};
}

// both consonants and vowels have spaces from splitText so now find the space where there is no space for consonants and vowels and put that in a text in order.
std::string mergeTextBasicEncoding(const std::string& consonants, const std::string& vowels) {
    std::string text;
    for (size_t i = 0; i < consonants.size(); ++i) {
        if (consonants[i] != ' ') {
            text += consonants[i];
        } 
        if (vowels[i] != ' ') {
            text += vowels[i];
        }
    }
    return text;
}

// receive consonant with tcp and vowel with tcp and do timeout for udp
void mergeReceivedText(int tcpSocket, int udpSocket, char* buffer, struct sockaddr_in* clientAddr, socklen_t* clientAddrLen) {
    memset(buffer, 0, 1024);
    ssize_t bytesReadTCP = read(tcpSocket, buffer, 1024);
    if (bytesReadTCP < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    std::string consonants(buffer, bytesReadTCP);
    memset(buffer, 0, 1024);
    
    struct timeval timeout;
    timeout.tv_sec = 5;  
    timeout.tv_usec = 0; 
    setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(udpSocket, &readfds);

    int maxfd = udpSocket + 1;
    int result = select(maxfd, &readfds, nullptr, nullptr, nullptr);

    if (result < 0) {
        std::cerr << "Error in select()" << std::endl;
        exit(EXIT_FAILURE);
    } else if (result == 0) {
        std::cout << "UDP socket timeout occurred" << std::endl;
    } else {
        if (FD_ISSET(udpSocket, &readfds)) {
            ssize_t bytesReadUDP = recvfrom(udpSocket, buffer, 1024, 0, (struct sockaddr*)clientAddr, clientAddrLen);
            if (bytesReadUDP < 0) {
                perror("udpError");
                exit(EXIT_FAILURE);
            }

            std::string vowels(buffer, bytesReadUDP);

            std::cout << "Received Consonants: " << consonants << std::endl;
            std::cout << "Received Vowels: " << vowels << std::endl;
            std::string mergedText = mergeTextBasicEncoding(consonants, vowels);
            std::cout << "Merged Text: " << mergedText << std::endl;
            send(tcpSocket, mergedText.c_str(), mergedText.size(), 0);
        }
    }
}

void mergeReceivedTextAdvanced(int tcpSocket, int udpSocket, char* buffer, struct sockaddr_in* clientAddr, socklen_t* clientAddrLen) {
    memset(buffer, 0, 1024);
    ssize_t bytesReadTCP = read(tcpSocket, buffer, 1024);
    if (bytesReadTCP < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    std::string consonants(buffer, bytesReadTCP);
    memset(buffer, 0, 1024);

    struct timeval timeout;
    timeout.tv_sec = 5;  
    timeout.tv_usec = 0; 
    setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(udpSocket, &readfds);

    int maxfd = udpSocket + 1;
    int result = select(maxfd, &readfds, nullptr, nullptr, nullptr);

    if (result < 0) {
        std::cerr << "Error in select()" << std::endl;
        exit(EXIT_FAILURE);
    } else if (result == 0) {
        std::cout << "UDP socket timeout occurred" << std::endl;
    } else {
        if (FD_ISSET(udpSocket, &readfds)) {
            ssize_t bytesReadUDP = recvfrom(udpSocket, buffer, 1024, 0, (struct sockaddr*)clientAddr, clientAddrLen);
            if (bytesReadUDP < 0) {
                perror("udpError");
                exit(EXIT_FAILURE);
            }

            std::string vowels(buffer, bytesReadUDP);
            std::cout << "Received Consonants: " << consonants << std::endl;
            std::cout << "Received Vowels: " << vowels << std::endl;
            std::string mergedText = mergeTextAdvancedEncoding(consonants, vowels);
            std::cout << "Merged Text: " << mergedText << std::endl;
            send(tcpSocket, mergedText.c_str(), mergedText.size(), 0);
        }
    }
}

void receviveAddress(int udpSocket, char* buffer, struct sockaddr_in* clientAddr, socklen_t* clientAddrLen) {
    memset(buffer, 0, 1024);
    
    ssize_t bytesReadUDP = recvfrom(udpSocket, buffer, 1024, 0, (struct sockaddr*)clientAddr, clientAddrLen);
    if (bytesReadUDP < 0) {
        perror("udpError");
        exit(EXIT_FAILURE);
    }
}

// Receive string from client and send back the data to client, consonants with tcp and vowels with udp
void splitReceivedText(int tcpSocket, int udpSocket, char* buffer, struct sockaddr_in* clientAddr, socklen_t* clientAddrLen) {
    memset(buffer, 0, 1024);
    ssize_t bytesRead = read(tcpSocket, buffer, 1024);
    if (bytesRead < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    std::string text(buffer, bytesRead); 
    std::cout << "Received string: " << text << std::endl;
    std::pair<std::string, std::string> splitText = splitTextBasicEncoding(text);
    std::cout << "Send Consonants: " << splitText.first << std::endl;
    std::cout << "Send Vowels: " << splitText.second << std::endl;
    send(tcpSocket, splitText.first.c_str(), splitText.first.length(), 0);
    if (sendto(udpSocket, splitText.second.c_str(), splitText.second.length(), 0, (struct sockaddr*)clientAddr, *clientAddrLen) < 0) {
        perror("sendto failed"); 
    }
}

void splitReceivedTextAdvanced(int tcpSocket, int udpSocket, char* buffer, struct sockaddr_in* clientAddr, socklen_t* clientAddrLen) {
    memset(buffer, 0, 1024);
    ssize_t bytesRead = read(tcpSocket, buffer, 1024);
    if (bytesRead < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    std::string text(buffer, bytesRead); 
    std::cout << "Received string: " << text << std::endl;
    std::pair<std::string, std::string> splitText = splitTextAdvancedEncoding(text);
    std::cout << "Send Consonants: " << splitText.first << std::endl;
    std::cout << "Send Vowels: " << splitText.second << std::endl;
    send(tcpSocket, splitText.first.c_str(), splitText.first.length(), 0);
    if (sendto(udpSocket, splitText.second.c_str(), splitText.second.length(), 0, (struct sockaddr*)clientAddr, *clientAddrLen) < 0) {
        perror("sendto failed"); 
    }
}

int connectUDP() {
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return -1;
    }

    // Bind the UDP socket to an address and port
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all available network interfaces

    int port;
    std::cout << "Enter the port number for UDP: ";
    std::cin >> port;
    serverAddress.sin_port = htons(port);      // Replace with your desired port number

    if (bind(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind UDP socket" << std::endl;
        return -1;
    }
    
    return udpSocket;
}

int connectTCP() {
    int server_fd;
    struct sockaddr_in address; // Structure containing an internet address
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        return -1;
    }

    address.sin_family = AF_INET; // Address family (IPv4)
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces

    int port;
    std::cout << "Enter the port number for TCP: ";
    std::cin >> port;

    address.sin_port = htons(port); // Port number, converted to network byte order

    // Binding the socket to the address and port number specified above
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    // Listening for client connections, with a maximum of 3 pending connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return -1;
    }

    return server_fd;
}


int main() {
    int server_fd = connectTCP();
    
    if (server_fd == -1) {
        perror("TCP Connection Error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int udpSocket = connectUDP();

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    // accepting connection from client
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        // receiving option input from client and then run each cases
        while (true) {
            memset(buffer, 0, 1024);
            ssize_t bytesReceived = recv(new_socket, buffer, 1024, 0);
            if (bytesReceived <= 0) {
                break;
            }

            int option = std::stoi(buffer);
            if (option == 5) {
                break;
            }

            switch (option) {
                case 1:
                    receviveAddress(udpSocket, buffer, &clientAddress, &clientAddressLength);
                    splitReceivedText(new_socket, udpSocket, buffer, &clientAddress, &clientAddressLength);
                    break;
                case 2:
                    mergeReceivedText(new_socket, udpSocket, buffer, &clientAddress, &clientAddressLength);
                    break;
                case 3:
                    receviveAddress(udpSocket, buffer, &clientAddress, &clientAddressLength);
                    splitReceivedTextAdvanced(new_socket, udpSocket, buffer, &clientAddress, &clientAddressLength);
                    break;
                case 4:
                    mergeReceivedTextAdvanced(new_socket, udpSocket, buffer, &clientAddress, &clientAddressLength);
                    break;
            }
        }

        close(new_socket);
    }
    close(server_fd);
    close(udpSocket);

    return 0;
}