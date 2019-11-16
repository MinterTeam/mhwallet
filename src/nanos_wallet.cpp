/*!
 * mhwallet.
 * nanos_wallet.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <stdexcept>
#include "minter/nanos_wallet.h"

minter::nanos_wallet::nanos_wallet() :
    m_hid(),
    m_framer(hidpp_device(LEDGER_VID, NANOS_PID)),
    m_valid(false) {
}

bool minter::nanos_wallet::init() {
    // check device is exists in system
    auto devs = m_hid.enumerate_devices(LEDGER_VID, NANOS_PID);
    if (devs.empty()) {
        ML_ERR("Please, connect your Nano S to computer");
        return false;
    } else if (devs.size() > 1) {
        ML_ERR("Did you opened Minter wallet?");
        return false;
    }

    auto dev_info = devs.begin();

    if (!m_framer.io().open()) {
        ML_ERR("Unable to open device {0}: {1}", dev_info->get()->path, m_framer.io().getError());
        if (errno == EACCES) {
            ML_ERR("Try to run this executable with sudo or grant to {0} read-write permissions to your user", dev_info->get()->path);
        }
        return false;
    }
    m_valid = m_framer.io().valid();
    return true;
}


bytes_data minter::nanos_wallet::exchange(const minter::APDU &apdu, uint16_t *resCode) {
    // only 1 thread can use device at time
    std::unique_lock<std::mutex> lock(m_devLock);

    if(!m_valid) {
        throw std::runtime_error("trying to send data to uninitialized device");
    }

    bytes_data out;
    try {
        out = m_framer.exchange(apdu, resCode);
    } catch(const std::out_of_range &e) {
        ML_ERR("Invalid response length: {0}", e.what());
        return bytes_data();
    }

    return out;
}

minter::address_t minter::nanos_wallet::get_address(uint32_t deriveIndex) {
    minter::APDU data = {
        DEVICE_CLASS,
        CMD_GET_ADDRESS,
        0,
        0,
        4,
        {0, 0, 0, 0}
    };

    std::vector<uint8_t> tmp(4);
    numToBytes<uint32_t>(deriveIndex, tmp);
    memmove(data.payload, tmp.data(), 4);
    tmp.clear();

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if(status != CODE_SUCCESS) {
        ML_ERR("Unable to get address: {0}", statusToString(status));
        return minter::address_t();
    }

    return minter::address_t(response.take_range(0, 20));
}

minter::signature minter::nanos_wallet::sign_tx(bytes_data txHash, uint32_t deriveIndex) {
    bytes_data unsignedHash(36);
    unsignedHash.write(0, (uint32_t) deriveIndex);
    unsignedHash.write(4, txHash);
    minter::APDU data = {
        DEVICE_CLASS,
        CMD_SIGN_TX,
        0,
        0,
        4 + 32,
        {}
    };
    memmove(data.payload, unsignedHash.cdata(), unsignedHash.size());

    minter::signature out;
    out.success = false;

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if(status != CODE_SUCCESS) {
        ML_ERR("Unable to sign transaction: {0}", statusToString(status));
        return out;
    }

    out.r = response.take_first(32);
    out.s = response.take_range(32, 64);
    out.v = response.take_last(1);
    out.success = true;

    return out;
}

std::string minter::nanos_wallet::get_app_version() {
    minter::APDU data {
        DEVICE_CLASS,
        CMD_GET_VERSION,
        0, 0, 0, {}
    };

    uint16_t status = CODE_SUCCESS;
    auto response = exchange(data, &status);
    if(status != CODE_SUCCESS) {
        return "";
    }

    std::stringstream ss;

    uint32_t maj, min, patch;
    maj = (uint32_t)response.at(0);
    min = (uint32_t)response.at(1);
    patch = (uint32_t)response.at(2);

    ss << maj << "." << min << "." << patch;

    return ss.str();
}