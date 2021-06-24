/*!
 * mhwallet.
 * frame_io.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "minter/frame_io.h"

#include "minter/errors.h"

#include <toolbox/data/literals.h>
#include <utility>

minter::frame_io::frame_io(const minter::hidpp_device& dev)
    : m_io(dev),
      m_seq(0),
      m_offset(0),
      m_buffer(64) {
}

minter::frame_io::frame_io(minter::hidpp_device&& dev)
    : m_io(std::move(dev)),
      m_seq(0),
      m_offset(0),
      m_buffer(64) {
}

#include <chrono>
#include <thread>

size_t minter::frame_io::read(tb::bytes_data& out) {
    size_t n = 0;
    n = io().read(m_buffer, 64);
    ML_LOG("Read data [{0}:{1}] {2}", m_seq, 64, dumpHexRet(m_buffer));
    if (n != 64) {
        return 0;
    }

    // parse header 5 bytes
    // 2 bytes
    auto channelId = m_buffer.to_num<uint16_t>(0);
    // 1 byte
    uint8_t commandTag = m_buffer.at(2);
    // 2 bytes
    auto cseq = m_buffer.to_num<uint16_t>(3);

    if (channelId != 0x0101) {
        throw std::runtime_error(fmt::format("bad channel ID 0x{0:2X}", channelId));
    } else if (commandTag != 0x05) {
        throw std::runtime_error(fmt::format("bad command tag 0x{0:2X}", commandTag));
    } else if (cseq != this->m_seq) {
        throw minter::wrong_seqno_error(m_seq, cseq);
    }

    if (m_seq == 0) {
        n = out.write(0, m_buffer.take_range_from(5));
    } else {
        // as we have "next" portion of data, we should write "next" part of data to tail with offset
        n = out.write(m_offset - 5, m_buffer.take_range_from(5));
    }

    m_seq++;

    m_offset += 64;
    return n;
}
size_t minter::frame_io::write(const minter::APDU& apdu) {
    auto apduData = apdu.to_bytes();
    tb::bytes_data chunk(64);
    chunk.write(0, (uint16_t) 0x0101);
    chunk.write(2, (uint8_t) 0x05);

    auto dataLen = (uint16_t) (apduData.size() & (uint16_t) 0xFFFF);

    tb::bytes_buffer buffer(apduData.size() + 2);
    // write 2 bytes data length prefix
    buffer.write(0, dataLen);
    // write APDU data
    buffer.write(2, apduData);

    uint16_t cseq = 0;

    while (!buffer.empty()) {
        chunk.write(3, cseq); // 2 bytes
        size_t n = buffer.pop_front_to(5, chunk);

        ML_LOG("Write data [{0}:{1}] {2}", cseq, chunk.size(), dumpHexRet(chunk));
        // first 5 bytes - service info
        // other data is an APDU frame
        auto toUsb = chunk.take_range_to(5 + n);

        cseq++;

        auto written = io().write(toUsb);
        ML_LOG("Write data [{0}:{1}] written {2} bytes", cseq, chunk.size(), written);
    }

    return apduData.size();
}
void minter::frame_io::reset() {
    m_seq = 0;
}

minter::hidpp_device& minter::frame_io::io() {
    return m_io;
}
