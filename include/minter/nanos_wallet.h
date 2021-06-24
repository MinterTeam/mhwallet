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

#include "frame_io_apdu.h"
#include "hidpp_base.h"
#include "hidpp_device.h"

#include <memory>
#include <minter/address.h>
#include <minter/tx/signature.h>
#include <mutex>

#define DEVICE_CLASS 0xe0

// Vendor/Product id
#define LEDGER_VID 0x2c97
#define NANOS_PID_ROOT 0x1011
#define NANOS_PID 0x0001
#define NANOS_PID_16_APP 0x1005
#define NANOX_PID 0x0004

// Commands
const uint8_t CMD_GET_VERSION = 0x01u;
constexpr const uint8_t CMD_GET_ADDRESS = 0x01u << 1u;
constexpr const uint8_t CMD_SIGN_TX = 0x01u << 2u;

namespace minter {

static std::string statusToString(uint16_t status) {
    switch (status) {
    case CODE_SUCCESS:
        return "success";
    case CODE_NO_STATUS_RESULT:
        return "no_status_result";
    case CODE_INVALID_PARAM:
        return "invalid_parameter";
    case CODE_USER_REJECTED:
        return "user_rejected";
    default:
        return "unknown_error";
    }
}

enum MINTER_MH_API dev_connection_type {
    DISCONNECTED,
    APP_NOT_OPENED,
    PERMISSION_ERROR,
    APP_OPENED
};

struct app_item {
    std::string name;
    tb::bytes_data hash;
    tb::bytes_data hash_code_data;
    size_t flags = 0;
};

class MINTER_MH_API nanos_wallet {
public:
    nanos_wallet();
    ~nanos_wallet();
    void close();
    minter::dev_connection_type reopen_with_app(uint16_t product_id);
    minter::dev_connection_type init_ext(uint16_t product_id = NANOS_PID_16_APP);

    /**
     * \brief opens root ledger device
     * \return Valid state for init root methods is APP_NOT_OPENED
     */
    minter::dev_connection_type init_ext_root();

    /**
     * \brief opens ledger specified product id
     * \param product_id
     * \return
     */
    bool init(uint16_t product_id = NANOS_PID_16_APP);
    bool init_root();

    tb::bytes_data exchange(const minter::APDU& apdu, uint16_t* resCode = nullptr);

    minter::address_t get_address(uint32_t deriveIndex = 0, bool silent = false);
    minter::signature sign_tx(tb::bytes_data txHash, uint32_t deriveIndex = 0);
    std::string get_app_version();

    // system
    uint16_t get_opened_app_pid();
    std::vector<app_item> get_app_list();
    uint16_t run_app(const std::string& name = "Minter");

private:
    minter::hidpp m_hid;
    minter::frame_io_apdu m_framer;
    bool m_valid;
    std::mutex m_devLock;
};

} // namespace minter

#endif // MHWALLET_NANOS_WALLET_H
