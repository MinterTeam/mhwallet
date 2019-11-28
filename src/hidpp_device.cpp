/*!
 * mhwallet.
 * hidpp_device.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#include <locale.h>
#include <cwchar>

#ifdef _MSC_VER
//#include <Windows.h>
#include <cuchar>
#include <corecrt.h>
#endif

#include "minter/hidpp_device.h"

minter::hidpp_device::hidpp_device(const minter::hidpp_device_info &info) :
    m_dev(nullptr),
    m_vendorId(info.get()->vendor_id),
    m_productId(info.get()->product_id) {
}

minter::hidpp_device::hidpp_device(uint16_t vendorId, uint16_t productId) :
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

const hid_device *minter::hidpp_device::cget() const {
    return m_dev;
}

hid_device *minter::hidpp_device::get() {
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

size_t minter::hidpp_device::write(const tb::bytes_data &data) const {
    return write(data.cdata(), data.size());
}

size_t minter::hidpp_device::read(tb::bytes_data &out, size_t readLen) const {
    std::vector<uint8_t> buf(readLen);
    auto res = hid_read(m_dev, buf.data(), readLen);
    auto err = hid_error(m_dev);
    if (err) {
        std::string errm;
        get_dev_error(errm);
        ML_ERR("Unable to read {0} bytes: {1}", readLen, errm);
        return 0;
    }

    out.write(0, buf);

    return res;
}

size_t minter::hidpp_device::read_back(tb::bytes_data &out, size_t seq, size_t readLen) const {
    std::vector<uint8_t> buf(readLen);
    hid_read(m_dev, buf.data(), readLen);
    auto err = hid_error(m_dev);
    if (err) {
        std::string errm;
        get_dev_error(errm);
        ML_ERR("Unable to read {0} bytes: {1}", readLen, errm);
        return 0;
    }

    size_t pos = seq * readLen;
    if (out.size() <= pos) {
        out.resize(out.size() + pos);
    }
    out.write(seq * readLen, buf.data(), readLen);
    return readLen;
}

size_t minter::hidpp_device::read(uint8_t *out, size_t readLen) const {
    if (!out) {
        throw std::runtime_error("Output variable is null");
    }
    #ifdef _MSC_VER
    hid_set_nonblocking(m_dev, 0);
    return hid_read(m_dev, out, readLen);
    #else
    return hid_read(m_dev, out, readLen);
    #endif
}

void minter::hidpp_device::get_dev_error(std::string &out) const {
    std::stringstream ss;

    if (m_dev != nullptr) {
        const wchar_t *err = hid_error(m_dev);
        if (err != nullptr) {
            std::wstring tmp(err);

            setlocale(LC_ALL, "");
            const std::locale locale("");
            typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
            const auto &converter = std::use_facet<converter_type>(locale);
            std::vector<char> to(tmp.length() * converter.max_length());
            std::mbstate_t state;
            const wchar_t *from_next;
            char *to_next;
            const converter_type::result result = converter
                .out(state, tmp.data(), tmp.data() + tmp.length(), from_next, &to[0], &to[0] + to.size(), to_next);
            if (result == converter_type::ok || result == converter_type::noconv) {
                const std::string s(&to[0], to_next);
                ss << "HID error: " << s;
                ss << ";";
            }
        }
    }

    if (errno == 0) {
        out = ss.str();
        return;
    }

    std::string errv;

    #ifdef _MSC_VER
    char strerr_buf[255];
    strerror_s(strerr_buf, 255, errno);
    errv = std::string(strerr_buf);
    #else
    errv = std::string(std::strerror(errno));
    #endif
    ss << "[" << errno << "] " << errv;
    out = ss.str();
}