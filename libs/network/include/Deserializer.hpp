/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Deserializer
*/

#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include <stdexcept>

namespace network {

    class Deserializer {
    public:
        Deserializer(const std::vector<uint8_t>& data)
            : _data(data)
            , _offset(0) {}

        uint8_t readU8() {
            check(1);
            return _data[_offset++];
        }

        uint16_t readU16() {
            check(2);
            uint16_t v = _data[_offset] | (_data[_offset + 1] << 8);
            _offset += 2;
            return v;
        }

        uint32_t readU32() {
            check(4);
            uint32_t v = 0;
            for (int i = 0; i < 4; ++i)
                v |= static_cast<uint32_t>(_data[_offset++]) << (i * 8);
            return v;
        }

        uint64_t readU64() {
            check(8);
            uint64_t v = 0;
            for (int i = 0; i < 8; ++i)
                v |= static_cast<uint64_t>(_data[_offset++]) << (i * 8);
            return v;
        }

        float readFloat() {
            uint32_t bits = readU32();
            float v;
            std::memcpy(&v, &bits, sizeof(float));
            return v;
        }

        std::string readString(size_t fixedSize) {
            check(fixedSize);
            std::string s;
            for (size_t i = 0; i < fixedSize && _data[_offset + i] != 0; ++i)
                s += static_cast<char>(_data[_offset + i]);
            _offset += fixedSize;
            return s;
        }

        bool isFinished() const { return _offset >= _data.size(); }
        size_t remaining() const { return _data.size() - _offset; }
    private:
        void check(size_t n) {
            if (_offset + n > _data.size())
                throw std::runtime_error("Deserializer: buffer overflow");
        }

        const std::vector<uint8_t>& _data;
        size_t _offset;
    };

}
