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

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <iostream>
#include <fmt/format.h>
#include <toolboxpp.hpp>
#include "mhwallet_core.h"
#include "hid/hidapi.h"

namespace tb = toolboxpp::data;
using tb::operator ""_dbyte;
using tb::operator ""_dbytes;
using tb::operator ""_byte;
using tb::operator ""_bytes;

// Status codes
const uint16_t CODE_SUCCESS = 0x9000;
const uint16_t CODE_USER_REJECTED = 0x6985;
const uint16_t CODE_INVALID_PARAM = 0x6b01;
constexpr const uint16_t CODE_NO_STATUS_RESULT = CODE_SUCCESS + 1;

volatile static bool HIDPP_VERBOSE = true;

static void ml_log_s(const std::string &msg, std::ostream *os = &std::cout) {
    if(HIDPP_VERBOSE)
        (*(os)) << msg << std::endl;
}

static void ML_LOG(const std::string &msg) {
    ml_log_s(msg);
}

template <typename... Args>
static void ML_LOG(const std::string &msg, const Args &... args) {
    ML_LOG(fmt::format(msg, args...));
}

static void ML_ERR(const std::string &msg) {
    std::cerr << msg << std::endl;
}

template <typename... Args>
static void ML_ERR(const std::string &msg, const Args &... args) {
    ML_ERR(fmt::format(msg, args...));
}

namespace minter {

struct hid_device_info_deleter {
  void operator()(hid_device_info *d) {
      free(d->path);
      free(d->serial_number);
      free(d->manufacturer_string);
      free(d->product_string);
      free(d);
  }
};

struct hid_device_deleter {
  void operator()(hid_device *d) {
      hid_close(d);
  }
};

using dev_info_t = std::unique_ptr<hid_device_info, hid_device_info_deleter>;
using dev_handle_t = std::unique_ptr<hid_device, hid_device_deleter>;

struct APDU {
  uint8_t cla;
  uint8_t ins;
  uint8_t p1;
  uint8_t p2;
  uint8_t payload_size;
  uint8_t payload[0xFF]; //max size - 255 bytes

  [[nodiscard]] tb::bytes_data to_bytes() const {
      tb::bytes_data out(5 + payload_size);
      out.write(0, cla);
      out.write(1, ins);
      out.write(2, p1);
      out.write(3, p2);
      out.write(4, payload_size);

      if (payload_size > 0) {
          out.write(5, payload, payload_size);
      }

      return out;
  }
};



static std::string dumpHex(const uint8_t *data, size_t size) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < size; i++) {
        const uint8_t tmp[1] = {data[i]};
        ss << tb::bytesToHex(tmp, 1);

        if (i != size - 1) {
            ss << " ";
        }
    }
    ss << "]";
    return ss.str();
}

static std::string dumpHexRet(const tb::bytes_data &data) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < data.size(); i++) {
        const uint8_t tmp[1] = {data.at(i)};
        ss << tb::bytesToHex(tmp, 1);

        if (i != data.size() - 1) {
            ss << " ";
        }
    }
    ss << "]";
    return ss.str();
}

static std::string dumpHexRet(const std::vector<uint8_t> &data) {
    return dumpHexRet(tb::bytes_data(data));
}

static void dumpHex(const tb::bytes_data &data, std::ostream *outstream = &std::cout) {
    (*(outstream)) << dumpHexRet(data) << std::endl;
}

} // minter

#endif //MHWALLET_HIDPP_BASE_H
