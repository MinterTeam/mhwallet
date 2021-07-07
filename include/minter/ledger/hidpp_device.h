/*!
 * mhwallet.
 * hidpp_device.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_HIDPP_DEVICE_H
#define MHWALLET_HIDPP_DEVICE_H

#include "hidpp_base.h"
#include "hidpp_device_info.h"

#include <cerrno>
#include <mutex>
#include <string>
#include <vector>

namespace minter {

class MINTER_MH_API hidpp_device {
public:
    hidpp_device(const hidpp_device_info& info);
    hidpp_device(uint16_t vendorId, uint16_t productId);
    hidpp_device(const char* path);
    ~hidpp_device();

    // copy ctors
    hidpp_device(const hidpp_device& other);
    hidpp_device& operator=(const hidpp_device& other);

    bool valid() const;
    bool open();
    void close();

    std::string getError() const;
    [[nodiscard]] const hid_device* cget() const;
    [[nodiscard]] hid_device* get();

    size_t write(const uint8_t* data, size_t len) const;
    size_t write(const tb::bytes_data& data) const;
    size_t read(tb::bytes_data& out, size_t readLen) const;
    size_t read_back(tb::bytes_data& out, size_t seq, size_t readLen) const;
    size_t read(uint8_t* out, size_t readLen) const;

private:
    void get_dev_error(std::string& out) const;
    hid_device* m_dev;
    uint16_t m_vendorId;
    uint16_t m_productId;
    const char* m_path;
    mutable std::mutex m_dev_lock;
};

} // namespace minter

#endif // MHWALLET_HIDPP_H
