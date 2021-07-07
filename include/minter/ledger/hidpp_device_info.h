/*!
 * mhwallet.
 * hidpp_device_info.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_HIDPP_DEVICE_INFO_H
#define MHWALLET_HIDPP_DEVICE_INFO_H

#include "hidpp_base.h"

namespace minter {

class MINTER_MH_API hidpp_device_info {
public:
    explicit hidpp_device_info(hid_device_info* ptr);
    [[nodiscard]] const dev_info_t& get() const;

private:
    dev_info_t m_info;
};

} // namespace minter

#endif // MHWALLET_HIDPP_DEVICE_INFO_H