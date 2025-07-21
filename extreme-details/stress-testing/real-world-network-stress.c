/*
 * Real-World Application Stress Test - Network Protocol Parser
 * 
 * This test implements a realistic network protocol parser that demonstrates
 * where CHERI overhead may be prohibitive in performance-critical applications.
 */

#ifdef __CHERI__
#include <cheriintrin.h>
typedef void* __capability cap_ptr_t;
#else
typedef void* cap_ptr_t;
#define cheri_bounds_set(ptr, size) (ptr)
#define cheri_tag_get(cap) 1
#endif

// Network packet simulation
#define MAX_PACKET_SIZE 1500  // Ethernet MTU
#define PACKETS_TO_PROCESS 100000  // High-volume processing

// Protocol header structures
struct ethernet_header {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short ethertype;
};

struct ip_header {
    unsigned char version_ihl;
    unsigned char tos;
    unsigned short total_length;
    unsigned short identification;
    unsigned short flags_fragment;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short checksum;
    unsigned int src_ip;
    unsigned int dest_ip;
};

struct tcp_header {
    unsigned short src_port;
    unsigned short dest_port;
    unsigned int seq_num;
    unsigned int ack_num;
    unsigned char data_offset_flags;
    unsigned char flags;
    unsigned short window;
    unsigned short checksum;
    unsigned short urgent_pointer;
};

// Global packet buffer
static char packet_buffer[MAX_PACKET_SIZE * 100];  // Buffer for multiple packets
static int buffer_offset = 0;

// Packet allocator
cap_ptr_t allocate_packet(int size) {
    if (buffer_offset + size > sizeof(packet_buffer)) {
        buffer_offset = 0;  // Wrap around (circular buffer)
    }
    
    void* ptr = &packet_buffer[buffer_offset];
    buffer_offset += size;
    
    return cheri_bounds_set(ptr, size);
}

// Network Protocol Parsing Functions

// Parse Ethernet header
int parse_ethernet(cap_ptr_t packet_data, int packet_len, cap_ptr_t* next_header) {
    if (packet_len < sizeof(struct ethernet_header)) {
        return -1;  // Packet too short
    }
    
    char* data = (char*)packet_data;
    struct ethernet_header* eth = (struct ethernet_header*)data;
    
    // Extract ethertype (bounds checking occurs here in CHERI)
    unsigned short ethertype = (eth->ethertype << 8) | (eth->ethertype >> 8);  // Network byte order
    
    // Calculate next header position
    *next_header = cheri_bounds_set(data + sizeof(struct ethernet_header), 
                                    packet_len - sizeof(struct ethernet_header));
    
    return (ethertype == 0x0800) ? 1 : 0;  // Return 1 if IPv4
}

// Parse IP header
int parse_ip(cap_ptr_t ip_data, int remaining_len, cap_ptr_t* next_header) {
    if (remaining_len < sizeof(struct ip_header)) {
        return -1;
    }
    
    char* data = (char*)ip_data;
    struct ip_header* ip = (struct ip_header*)data;
    
    // Extract header length (bounds checking in CHERI)
    int header_len = (ip->version_ihl & 0x0F) * 4;
    
    if (header_len < sizeof(struct ip_header) || header_len > remaining_len) {
        return -1;
    }
    
    // Extract total length
    unsigned short total_length = (ip->total_length << 8) | (ip->total_length >> 8);
    
    // Calculate payload position
    *next_header = cheri_bounds_set(data + header_len, 
                                    remaining_len - header_len);
    
    return ip->protocol;  // Return protocol number
}

// Parse TCP header
int parse_tcp(cap_ptr_t tcp_data, int remaining_len, cap_ptr_t* payload) {
    if (remaining_len < sizeof(struct tcp_header)) {
        return -1;
    }
    
    char* data = (char*)tcp_data;
    struct tcp_header* tcp = (struct tcp_header*)data;
    
    // Extract data offset (bounds checking in CHERI)
    int header_len = ((tcp->data_offset_flags >> 4) & 0x0F) * 4;
    
    if (header_len < sizeof(struct tcp_header) || header_len > remaining_len) {
        return -1;
    }
    
    // Calculate payload position
    *payload = cheri_bounds_set(data + header_len, 
                                remaining_len - header_len);
    
    return remaining_len - header_len;  // Return payload length
}

// High-Performance Pattern Matching
int pattern_match(cap_ptr_t data, int len, const char* pattern, int pattern_len) {
    if (len < pattern_len) return 0;
    
    char* ptr = (char*)data;
    
    // Fast string search (Boyer-Moore-like)
    for (int i = 0; i <= len - pattern_len; i++) {
        int match = 1;
        
        // Compare pattern (each access triggers bounds checking in CHERI)
        for (int j = 0; j < pattern_len; j++) {
            if (ptr[i + j] != pattern[j]) {
                match = 0;
                break;
            }
        }
        
        if (match) return 1;  // Pattern found
    }
    
    return 0;  // Pattern not found
}

