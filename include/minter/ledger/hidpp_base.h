/*!
 * mhwallet.
 * hidpp_base.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_HIDPP_BASE_H
#define MHWALLET_HIDPP_BASE_H

#include "hid/hidapi.h"
#include "mhwallet_core.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <toolbox/data/bytes_buffer.h>
#include <toolbox/data/bytes_data.h>
#include <toolbox/data/literals.h>

namespace tb = toolbox::data;
using namespace toolbox::data::literals;

volatile static bool HIDPP_VERBOSE = true;

static void ml_log_s(const std::string& msg, std::ostream* os = &std::cout) {
    if (HIDPP_VERBOSE)
        (*(os)) << msg << std::endl;
}

static void ML_LOG(const std::string& msg) {
    ml_log_s(msg);
}

template<typename... Args>
static void ML_LOG(const std::string& msg, const Args&... args) {
    ML_LOG(fmt::format(msg, args...));
}

static void ML_ERR(const std::string& msg) {
    std::cerr << msg << std::endl;
}

template<typename... Args>
static void ML_ERR(const std::string& msg, const Args&... args) {
    ML_ERR(fmt::format(msg, args...));
}

namespace minter {

struct MINTER_MH_API hid_device_info_deleter {
    void operator()(hid_device_info* d) {
        free(d->path);
        free(d->serial_number);
        free(d->manufacturer_string);
        free(d->product_string);
        free(d);
    }
};

using dev_info_t = std::unique_ptr<hid_device_info, hid_device_info_deleter>;

struct MINTER_MH_API APDU {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    size_t payload_size;
    uint8_t payload[0xFF]; // max size - 255 bytes

    [[nodiscard]] tb::bytes_data to_bytes() const {
        tb::bytes_data out(5 + payload_size);
        out.write(0, cla);
        out.write(1, ins);
        out.write(2, p1);
        out.write(3, p2);
        out.write(4, (uint8_t) payload_size);

        if (payload_size > 0) {
            out.write(5, payload, (uint8_t) payload_size);
        }

        return out;
    }
};

static std::string dumpHex(const uint8_t* data, size_t size) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < size; i++) {
        const uint8_t tmp[1] = {data[i]};
        ss << tb::bytes_to_hex(tmp, 1);

        if (i != size - 1) {
            ss << " ";
        }
    }
    ss << "]";
    return ss.str();
}

static std::string dumpHexRet(const tb::bytes_data& data) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < data.size(); i++) {
        const uint8_t tmp[1] = {data.at(i)};
        ss << tb::bytes_to_hex(tmp, 1);

        if (i != data.size() - 1) {
            ss << " ";
        }
    }
    ss << "]";
    return ss.str();
}

static std::string dumpHexRet(const std::vector<uint8_t>& data) {
    return dumpHexRet(tb::bytes_data(data));
}

static void dumpHex(const tb::bytes_data& data, std::ostream* outstream = &std::cout) {
    (*(outstream)) << dumpHexRet(data) << std::endl;
}

} // namespace minter

#endif // MHWALLET_HIDPP_BASE_H
