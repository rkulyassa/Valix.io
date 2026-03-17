#include <vector>
#include <cstdint>

class BinaryWriter {
public:
    std::vector<uint8_t> buffer;

    void writeUint8(uint8_t v) {
        buffer.push_back(v);
    }

    void writeUint16(uint16_t v) {
        buffer.push_back((v >> 8) & 0xFF);
        buffer.push_back(v & 0xFF);
    }

    void writeUint32(uint32_t v) {
        buffer.push_back((v >> 24) & 0xFF);
        buffer.push_back((v >> 16) & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
        buffer.push_back(v & 0xFF);
    }

    const uint8_t* data() const {
        return buffer.data();
    }

    size_t size() const {
        return buffer.size();
    }

    std::string_view asStringView() const {
        return std::string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    }

    void print() const {
        // Print each byte in hex for readability
        for (size_t i = 0; i < buffer.size(); ++i) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
    }
};