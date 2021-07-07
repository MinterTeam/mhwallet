/*!
 * mhwallet.
 * nanos_wallet.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#include "minter/ledger/nanos_wallet.h"

#include "minter/ledger/errors.h"
#include "minter/ledger/mhwallet_core.h"

#include <chrono>
#include <stdexcept>
#include <thread>

minter::nanos_wallet::nanos_wallet()
    : m_hid()
    , m_framer(hidpp_device(LEDGER_VID, NANOS_PID))
    , m_valid(false) {
}

minter::nanos_wallet::~nanos_wallet() {
    close();
}

void minter::nanos_wallet::close() {
    if (m_framer.io().valid()) {
        m_framer.io().close();
    }
    m_valid = false;
}

minter::dev_connection_type minter::nanos_wallet::reopen_with_app(uint16_t product_id) {
    close();
    return init_ext(product_id);
}

minter::dev_connection_type minter::nanos_wallet::init_ext(uint16_t product_id) {
    std::lock_guard<std::mutex> lock(m_devLock);

    auto app_devs = m_hid.enumerate_devices(LEDGER_VID, product_id);

    if (m_valid && !app_devs.empty()) {
        return minter::dev_connection_type::APP_OPENED;
    }

    if (m_valid) {
        if (m_framer.io().valid()) {
            m_framer.io().close();
        }
        m_valid = false;
    }

    auto system_devs = m_hid.enumerate_devices(LEDGER_VID, NANOS_PID_ROOT);

    if (system_devs.empty() && app_devs.empty()) {
        return minter::dev_connection_type::DISCONNECTED;
    }

    uint16_t vendor_id;
    char* path;

    if (!system_devs.empty()) {
        return minter::dev_connection_type::APP_NOT_OPENED;
    } else if (!app_devs.empty()) {
        vendor_id = app_devs.begin()->get()->vendor_id;
        product_id = app_devs.begin()->get()->product_id;
        path = app_devs.begin()->get()->path;
    } else {
        return minter::dev_connection_type::DISCONNECTED;
    }

    m_framer = hidpp_device(vendor_id, product_id);

    if (!m_framer.io().open()) {
        ML_ERR("Unable to open device {0}: {1}", path, m_framer.io().getError());
        if (errno == EACCES) {
            ML_ERR("Try to run this executable with sudo or grant to {0} read-write permissions to your user", path);
        }
        return minter::dev_connection_type::PERMISSION_ERROR;
    }
    m_valid = m_framer.io().valid();

    return minter::dev_connection_type::APP_OPENED;
}

minter::dev_connection_type minter::nanos_wallet::init_ext_root() {
    std::lock_guard<std::mutex> lock(m_devLock);

    auto system_devs = m_hid.enumerate_devices(LEDGER_VID, NANOS_PID_ROOT);

    if (m_valid && !system_devs.empty()) {
        return minter::dev_connection_type::APP_NOT_OPENED;
    }

    if (m_valid) {
        close();
    }

    if (system_devs.empty()) {
        return minter::dev_connection_type::DISCONNECTED;
    }

    if (system_devs.empty()) {
        return minter::dev_connection_type::DISCONNECTED;
    }
    auto dev = system_devs.begin();
    const char* path = dev->get()->path;

    m_framer = hidpp_device(LEDGER_VID, NANOS_PID_ROOT);

    if (!m_framer.io().open()) {
        ML_ERR("Unable to open device {0}: {1}", path, m_framer.io().getError());
        if (errno == EACCES) {
            ML_ERR("Try to run this executable with sudo or grant to {0} read-write permissions to your user", path);
        }
        return minter::dev_connection_type::PERMISSION_ERROR;
    }
    m_valid = m_framer.io().valid();

    return minter::dev_connection_type::APP_NOT_OPENED;
}

bool minter::nanos_wallet::init(uint16_t product_id) {
    std::lock_guard<std::mutex> lock(m_devLock);

    auto system_devs = m_hid.enumerate_devices(LEDGER_VID, NANOS_PID_ROOT);
    auto app_devs = m_hid.enumerate_devices(LEDGER_VID, product_id);
    if (system_devs.empty() && app_devs.empty()) {
        ML_ERR("Please, connect your Nano S to computer and open the Minter app");
        return false;
    }

    uint16_t vendor_id;
    char* path;

    if (!system_devs.empty()) {
        ML_ERR("Please open the Minter app");
        return false;
    } else if (!app_devs.empty()) {
        vendor_id = app_devs.begin()->get()->vendor_id;
        path = app_devs.begin()->get()->path;
    } else {
        ML_ERR("Please, connect your Nano S to computer and open the Minter app");
        return false;
    }

    m_framer = hidpp_device(vendor_id, product_id);

    if (!m_framer.io().open()) {
        ML_ERR("Unable to open device {0}\n\t {1}", path, m_framer.io().getError());
        if (errno == EACCES) {
            ML_ERR("Try to run this executable with sudo or grant to {0} read-write permissions to your user", path);
        }
        return false;
    }
    m_valid = m_framer.io().valid();
    return m_valid;
}

bool minter::nanos_wallet::init_root() {
    std::lock_guard<std::mutex> lock(m_devLock);

    auto system_devs = m_hid.enumerate_devices(LEDGER_VID, NANOS_PID_ROOT);
    if (system_devs.empty()) {
        ML_ERR("Please, connect your Nano S to computer");
        return false;
    }

    uint16_t vendor_id, product_id;
    char* path;

    if (system_devs.empty()) {
        ML_ERR("Please open the Minter app");
        return false;
    }

    vendor_id = system_devs.begin()->get()->vendor_id;
    product_id = system_devs.begin()->get()->product_id;
    path = system_devs.begin()->get()->path;

    m_framer = hidpp_device(vendor_id, product_id);

    if (!m_framer.io().open()) {
        ML_ERR("Unable to open device {0}\n\t {1}", path, m_framer.io().getError());
        if (errno == EACCES) {
            ML_ERR("Try to run this executable with sudo or grant to {0} read-write permissions to your user", path);
        }
        return false;
    }
    m_valid = m_framer.io().valid();
    return m_valid;
}

tb::bytes_data minter::nanos_wallet::exchange(const minter::APDU& apdu, uint16_t* res_code) {
    std::lock_guard<std::mutex> lock(m_devLock);
    if (!m_valid) {
        throw std::runtime_error("trying to send data to uninitialized device");
    }

    tb::bytes_data out;
    bool success = false;

    // sometimes, ledger nano s is shitting to our input part of IO with strange data
    // do exchange until we don't receive valid result
    do {
        try {
            if (!m_framer.io().get()) {
                m_valid = false;
                break;
            }
            out = m_framer.exchange(apdu, res_code);
            success = true;
        } catch (const std::out_of_range& e) {
            ML_ERR("Invalid response length: {0}", e.what());
            throw e;
        } catch (const minter::wrong_seqno_error& e) {
            m_framer.io().close();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            m_framer.io().open();
        }
    } while (!success);

    return out;
}

minter::address_t minter::nanos_wallet::get_address(uint32_t derive_index, bool silent) {
    minter::APDU data = {
        DEVICE_CLASS,
        CMD_GET_ADDRESS,
        silent ? (uint8_t) 0x01 : (uint8_t) 0x00,
        0,
        4,
        {0, 0, 0, 0}};

    std::vector<uint8_t> tmp(4);
    tb::num_to_bytes<uint32_t>(derive_index, tmp);
    memmove(data.payload, tmp.data(), 4);
    tmp.clear();

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if (status != CODE_SUCCESS) {
        ML_ERR("Unable to get address: {0}", minter::utils::ledger_status_to_string(status));
        throw exchange_error(fmt::format("Unable to get address: {0}", minter::utils::ledger_status_to_string(status)), status);
    }

    return minter::address_t(response.take_range(0, 20));
}

minter::signature minter::nanos_wallet::sign_tx(const tb::bytes_data& raw_tx, uint32_t derive_index) {
    tb::bytes_data unsignedHash(36);
    unsignedHash.write(0, (uint32_t) derive_index);
    unsignedHash.write(4, raw_tx);
    minter::APDU data = {
        DEVICE_CLASS,
        CMD_SIGN_TX,
        0,
        0,
        4 + 32, // 4 bytes derive index + 32 bytes hash
        {}};
    memmove(data.payload, unsignedHash.cdata(), unsignedHash.size());

    minter::signature out;
    out.success = false;

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);

    if (status != CODE_SUCCESS) {
        ML_ERR("Unable to sign transaction: {0}", minter::utils::ledger_status_to_string(status));
        throw exchange_error(fmt::format("Unable to sign transaction: {0}", minter::utils::ledger_status_to_string(status)), status);
    }

    out.r = response.take_first(32);
    out.s = response.take_range(32, 64);
    out.v = response.take_last(1);
    out.success = true;

    return out;
}

std::string minter::nanos_wallet::get_app_version() {
    minter::APDU data{
        DEVICE_CLASS,
        CMD_GET_VERSION,
        0,
        0,
        0,
        {}};

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if (status != CODE_SUCCESS) {
        throw exchange_error(fmt::format("Unable to get app version: {0}", minter::utils::ledger_status_to_string(status)), status);
    }

    std::stringstream ss;

    uint32_t maj, min, patch;
    maj = (uint32_t) response.at(0);
    min = (uint32_t) response.at(1);
    patch = (uint32_t) response.at(2);

    ss << maj << "." << min << "." << patch;

    return ss.str();
}

std::vector<minter::app_item> minter::nanos_wallet::get_app_list() {
    minter::APDU data{
        DEVICE_CLASS,
        0xDE,
        0,
        0,
        0,
        {}};

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if (status != CODE_SUCCESS) {
        throw exchange_error(fmt::format("Unable to get app version: {0}", minter::utils::ledger_status_to_string(status)), status);
    }
    if (response.empty()) {
        return {};
    }

    std::vector<minter::app_item> out;
    size_t offset = 1;
    while (offset != response.size()) {
        minter::app_item item;
        offset++;
        item.flags = response.to_num<uint32_t>(offset);
        std::cout << item.flags << std::endl;

        offset += sizeof(uint32_t);
        item.hash_code_data = response.take_range(offset, offset + 32);
        offset += 32;

        item.hash = response.take_range(offset, offset + 32);
        offset += 32;

        uint8_t nameLen = response[offset];
        tb::bytes_data nameBytes = (tb::bytes_data) response.take_range_c(offset + 1, offset + 1 + nameLen);
        offset += 1 + nameLen;
        item.name = nameBytes.to_string();

        out.push_back(std::move(item));
    }

    return out;
}

uint16_t minter::nanos_wallet::run_app(const std::string& name) {
    minter::APDU data{
        DEVICE_CLASS,
        0xD8,
        0,
        0,
        name.size(),
        {}};

    tb::bytes_data payload = tb::bytes_data::from_string_raw(name);
    memmove(data.payload, payload.cdata(), payload.size());

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if (status != CODE_SUCCESS) {
        throw exchange_error(fmt::format("Unable to run app {0}: {1}", name, minter::utils::ledger_status_to_string(status)), status);
    }

    bool found = false;
    uint16_t pid = 0;
    size_t max_wait_tries = 300; // 200 milliseconds * 300 cycles = 60 sec
    size_t n = 0;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        auto system_devs = m_hid.enumerate_devices(LEDGER_VID, 0);
        for (const auto& dev : system_devs) {
            if (dev.get()->product_id != NANOS_PID_ROOT) {
                found = true;
                pid = dev.get()->product_id;
            }
        }
        n++;
        if (n >= max_wait_tries) {
            break;
        }
    } while (!found);

    return pid;
}

uint16_t minter::nanos_wallet::get_opened_app_pid() {
    std::lock_guard<std::mutex> lock(m_devLock);
    auto system_devs = m_hid.enumerate_devices(LEDGER_VID, 0);
    uint16_t pid = 0;
    for (const auto& dev : system_devs) {
        if (dev.get()->product_id != NANOS_PID_ROOT) {
            pid = dev.get()->product_id;
            break;
        }
    }

    return pid;
}
