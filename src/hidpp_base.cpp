//
// Created by edward on 13.11.2019.
//

#include "minter/hidpp.h"
#include "minter/hidpp_base.h"
#include "minter/hidpp_device_info.h"

minter::hidpp::hidpp() {
    hid_init();
}
minter::hidpp::~hidpp() {
    hid_exit();
}

std::vector<minter::hidpp_device_info> minter::hidpp::enumerate_devices(uint16_t vendorId, uint16_t productId) {
    std::vector<hidpp_device_info> infos;
    hid_device_info *current = hid_enumerate(vendorId, productId);
    while (current != nullptr) {
        infos.push_back(hidpp_device_info(current));
        current = current->next;
    }

    return infos;
}