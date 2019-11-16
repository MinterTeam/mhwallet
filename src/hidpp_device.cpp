/*!
 * mhwallet.
 * hidpp_device.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "minter/hidpp_device.h"

minter::hidpp_device::hidpp_device(const minter::hidpp_device_info &info):
    m_dev(nullptr),
    m_vendorId(info.get()->vendor_id),
    m_productId(info.get()->product_id) {
}

minter::hidpp_device::hidpp_device(uint16_t vendorId, uint16_t productId):
    m_dev(nullptr),
    m_vendorId(vendorId),
    m_productId(productId) {
}

minter::hidpp_device::~hidpp_device() {
    close();
}

bool minter::hidpp_device::valid() const {
    return m_dev != nullptr;
}

bool minter::hidpp_device::open() {
    m_dev = hid_open(m_vendorId, m_productId, nullptr);
    return m_dev != nullptr;
}

const hid_device* minter::hidpp_device::cget() const {
    return m_dev;
}

hid_device* minter::hidpp_device::get() {
    return m_dev;
}

std::string minter::hidpp_device::getError() const {
    std::string err;
    get_dev_error(err);
    return err;
}

size_t minter::hidpp_device::write(const uint8_t *data, size_t len) const {
    return hid_write(m_dev, data, len);
}

size_t minter::hidpp_device::write(const bytes_data &data) const {
    return write(data.cdata(), data.size());
}

size_t minter::hidpp_device::read(bytes_data &out, size_t readLen) const {
    auto res = hid_read(m_dev, out.data(), readLen);
    auto err = hid_error(m_dev);
    if (err) {
        std::cerr << "Unable to read " << readLen << " bytes: " << err << std::endl;
    }
    return res;
}

size_t minter::hidpp_device::read_back(bytes_data &out, size_t seq, size_t readLen) const {
    uint8_t buf[readLen];
    auto res = hid_read(m_dev, buf, readLen);
    auto err = hid_error(m_dev);
    if (err) {
        std::stringstream ss;
        ss << err;
        ML_ERR("Unable to read {0} bytes: {1}", readLen, ss.str());
        return 0;
    }

    size_t pos = seq * readLen;
    if(out.size() <= pos) {
        out.resize( out.size() + pos);
    }
    out.write(seq * readLen, buf, readLen);
    return readLen;
}

size_t minter::hidpp_device::read(uint8_t *out, size_t readLen) const {
    if (!out) {
        throw std::runtime_error("Output variable is null");
    }
    return hid_read(m_dev, out, readLen);
}

void minter::hidpp_device::get_dev_error(std::string &out) const {
    if (m_dev != nullptr) {
        const wchar_t *err = hid_error(m_dev);
        if (err != nullptr) {
            std::wstring tmp(err);
            out = std::string(tmp.begin(), tmp.end());
            return;
        }
    }

    std::stringstream ss;
    const std::string errv(std::strerror(errno));
    ss << "[" << errno << "] " << errv;
    out = ss.str();
}