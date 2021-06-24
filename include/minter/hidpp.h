/*!
 * mhwallet.
 * hidpp.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_HIDPP_H
#define MHWALLET_HIDPP_H

#include "hidpp_base.h"
#include "hidpp_device.h"
#include "hidpp_device_info.h"

#include <memory>
#include <vector>

namespace minter {

class MINTER_MH_API hidpp {
public:
    hidpp();
    ~hidpp();

    std::vector<hidpp_device_info> enumerate_devices(uint16_t vendorId, uint16_t productId);
};

} // namespace minter

#endif // MHWALLET_HIDPP_DEVICE_H
