/*!
 * minter_tx.
 * minter_tx_core.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MINTER_MHWALLET_CORE_H
#define MINTER_MHWALLET_CORE_H

#cmakedefine MINTER_MH_EXPORTING 1
#cmakedefine MINTER_EXPORT_SHARED 1

#ifdef MINTER_EXPORT_SHARED
#ifdef MINTER_MH_EXPORTING
#if _MSC_VER
#define MINTER_MH_API __declspec(dllexport)
#else
#define MINTER_MH_API __attribute__((visibility("default")))
#endif
#else
#if _MSC_VER
#define MINTER_MH_API __declspec(dllimport)
#else
#define MINTER_MH_API
#endif
#endif
#else
#define MINTER_MH_API
#endif // MINTER_EXPORT_SHARED

#if defined(_MSC_VER) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <stdint.h>

#define DEVICE_CLASS 0xe0

// Vendor/Product id
#define LEDGER_VID 0x2c97
#define NANOS_PID_ROOT 0x1011
#define NANOS_PID 0x0001
#define NANOS_PID_16_APP 0x1005
#define NANOX_PID 0x0004

// Status codes
const uint16_t CODE_SUCCESS = 0x9000;
const uint16_t CODE_USER_REJECTED = 0x6985;
const uint16_t CODE_INVALID_PARAM = 0x6b01;
const uint16_t CODE_NO_STATUS_RESULT = 0x9001;

#endif // MINTER_MHWALLET_CORE_H
