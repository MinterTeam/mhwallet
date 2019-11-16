/*!
 * mhwallet.
 * hidpp_device_info.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "minter/hidpp_device_info.h"

minter::hidpp_device_info::hidpp_device_info(hid_device_info *ptr) : m_info(dev_info_t(ptr)) { }

const minter::dev_info_t &minter::hidpp_device_info::get() const {
    return m_info;
}