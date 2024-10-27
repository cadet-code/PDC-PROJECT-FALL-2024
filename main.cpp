#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <omp.h>
#include <stdexcept>
#include <chrono>  // For execution time measurement

using namespace std;
using namespace chrono;

// Struct to represent network packet information
struct Packet {
    string timestamp;
    string srcIP;
    string destIP;
    string protocol;
    int packetSize;
};

// Function to parse a network packet from a line
Packet parsePacket(const string& line) {
    Packet packet;
    stringstream ss(line);

    if (!(ss >> packet.timestamp >> packet.srcIP >> packet.destIP >> packet.protocol >> packet.packetSize)) {
        throw runtime_error("Failed to parse packet data.");
    }

    return packet;
}

// Function to load packets from a file
vector<Packet> loadPackets(const string& filename) {
    vector<Packet> packets;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        throw runtime_error("Failed to open file: " + filename);
    }

    while (getline(file, line)) {
        if (!line.empty()) {
            try {
                packets.push_back(parsePacket(line));
            } catch (const runtime_error& e) {
                cerr << "Error parsing line: " << e.what() << endl;
            }
        }
    }

    file.close();
    return packets;
}

int main() {
    string filename = "network_traffic.txt";  // File containing network traffic data

    // Load packets from the file and measure time
    auto start = high_resolution_clock::now();
    vector<Packet> packets;
    try {
        packets = loadPackets(filename);
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }
    auto end = high_resolution_clock::now();
    auto loadTime = duration_cast<milliseconds>(end - start);
    cout << "Loaded " << packets.size() << " packets in " << loadTime.count() << " ms." << endl;

    // Variables to store analysis results
    int totalPackets = 0;
    map<string, int> protocolCount;
    long long totalDataTransferred = 0;

    // Initialize protocol counts for common protocols
    protocolCount["TCP"] = 0;
    protocolCount["UDP"] = 0;
    protocolCount["ICMP"] = 0;
    protocolCount["Others"] = 0;

    // Parallel processing of packets using OpenMP
    start = high_resolution_clock::now();
    #pragma omp parallel for reduction(+:totalPackets, totalDataTransferred)
    for (int i = 0; i < packets.size(); i++) {
        // Increase total packet count
        totalPackets++;

        // Update protocol count
        string protocol = packets[i].protocol;
        if (protocol != "TCP" && protocol != "UDP" && protocol != "ICMP") {
            protocol = "Others";
        }

        // Use critical section for thread-safe updates of the map
        #pragma omp critical
        {
            protocolCount[protocol]++;
        }

        // Update total data transferred
        totalDataTransferred += packets[i].packetSize;
    }
    end = high_resolution_clock::now();
    auto analysisTime = duration_cast<milliseconds>(end - start);

    // Display results
    cout << "\nAnalysis Complete!" << endl;
    cout << "---------------------------" << endl;
    cout << "Total Packets: " << totalPackets << endl;
    cout << "Total Data Transferred: " << totalDataTransferred << " bytes" << endl;

    cout << "Packets by Protocol:" << endl;
    for (const auto& entry : protocolCount) {
        cout << entry.first << ": " << entry.second << " packets" << endl;
    }

    // Display execution time
    cout << "\nTime taken for loading: " << loadTime.count() << " ms" << endl;
    cout << "Time taken for analysis: " << analysisTime.count() << " ms" << endl;

    return 0;
}

