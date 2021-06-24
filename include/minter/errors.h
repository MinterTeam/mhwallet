//
// Created by Эдуард Максимович on 04.06.2021.
//

#ifndef MHWALLET_ERRORS_H
#define MHWALLET_ERRORS_H

#include <exception>
#include <fmt/format.h>

namespace minter {

struct MINTER_MH_API wrong_seqno_error : std::exception {
    wrong_seqno_error(uint16_t expected, uint16_t given)
        : msg(fmt::format("bad sequence number {0} (expected {1})", given, expected)),
          expected(expected),
          given(given) {
    }
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }

    const std::string msg;
    uint16_t expected = 0, given = 0;
};

} // namespace minter

#endif // MHWALLET_ERRORS_H
