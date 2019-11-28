/*!
 * mhwallet.
 * frame_io_apdu.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_FRAME_IO_APDU_H
#define MHWALLET_FRAME_IO_APDU_H

#include "hidpp_base.h"
#include "hidpp.h"
#include "frame_io.h"

namespace minter {

class MINTER_MH_API frame_io_apdu : public minter::frame_io {
 public:
    frame_io_apdu(const hidpp_device &dev);
    frame_io_apdu(hidpp_device &&dev);
    tb::bytes_data exchange(const APDU &apdu, uint16_t *statusCode = nullptr);

 private:
    tb::bytes_data buf;
    size_t rn;
};

} // minter

#endif //MHWALLET_FRAME_IO_APDU_H
