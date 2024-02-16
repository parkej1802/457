#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <limits>

char buffer[1024] = {0};

// send this to give the address of client side
void sendAddressUDP(int udpSocket, const struct sockaddr_in& udpServerAddress) {
    if (sendto(udpSocket, "", 0, 0, (struct sockaddr*)&udpServerAddress, sizeof(udpServerAddress)) < 0) {
        perror("sendto failed"); 
    }
}

// user sending string to server for split text action
void sendTextSplit(int sock) {
    std::string text;
    std::cout << "Enter a string: ";
    std::cin.ignore(); 
    std::getline(std::cin, text);

    send(sock, text.c_str(), text.length(), 0);
}

// sending consonants and vowels to server, consonant over tcp and vowels with udp
void sendTextMerge(int tcpSocket, int udpSocket, const struct sockaddr_in& udpServerAddress) {
    std::string consonants, vowels;
    std::cout << "Enter consonants: ";
    std::cin.ignore();
    std::getline(std::cin, consonants);
    std::cout << "Enter vowels: ";
    std::cin.ignore();
    std::getline(std::cin, vowels);

    send(tcpSocket, consonants.c_str(), consonants.length(), 0);
    if (sendto(udpSocket, vowels.c_str(), vowels.length(), 0, (struct sockaddr*)&udpServerAddress, sizeof(udpServerAddress)) < 0 ) {
        perror("sendto failed"); 
    }
}

// receving split text, 
void receivSplitText(int tcpSocket, int udpSocket, char* buffer, struct sockaddr_in& udpServerAddress) {
    // Clear buffer first and read sent data from server and save in buffer
    memset(buffer, 0, 1024);
    ssize_t bytesReadTCP = read(tcpSocket, buffer, 1024);
    if (bytesReadTCP < 0) {
        perror("TCP read error");
        exit(EXIT_FAILURE);
    }

    // get consonants values and print and then reset the buffer again for another use
    std::string consonants(buffer, bytesReadTCP);
    std::cout << "Consonants: " << consonants << std::endl;
    memset(buffer, 0, 1024);

    // Timeout for udp connection
    struct timeval timeout;
    timeout.tv_sec = 5;  // Set a timeout of 5 seconds
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
            ssize_t bytesReadUDP = recvfrom(udpSocket, buffer, 1024, 0, nullptr, nullptr);
            if (bytesReadUDP < 0) {
                perror("UDP receive error");
                exit(EXIT_FAILURE);
            }

            std::string vowels(buffer, bytesReadUDP);
            std::cout << "Vowels: " << vowels << std::endl;
        }
    }
}

// receiving merged text from server 
void receiveMergeText(int sock, char* buffer) {
    memset(buffer, 0, 1024);
    ssize_t bytesRead = read(sock, buffer, 1024);
    if (bytesRead < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    std::string merged(buffer, bytesRead);
    std::cout << "Mereged Text: " << merged << std::endl;

}

int connectTCP(std::string ipAddress) {
    int sock = 0; // File descriptor for the socket
    struct sockaddr_in serv_addr; // Structure containing the server address

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket creation error" << std::endl;
        return -1;
    }

    int port;
    std::cout << "Enter the port number for TCP: ";
    std::cin >> port;

    serv_addr.sin_family = AF_INET; // Address family (IPv4)
    serv_addr.sin_port = htons(port); // Port number to connect to, converted to network byte order
    if (inet_pton(AF_INET, ipAddress.c_str(), &serv_addr.sin_addr) <= 0) { // Converting IPv4 and IPv6 addresses from text to binary form
        std::cerr << "Invalid address" << std::endl;
        return -1;
    } 


    // Connecting to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection TCP Failed" << std::endl;
        return -1;
    }
    return sock;
}

int connectUDP(struct sockaddr_in* serverAddress, std::string ipAddress) {
    // Creating udpSocket
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return -1;
    }

    int port;
    std::cout << "Enter the port number for UDP: ";
    std::cin >> port;

    // Set the server's IP address and port
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(port);  // The server's port number
    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddress->sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return -1;
    }

    return udpSocket;
}

int main() {
    // Connect TCP if it fails terminate program

    std::string ipAddress;
    std::cout << "Enter the IP address: ";
    std::cin >> ipAddress;

    int tcpSocket = connectTCP(ipAddress);
    if (tcpSocket == -1) {
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "TCP Connected\n";
    }
    // get udp address and connect with udp
    struct sockaddr_in udpServerAddress;
    int udpSocket = connectUDP(&udpServerAddress, ipAddress);
    


    // If user select wrong number then it ask for user to enter again
    while (true) {
        int option;
        std::cout << "1. Split Text (Basic Encoding)\n";
        std::cout << "2. Merge Text (Basic Encoding)\n";
        std::cout << "3. Split Text (Advanced Encoding)\n";
        std::cout << "4. Merge Text (Advanced Encoding)\n";
        std::cout << "5. Quit\n";
        std::cout << "Choose option: ";
        std::cin >> option;

        if (std::cin.fail() || option < 1 || option > 5) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 5.\n";
            continue;
        }

        if (option == 5) {
            break;
        }

        // send option that I pick to server
        std::string optionStr = std::to_string(option);
        send(tcpSocket, optionStr.c_str(), optionStr.length(), 0);

        switch (option) {
            case 1:
                sendAddressUDP(udpSocket, udpServerAddress);
                sendTextSplit(tcpSocket);
                receivSplitText(tcpSocket, udpSocket, buffer, udpServerAddress);
                break;
            case 2:
                sendTextMerge(tcpSocket, udpSocket, udpServerAddress);
                receiveMergeText(tcpSocket, buffer);
                break;
            case 3:
                sendAddressUDP(udpSocket, udpServerAddress);
                sendTextSplit(tcpSocket);
                receivSplitText(tcpSocket, udpSocket, buffer, udpServerAddress);
                break;
            case 4:
                sendTextMerge(tcpSocket, udpSocket, udpServerAddress);
                receiveMergeText(tcpSocket, buffer);
                break;
        }
    }

    close(tcpSocket);
    close(udpSocket);

    return 0;
}

