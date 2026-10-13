#include "SGKDSerializer.h"

namespace gkd_vanet{

// Converts raw binary bytes to a hexadecimal string
std::string bytesToHex(const uint8_t* data, size_t len) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

// Converts a hexadecimal string back to raw binary bytes
std::vector<uint8_t> hexToBytes(const std::string& hex) {
    if (hex.length() % 2 != 0) {
        throw std::invalid_argument("Hex string must have an even length.");
    }
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.length() / 2);

    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// =========================================================================
// SERIALIZERS (RELIC Element -> Hex String)
// =========================================================================

std::string serializeToHex(const bn_t element) {
    int len = bn_size_bin(element);
    std::vector<uint8_t> buffer(len);
    bn_write_bin(buffer.data(), len, element);
    return bytesToHex(buffer.data(), buffer.size());
}

std::string serializeToHex(const ep_t element) {
    int len = ep_size_bin(element, 1); // 1 = compressed format
    std::vector<uint8_t> buffer(len);
    ep_write_bin(buffer.data(), len, element, 1);
    return bytesToHex(buffer.data(), buffer.size());
}
std::string serializeToHex(const ep2_t element) {
    int len = ep2_size_bin(element, 1); // 1 = compressed format
    std::vector<uint8_t> buffer(len);
    ep2_write_bin(buffer.data(), len, element, 1);
    return bytesToHex(buffer.data(), buffer.size());
}

std::string serializeToHex(const gt_t element) {
    int len = gt_size_bin(element, 1); // 1 = compressed format
    std::vector<uint8_t> buffer(len);
    gt_write_bin(buffer.data(), len, element, 1);
    return bytesToHex(buffer.data(), buffer.size());
}

// =========================================================================
// DESERIALIZERS (Hex String -> RELIC Element)
// =========================================================================

void deserializeFromHex(bn_t element, const std::string& hexStr) {
    std::vector<uint8_t> bytes = hexToBytes(hexStr);
    bn_read_bin(element, bytes.data(), static_cast<int>(bytes.size()));
}

void deserializeFromHex(ep_t element, const std::string& hexStr) {
    std::vector<uint8_t> bytes = hexToBytes(hexStr);
    ep_read_bin(element, bytes.data(), static_cast<int>(bytes.size()));
}

void deserializeFromHex(ep2_t element, const std::string& hexStr) {
    std::vector<uint8_t> bytes = hexToBytes(hexStr);
    ep2_read_bin(element, bytes.data(), static_cast<int>(bytes.size()));
}


void deserializeFromHex(gt_t element, const std::string& hexStr) {
    std::vector<uint8_t> bytes = hexToBytes(hexStr);
    gt_read_bin(element, bytes.data(), static_cast<int>(bytes.size()));
}

} // namespace gkd_vanet
