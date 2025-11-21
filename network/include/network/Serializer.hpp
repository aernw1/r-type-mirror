#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <cstring>

namespace network {

    class Serializer {
    public:
        void writeU8(uint8_t v) { _buf.push_back(v); }

        void writeU16(uint16_t v) {
            _buf.push_back(v & 0xFF);
            _buf.push_back((v >> 8) & 0xFF);
        }

        void writeU32(uint32_t v) {
            for (int i = 0; i < 4; ++i)
                _buf.push_back((v >> (i * 8)) & 0xFF);
        }

        void writeU64(uint64_t v) {
            for (int i = 0; i < 8; ++i)
                _buf.push_back((v >> (i * 8)) & 0xFF);
        }

        void writeFloat(float v) {
            uint32_t bits;
            std::memcpy(&bits, &v, sizeof(float));
            writeU32(bits);
        }

        void writeString(const std::string& s, size_t fixedSize) {
            size_t len = std::min(s.size(), fixedSize - 1);
            for (size_t i = 0; i < len; ++i)
                _buf.push_back(static_cast<uint8_t>(s[i]));
            for (size_t i = len; i < fixedSize; ++i)
                _buf.push_back(0);
        }

        std::vector<uint8_t> finalize() const { return _buf; }
        size_t size() const { return _buf.size(); }
        void clear() { _buf.clear(); }
    private:
        std::vector<uint8_t> _buf;
    };

}
