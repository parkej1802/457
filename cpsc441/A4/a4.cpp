#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <queue>


struct Packet {
    double timestamp;
    int size;
    double departureTime;
    double arrivalTime;
};

enum EventType {
    PACKET_ARRIVAL,
    PACKET_DROP,
    PACKET_DEPARTURE
};

struct Event {
    EventType type;
    double time;
    Packet packet;
    bool operator<(const Event& other) const {
        return time > other.time;
        
    }

};
// retrieve timestamp and size from the file 
std::vector<Packet> readTraceFile(const std::string& filename) {
    std::vector<Packet> packets;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return packets;
    }

    while (getline(file, line)) {
        std::istringstream iss(line);
        double timestamp;
        int size;

        if (!(iss >> timestamp >> size)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;
        }

        packets.push_back({timestamp, size});
    }

    file.close();
    return packets;
}
// getting input of buffer size
int getBufferSize() {
    int size;
    while (true) {
        std::cout << "Enter the AP buffer size: ";
        std::cin >> size;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter an integer value." << std::endl;
        } else {
            break;
        }
    }
    return size;
}
//getting input of speed 
int getWLAN() {
    int wlan;
    while (true) {
        std::cout << "Enter the WLAN: ";
        std::cin >> wlan;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter an integer value." << std::endl;
        } else {
            break;
        }
    }
    return wlan;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong input! Type : './[program name] [txt file]'\n", argv[0]);
        return 1;
    }

    std::priority_queue<Event> events;
    std::queue<Packet> buffer;
    int totalPacketsIn = 0;
    int totalPacketsInByte = 0;
    int totalPacketsOut = 0;
    int totalPacketsOutByte = 0;
    int totalPacketsLost = 0;
    int totalPacketsLostByte = 0;
    double currentTime = 0.0;
    double totalQueuingDelay = 0.0;

    std::string filename = argv[1];
    std::vector<Packet> packets = readTraceFile(filename);
    int bufferSize = getBufferSize();
    int wlan = getWLAN();

    for (const Packet& packet : packets) {
        events.push({ PACKET_ARRIVAL, packet.timestamp, packet });
    }

    double at = 0.0;
    double departureTime = 0.0;

    // while event is empty
    while (!events.empty()) {
        Event event = events.top();
        events.pop();

        double event_time = event.time;
        Packet& packet = event.packet;

        switch (event.type) {
            case PACKET_ARRIVAL:
                // all the packets are added to the packet arrival
                totalPacketsIn++;
                totalPacketsInByte += packet.size;

                // if buffer can take packets
                if (buffer.size() < bufferSize) {
                    buffer.push(packet);

                    // if buffer has size of 1, it goes to the packet departure
                    if (buffer.size() == 1) {
                        // calculate exact time of packet that goes to the packet_departure 
                        packet.departureTime = event_time + (packet.size * 8.0 / (wlan * 1000000));
                        events.push({PACKET_DEPARTURE, packet.departureTime, packet});
                    }
                    
                }

                //if buffer is full then we drop packets 
                else {
                    events.push({PACKET_DROP, event_time, packet});
                }
                break;


            case PACKET_DEPARTURE:
            { 
                // get current time
                packet.arrivalTime = event.time;
                // receive departure time from arrival so current time - departureTime will give queuing delay
                totalQueuingDelay += packet.arrivalTime - packet.departureTime;
                // calculating number of packets that come to packet departure 
                totalPacketsOut++;
                totalPacketsOutByte += packet.size;
                // remove front packet
                buffer.pop();
                // if buffer is not empty
                if (!buffer.empty()) {
                    // next packets to go to calculate departuretime and push that to departure
                    Packet& next_packet = buffer.front();
                    // updating the departure time for the next packet so we know the next packet time now for each packets
                    next_packet.departureTime = event_time + (next_packet.size * 8.0 / (wlan * 1000000));
                    events.push({PACKET_DEPARTURE, next_packet.departureTime, next_packet});
                    totalQueuingDelay += packet.arrivalTime - buffer.front().timestamp;
                }
            }
            break;
            // when buffer is full, it drops packets that calculating number of packets lost
            case PACKET_DROP:
                totalPacketsLost++;
                totalPacketsLostByte += packet.size;
                break;
        }
    }
    std::cout << "Total packets in: " << totalPacketsIn << std::endl;
    std::cout << "Total packets in Byte: " << totalPacketsInByte << std::endl;
    std::cout << "Total packets out: " << totalPacketsOut << std::endl;
    std::cout << "Total packets out Byte: " << totalPacketsOutByte << std::endl;
    std::cout << "Total packets lost: " << totalPacketsLost << std::endl;
    std::cout << "Total packets lost Byte: " << totalPacketsLostByte << std::endl;

    double averageQueuingDelay = totalPacketsOut > 0 ? totalQueuingDelay / totalPacketsOut : 0;
    double lostpacket = (double)totalPacketsLost / totalPacketsIn * 100.0;
    std::cout << "Lost packets in %: " << lostpacket << " % " << std::endl;
    std::cout << "Average queuing delay: " << averageQueuingDelay << " seconds" << std::endl;

    return 0;
}
