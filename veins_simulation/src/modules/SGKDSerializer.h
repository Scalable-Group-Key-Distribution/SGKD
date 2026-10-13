#ifndef SGKDSERIALIZER_H
#define SGKDSERIALIZER_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
extern "C" {
#include <relic/relic.h>
}

namespace gkd_vanet {

// Utility functions to convert byte arrays to/from Hex strings
std::string bytesToHex(const uint8_t* data, size_t len);
std::vector<uint8_t> hexToBytes(const std::string& hex);

// -------------------------------------------------------------------------
// Serialization: RELIC Element -> Hex String
// -------------------------------------------------------------------------
std::string serializeToHex(const bn_t element);
std::string serializeToHex(const ep_t element);
std::string serializeToHex(const ep2_t element);
std::string serializeToHex(const gt_t element);

// -------------------------------------------------------------------------
// Deserialization: Hex String -> RELIC Element
// -------------------------------------------------------------------------
void deserializeFromHex(bn_t element, const std::string& hexStr);
void deserializeFromHex(ep_t element, const std::string& hexStr);
void deserializeFromHex(ep2_t element, const std::string& hexStr);
void deserializeFromHex(gt_t element, const std::string& hexStr);

} // namespace gkd_vanet

#endif // SGKDSERIALIZER_H
