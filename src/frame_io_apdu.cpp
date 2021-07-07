/*!
 * mhwallet.
 * frame_io_apdu.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "minter/ledger/frame_io_apdu.h"

minter::frame_io_apdu::frame_io_apdu(const minter::hidpp_device &dev) :
    frame_io(dev),
    buf(2),
    rn(0) {

}

minter::frame_io_apdu::frame_io_apdu(minter::hidpp_device &&dev) :
    frame_io(std::move(dev)),
    buf(2),
    rn(0) {

}

tb::bytes_data minter::frame_io_apdu::exchange(const minter::APDU &apdu, uint16_t *statusCode) {
    if (apdu.payload_size > (uint8_t) 0xFF) {
        throw std::runtime_error("payload size can't be more than 255");
    }

    size_t n = 0;
    // reset chunks counter
    reset();
    // IO write
    n = write(apdu);
    ML_LOG("Write APDU frame ({0}) bytes", n);

    // prepare response buffer
    tb::bytes_data buffer(255);
    // read maximum 255 bytes, returned bytes is a number of user data, without prefix, so adding 5 to this
    rn += read(buffer) + 5;

    ML_LOG("Read APDU frame (2 bytes)");

    // read length of full response data, not for this chunk
    uint16_t respLen = buffer.to_num<uint16_t>(0) + 2;
    ML_LOG("Response length (raw): {0}", respLen);

    // until we didn't get whole data, read again, buffer will be expanded from 255 to readLen + some additional bytes
    while (rn < respLen) {
        rn += read(buffer) + 5;
    }

    tb::bytes_data resp;
    tb::bytes_data respCode(2);

    if (statusCode) {
        *statusCode = CODE_NO_STATUS_RESULT;
    }
    if (respLen >= 2) {
        // response structure
        // [ 2 bytes - length prefix; N bytes - data; 2 bytes - status code]
        resp = buffer.take_range(2, respLen - 2);
        respCode = buffer.take_range(respLen - 2, respLen);

        if (statusCode) {
            *statusCode = respCode.to_num<uint16_t>();
        }
    }

    ML_LOG("Response data ({0} bytes): {1}", resp.size(), dumpHexRet(resp));
    // cleanup state
    m_seq = 0;
    m_offset = 0;
    rn = 0;
    buf.clear();

    return resp;
}
