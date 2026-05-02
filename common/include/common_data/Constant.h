/**
 * @file Constant.h
 * @brief Constants used by the data layer (database paths and similar).
 *
 * This header contains small, shared constants used by the persistence
 * implementation. It is intended to be included by implementation files in
 * the `data` module.
 */

#pragma once
#include <string>

namespace data {
    const std::string databasePath = "exchange.db";
}
