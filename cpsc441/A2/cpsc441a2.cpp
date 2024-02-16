#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <climits>
#include <algorithm>
#include <iostream>
#include <regex>

#define PORT 1111
#define BUFFER_SIZE 10000

using namespace std;

int connect_to_host(const char* host, const char* port) {
    int sockfd;
    struct addrinfo hints, *result, *rp;
    int s;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((s = getaddrinfo(host, port, &hints, &result)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(s) << endl;
        return -1;
    }

    for(rp = result; rp != NULL; rp = rp->ai_next) {
        if ((sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            perror("Socket Fail");
            continue;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == -1) {
            close(sockfd);
            perror("Connection Fail");
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (rp == NULL) {
        cerr << "Failed to connect" << endl;
        return -1;
    }
    return sockfd;
}

int enterPortNumber() {
    int port;
    
    while (true) {
        cout << "Enter port you wish the proxy to run on: ";
        cin >> port;
        if(cin.fail() || port < 1025 || port > 65535) {
            cin.clear();
            cout << "You can only enter numbers between 1025 and 65535.\n";
            cin.ignore(INT_MAX, '\n');
        } 
        else {
            break;
        }
    }
    return port;
}

void replaceFrogWithFred(char* buffer, ssize_t length) {
    const std::string from = "frog";
    const std::string to = "fred";

    bool insideTag = false;
    
    for (ssize_t i = 0; i < length - from.size() + 1; ++i) {
        if (buffer[i] == '<') {
            insideTag = true;
        } else if (buffer[i] == '>') {
            insideTag = false;
        }

        if (insideTag) continue;
        bool match = true;
        for (size_t j = 0; j < from.size(); ++j) {
            if (std::tolower(buffer[i + j]) != from[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            for (size_t j = 0; j < to.size(); ++j) {
                buffer[i + j] =
                    std::isupper(buffer[i + j]) ? std::toupper(to[j]) : std::tolower(to[j]);
            }
            i += from.size() - 1;
        }
    }
}

void replaceSrcImage(std::string& html) {
    std::string new_src = "https://thumbs.dreamstime.com/z/american-green-tree-frog-isolated-american-green-tree-frog-isolated-white-101756875.jpg?w=992";
    std::size_t startp = 0;
    while((startp = html.find("<img src=\"", startp)) != std::string::npos) {
        std::size_t endp = html.find("\"", startp + 10);
        if(endp != std::string::npos) {
            html.replace(startp + 10, endp - (startp + 10), new_src);
            startp = endp + new_src.length();
        } else {
            break;
        }
    }
}

int main() {
    int proxy_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    
    int port = enterPortNumber();

    char hostname[1024];
    gethostname(hostname, 1024);
    hostent* IPaddress = gethostbyname(hostname);

    if (hostname == NULL) {
        perror("gethostbyname");
        return 1;
    }

    cout << "You chose to run the proxy on PORT: " << port << "\n";
    cout << "Proxy is hosted on server : " << hostname << "\n";
    cout << "The IP of the server that the Proxy is hosted on is: " << inet_ntoa(**(in_addr**)IPaddress->h_addr_list) << "\n";

    if ((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(proxy_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(proxy_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Proxy server started at port " << port << ". Waiting for connections..." << endl;

    while (true) {
        if ((client_fd = accept(proxy_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        ssize_t bytesRead = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytesRead < 0) {
            perror("read");
            close(client_fd);
            continue;
        }


        int website_fd = connect_to_host("pages.cpsc.ucalgary.ca", "80");
        if (website_fd < 0) {
            close(client_fd);
            continue;
        }

        send(website_fd, buffer, bytesRead, 0);

        while (true) {
            bytesRead = recv(website_fd, buffer, BUFFER_SIZE, 0);
            if (bytesRead <= 0) {
                break;
            }
            
            //replace Frog to Fred
            replaceFrogWithFred(buffer, bytesRead);


            // picture change
            std::string htmlStr(buffer);
            replaceSrcImage(htmlStr);
            memset(buffer, 0, sizeof(buffer)); 
            strncpy(buffer, htmlStr.c_str(), sizeof(buffer) - 1); 

            ssize_t bytesSent = 0;
            while (bytesSent < bytesRead) {
                ssize_t result = send(client_fd, buffer + bytesSent, htmlStr.size() - bytesSent, 0); 
                if(result <= 0){
                    break;
                }
                bytesSent += result; 
            }
        }

        close(website_fd);
        close(client_fd);
    }

    return 0;
}