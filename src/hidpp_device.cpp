/*!
 * mhwallet.
 * hidpp_device.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#include <cwchar>
#include <locale.h>

#ifdef _MSC_VER
//#include <Windows.h>
#include <corecrt.h>
#include <cuchar>
#endif

#include "minter/hidpp_device.h"

#include <locale>

minter::hidpp_device::hidpp_device(const minter::hidpp_device_info& info)
    : m_dev(nullptr),
      m_vendorId(info.get()->vendor_id),
      m_productId(info.get()->product_id),
      m_path(nullptr),
      m_dev_lock() {
}

minter::hidpp_device::hidpp_device(uint16_t vendorId, uint16_t productId)
    : m_dev(nullptr),
      m_vendorId(vendorId),
      m_productId(productId),
      m_path(nullptr),
      m_dev_lock() {
}

minter::hidpp_device::hidpp_device(const char* path)
    : m_dev(nullptr),
      m_vendorId((uint16_t) 0),
      m_productId((uint16_t) 0),
      m_path(path),
      m_dev_lock() {
}

minter::hidpp_device::hidpp_device(const minter::hidpp_device& other) {
    m_dev = other.m_dev;
    m_vendorId = other.m_vendorId;
    m_productId = other.m_productId;
    m_path = other.m_path;
}
minter::hidpp_device& minter::hidpp_device::operator=(const minter::hidpp_device& other) {
    m_dev = other.m_dev;
    m_vendorId = other.m_vendorId;
    m_productId = other.m_productId;
    m_path = other.m_path;
    return *this;
}

minter::hidpp_device::~hidpp_device() {
    close();
}

bool minter::hidpp_device::valid() const {
    return m_dev != nullptr;
}

bool minter::hidpp_device::open() {
    std::lock_guard<std::mutex> lock(m_dev_lock);
    if (m_vendorId == 0 || m_productId == 0) {
        m_dev = hid_open_path(m_path);
    } else {
        m_dev = hid_open(m_vendorId, m_productId, nullptr);
    }

    if (m_dev) {
        hid_set_nonblocking(m_dev, 0);
    }

    return m_dev != nullptr;
}

void minter::hidpp_device::close() {
    std::lock_guard<std::mutex> lock(m_dev_lock);
    if (m_dev) {
        hid_close(m_dev);
        m_dev = nullptr;
    }
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

size_t minter::hidpp_device::write(const uint8_t* data, size_t len) const {
    std::lock_guard<std::mutex> lock(m_dev_lock);
    return hid_write(m_dev, data, len);
}

size_t minter::hidpp_device::write(const tb::bytes_data& data) const {
//    std::lock_guard<std::mutex> lock(m_dev_lock);
#ifdef WIN32
    // https://www.microchip.com/forums/m1001939.aspx
    tb::bytes_data with_hid_report(data.size() + 1);
    with_hid_report.write(0, (uint8_t) 0);
    with_hid_report.write(1, data);
    return write(with_hid_report.cdata(), with_hid_report.size());
#else
    return write(data.cdata(), data.size());
#endif
}

/**
 * \brief Special behavior for windows: read length must be N+1, then remove report code [1:N] and return again N bytes as expected
 * \param out
 * \param readLen
 * \return
 */
size_t minter::hidpp_device::read(tb::bytes_data& out, size_t readLen) const {
    size_t targetReadLen = readLen;
#ifdef _MSC_VER
    targetReadLen = readLen + 1;
    std::vector<uint8_t> buf(targetReadLen);
#else
    std::vector<uint8_t> buf(readLen);
#endif

    std::lock_guard<std::mutex> lock(m_dev_lock);
    auto res = hid_read(m_dev, buf.data(), targetReadLen);
    auto err = hid_error(m_dev);
    if (err) {
        std::string errm;
        get_dev_error(errm);
        ML_ERR("Unable to read {0} bytes: {1}", readLen, errm);
        return 0;
    } else {
        std::cout << "Read buf: " << dumpHexRet(tb::bytes_data(buf)) << std::endl;
    }

#ifdef _MSC_VER
    tb::bytes_data tmp(buf);
    out.write(0, tmp.take_range(1, targetReadLen));
    std::cout << "aaa" << std::endl;

    return res - 1;
#else
    out.write(0, buf);
#endif

    return res;
}

size_t minter::hidpp_device::read_back(tb::bytes_data& out, size_t seq, size_t readLen) const {
    std::vector<uint8_t> buf(readLen);

    {
        std::lock_guard<std::mutex> lock(m_dev_lock);
        hid_read(m_dev, buf.data(), readLen);
        auto err = hid_error(m_dev);
        if (err) {
            std::string errm;
            get_dev_error(errm);
            ML_ERR("Unable to read {0} bytes: {1}", readLen, errm);
            return 0;
        }
    }

    size_t pos = seq * readLen;
    if (out.size() <= pos) {
        out.resize(out.size() + pos);
    }
    out.write(seq * readLen, buf.data(), readLen);
    return readLen;
}

size_t minter::hidpp_device::read(uint8_t* out, size_t readLen) const {
    if (!out) {
        throw std::runtime_error("Output variable is null");
    }
    std::lock_guard<std::mutex> lock(m_dev_lock);
    //#ifdef _MSC_VER
    //    hid_set_nonblocking(m_dev, 0);
    //    return hid_read(m_dev, out, readLen);
    //#else
    return hid_read(m_dev, out, readLen);
    //#endif
}

void minter::hidpp_device::get_dev_error(std::string& out) const {
    std::lock_guard<std::mutex> lock(m_dev_lock);
    std::stringstream ss;

    if (m_dev != nullptr) {
        const wchar_t* err = hid_error(m_dev);
        if (err != nullptr) {
            std::wstring tmp(err);

            setlocale(LC_ALL, "");
            const std::locale locale("");
            using converter_type = std::codecvt<wchar_t, char, std::mbstate_t>;
            const auto& converter = std::use_facet<converter_type>(locale);
            std::vector<char> to(tmp.length() * converter.max_length());
            std::mbstate_t state;
            const wchar_t* from_next;
            char* to_next;
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
