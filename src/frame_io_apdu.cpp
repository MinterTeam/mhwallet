/*!
 * mhwallet.
 * frame_io_apdu.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */


#include "minter/frame_io_apdu.h"

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

bytes_data minter::frame_io_apdu::exchange(const minter::APDU &apdu, uint16_t *statusCode) {
    if (apdu.payload_size > 0xFF) {
        throw std::runtime_error("payload size can't be more than 255");
    }

    size_t n = 0;
    reset();
    n = write(apdu);
    ML_LOG("Write APDU frame ({0}) bytes", n);

    bytes_data buffer(255);
    rn += read(buffer) + 5;

    ML_LOG("Read APDU frame (2 bytes)");

    // read APDU payload
    auto respLen = buffer.to_num<uint16_t>(0) + 2;
    ML_LOG("Response length (raw): {0}", respLen);

    while (rn < respLen) {
        rn += read(buffer) + 5;
    }

    bytes_data resp;
    bytes_data respCode(2);

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

    return resp;
}
