/*!
 * mhwallet.
 * frame_io.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef MHWALLET_FRAME_IO_H
#define MHWALLET_FRAME_IO_H

#include "hidpp.h"
#include "hidpp_base.h"

namespace minter {

class MINTER_MH_API frame_io {
public:
    frame_io(const hidpp_device& dev);
    frame_io(hidpp_device&& dev);

    size_t read(tb::bytes_data& out);
    size_t write(const APDU& apdu);
    void reset();
    hidpp_device& io();

protected:
    hidpp_device m_io;
    uint16_t m_seq = 0;
    size_t m_offset = 0;
    tb::bytes_buffer m_buffer;
};

} // namespace minter

#endif // MHWALLET_FRAME_IO_H
