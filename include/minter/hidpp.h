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

#include <vector>
#include <memory>

#include "hidpp_base.h"
#include "hidpp_device.h"
#include "hidpp_device_info.h"

namespace minter {

class hidpp {
 public:
    hidpp();
    ~hidpp();

    std::vector<hidpp_device_info> enumerate_devices(uint16_t vendorId, uint16_t productId);
};

} // minter

#endif //MHWALLET_HIDPP_DEVICE_H
