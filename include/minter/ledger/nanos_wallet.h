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
#include "mhwallet_core.h"

#include <memory>
#include <minter/address.h>
#include <minter/tx/signature.h>
#include <mutex>

// Commands
const uint8_t CMD_GET_VERSION = 0x01u;
constexpr const uint8_t CMD_GET_ADDRESS = 0x01u << 1u;
constexpr const uint8_t CMD_SIGN_TX = 0x01u << 2u;

namespace minter {

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

    tb::bytes_data exchange(const minter::APDU& apdu, uint16_t* res_code = nullptr);

    minter::address_t get_address(uint32_t derive_index = 0, bool silent = false);
    minter::signature sign_tx(const tb::bytes_data& raw_tx, uint32_t derive_index = 0);
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
