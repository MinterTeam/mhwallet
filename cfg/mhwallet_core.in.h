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

#endif // MINTER_MHWALLET_CORE_H