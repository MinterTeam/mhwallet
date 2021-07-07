//
// Created by Эдуард Максимович on 04.06.2021.
//

#ifndef MHWALLET_ERRORS_H
#define MHWALLET_ERRORS_H

#include "mhwallet_core.h"
#include "utils.h"

#include <exception>
#include <fmt/format.h>
#include <utility>

namespace minter {

struct MINTER_MH_API wrong_seqno_error : std::exception {
    wrong_seqno_error(uint16_t expected, uint16_t given)
        : msg(fmt::format("bad sequence number {0} (expected {1})", given, expected))
        , expected(expected)
        , given(given) {
    }
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }

    const std::string msg;
    uint16_t expected = 0, given = 0;
};

class MINTER_MH_API exchange_error : public std::exception {
public:
    exchange_error(std::string message, uint16_t code)
        : m_msg(std::move(message))
        , m_code(code) {
    }

    exchange_error(uint16_t code)
        : m_msg(fmt::format("unable to exchange: code={}", code))
        , m_code(code) {
    }

    [[nodiscard]] const char* what() const noexcept override {
        return m_msg.c_str();
    }

    uint16_t code() const {
        return m_code;
    }

    std::string codeString() const {
        return minter::utils::ledger_status_to_string(m_code);
    }

private:
    const std::string m_msg;
    const uint16_t m_code;
};

} // namespace minter

#endif // MHWALLET_ERRORS_H
