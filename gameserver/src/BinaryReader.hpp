#include <cstddef>
#include <cstdint>
#include <stdexcept>

class BinaryReader {
public:
    const uint8_t *data;
    size_t size;
    size_t offset;

    BinaryReader(const uint8_t *data, size_t size) : data(data), size(size), offset(0) {}

    uint8_t readUint8() {
        ensureAvailable(1);
        return data[offset++];
    }

    uint16_t readUint16() {
        ensureAvailable(2);
        uint16_t v = (data[offset] << 8) | (data[offset+1]);
        offset += 2;
        return v;
    }

private:
    void ensureAvailable(size_t n) {
        if (offset + n > size) {
            throw std::runtime_error("Buffer underflow");
        }
    }
};