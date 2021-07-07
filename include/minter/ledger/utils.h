/*!
 * mhwallet.
 * utils.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MHWALLET_UTILS_H
#define MHWALLET_UTILS_H

#include "mhwallet_core.h"

#include <string>

namespace minter {
namespace utils {

MINTER_MH_API std::string ledger_status_to_string(uint16_t status);

} // namespace utils
} // namespace minter

#endif // MHWALLET_UTILS_H
