/*!
 * mhwallet.
 * nanos_wallet.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_NANOS_WALLET_H
#define MHWALLET_NANOS_WALLET_H

#include <memory>
#include <mutex>
#include <minter/address.h>
#include <minter/tx/signature.h>
#include <toolboxpp.hpp>

#include "hidpp_base.h"
#include "hidpp_device.h"
#include "frame_io_apdu.h"

#define DEVICE_CLASS 0xe0

// Vendor/Product id
#define LEDGER_VID 0x2c97
#define NANOS_PID 0x0001
#define NANOX_PID 0x0004

// Commands
const uint8_t CMD_GET_VERSION = 0x01u;
constexpr const uint8_t CMD_GET_ADDRESS = 0x01u << 1u;
constexpr const uint8_t CMD_SIGN_TX = 0x01u << 2u;

namespace minter {

static std::string statusToString(uint16_t status) {
    switch (status) {
        case CODE_SUCCESS:return "success";
        case CODE_NO_STATUS_RESULT: return "no_status_result";
        case CODE_INVALID_PARAM: return "invalid_parameter";
        case CODE_USER_REJECTED: return "user_rejected";
        default: return "unknown_error";
    }
}

class nanos_wallet {
 public:
    nanos_wallet();

    bool init();
    bytes_data exchange(const minter::APDU &apdu, uint16_t *resCode = nullptr);

    minter::address_t get_address(uint32_t deriveIndex = 0);
    minter::signature sign_tx(bytes_data txHash, uint32_t deriveIndex = 0);
    std::string get_app_version();

 private:
    minter::hidpp m_hid;
    minter::frame_io_apdu m_framer;
    bool m_valid;
    std::mutex m_devLock;
};

}

#endif //MHWALLET_NANOS_WALLET_H
