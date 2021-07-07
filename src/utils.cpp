/*!
 * mhwallet.
 * utils.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "minter/ledger/utils.h"

std::string minter::utils::ledger_status_to_string(uint16_t status) {
    switch (status) {
    case CODE_SUCCESS:
        return "success";
    case CODE_NO_STATUS_RESULT:
        return "no_status_result";
    case CODE_INVALID_PARAM:
        return "invalid_parameter";
    case CODE_USER_REJECTED:
        return "user_rejected";
    default:
        return "unknown_error";
    }
}