// Simulate realistic packet processing
void process_packet(cap_ptr_t packet, int packet_len) {
    cap_ptr_t current_header = packet;
    int remaining_len = packet_len;
    
    // Parse Ethernet header (bounds checking in CHERI)
    cap_ptr_t ip_header;
    if (parse_ethernet(current_header, remaining_len, &ip_header) != 1) {
        return;  // Not IPv4
    }
    
    remaining_len -= sizeof(struct ethernet_header);
    
    // Parse IP header (more bounds checking)
    cap_ptr_t transport_header;
    int protocol = parse_ip(ip_header, remaining_len, &transport_header);
    
    if (protocol != 6) {  // Not TCP
        return;
    }
    
    remaining_len -= sizeof(struct ip_header);  // Simplified
    
    // Parse TCP header (even more bounds checking)
    cap_ptr_t payload;
    int payload_len = parse_tcp(transport_header, remaining_len, &payload);
    
    if (payload_len <= 0) {
        return;  // No payload
    }
    
    // Pattern matching on payload (intensive bounds checking)
    const char* patterns[] = {
        "GET ",
        "POST ",
        "HTTP/1.1",
        "Content-Length:",
        "User-Agent:",
        "Accept:",
        "Authorization:"
    };
    
    int pattern_count = sizeof(patterns) / sizeof(patterns[0]);
    
    for (int i = 0; i < pattern_count; i++) {
        if (pattern_match(payload, payload_len, patterns[i], 
                         (patterns[i][0] == 'G') ? 4 : 
                         (patterns[i][0] == 'P') ? 5 : 
                         (patterns[i][0] == 'H') ? 8 : 
                         (patterns[i][0] == 'C') ? 15 : 
                         (patterns[i][0] == 'U') ? 11 : 
                         (patterns[i][0] == 'A' && patterns[i][1] == 'c') ? 7 : 14)) {
            // Pattern found - do something
            volatile int pattern_found = i;
            (void)pattern_found;
        }
    }
}

// Create realistic packet data
void create_test_packet(cap_ptr_t packet, int packet_len) {
    char* data = (char*)packet;
    
    // Ethernet header
    struct ethernet_header* eth = (struct ethernet_header*)data;
    eth->ethertype = 0x0008;  // IPv4 (network byte order)
    
    // IP header
    struct ip_header* ip = (struct ip_header*)(data + sizeof(struct ethernet_header));
    ip->version_ihl = 0x45;  // IPv4, 20-byte header
    ip->protocol = 6;  // TCP
    ip->total_length = packet_len - sizeof(struct ethernet_header);
    
    // TCP header
    struct tcp_header* tcp = (struct tcp_header*)((char*)ip + sizeof(struct ip_header));
    tcp->data_offset_flags = 0x50;  // 20-byte header
    tcp->src_port = 0x5000;  // Port 80 (network byte order)
    tcp->dest_port = 0x5000;
    
    // HTTP payload
    char* payload = (char*)tcp + sizeof(struct tcp_header);
    int payload_size = packet_len - sizeof(struct ethernet_header) - 
                      sizeof(struct ip_header) - sizeof(struct tcp_header);
    
    if (payload_size > 0) {
        const char* http_request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n"
                                  "User-Agent: TestClient/1.0\r\nAccept: text/html\r\n\r\n";
        
        int http_len = 0;
        while (http_request[http_len] != '\0') http_len++;  // strlen
        
        for (int i = 0; i < payload_size && i < http_len; i++) {
            payload[i] = http_request[i];
        }
    }
}

// Main network processing stress test
void test_network_processing_stress() {
    volatile int packets_processed = 0;
    volatile int total_bytes = 0;
    
    // Process large number of packets
    for (int i = 0; i < PACKETS_TO_PROCESS; i++) {
        // Variable packet sizes (realistic)
        int packet_size = 64 + (i % (MAX_PACKET_SIZE - 64));
        
        cap_ptr_t packet = allocate_packet(packet_size);
        if (packet == (cap_ptr_t)0) continue;
        
        // Create realistic packet content
        create_test_packet(packet, packet_size);
        
        // Process packet (intensive bounds checking in CHERI)
        process_packet(packet, packet_size);
        
        packets_processed++;
        total_bytes += packet_size;
    }
    
    // Network processing markers
    volatile int net_packets = packets_processed;
    volatile int net_bytes = total_bytes;
    (void)net_packets; (void)net_bytes;
}

// Test deep packet inspection workload
void test_deep_packet_inspection() {
    const char* suspicious_patterns[] = {
        "eval(",
        "script>",
        "../../../",
        "DROP TABLE",
        "UNION SELECT",
        "javascript:",
        "<iframe",
        "onload="
    };
    
    int pattern_count = sizeof(suspicious_patterns) / sizeof(suspicious_patterns[0]);
    volatile int detections = 0;
    
    for (int packet_num = 0; packet_num < 10000; packet_num++) {
        int packet_size = 200 + (packet_num % 1000);
        cap_ptr_t packet = allocate_packet(packet_size);
        if (packet == (cap_ptr_t)0) continue;
        
        // Create packet with potential malicious content
        char* data = (char*)packet;
        for (int i = 0; i < packet_size; i++) {
            data[i] = 'A' + (i % 26);  // Fill with test data
        }
        
        // Insert suspicious pattern occasionally
        if (packet_num % 100 == 0 && packet_size > 100) {
            const char* pattern = suspicious_patterns[packet_num % pattern_count];
            int pattern_len = 0;
            while (pattern[pattern_len] != '\0') pattern_len++;
            
            for (int i = 0; i < pattern_len && i < packet_size - 50; i++) {
                data[50 + i] = pattern[i];
            }
        }
        
        // Deep packet inspection (intensive string matching)
        for (int i = 0; i < pattern_count; i++) {
            const char* pattern = suspicious_patterns[i];
            int pattern_len = 0;
            while (pattern[pattern_len] != '\0') pattern_len++;
            
            if (pattern_match(packet, packet_size, pattern, pattern_len)) {
                detections++;
            }
        }
    }
    
    // DPI results marker
    volatile int dpi_detections = detections;
    (void)dpi_detections;
}

// Main real-world application test
int main() {
    // Test 1: High-volume network packet processing
    test_network_processing_stress();
    
    // Test 2: Deep packet inspection workload
    test_deep_packet_inspection();
    
    // Real-world test completion marker
    volatile int realworld_complete = 0x8EA1F081;  // REAL WORLD
    (void)realworld_complete;
    
    return 0;
}
