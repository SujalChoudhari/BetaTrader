/**
 * @file LogoutRequest.h
 * @brief Defines the structure for a FIX Logout message.
 */

#pragma once

#include <string>

namespace fix
{

/**
 * @brief Represents a parsed FIX Logout (35=5) message.
 *
 * This struct encapsulates the fields from a raw FIX message
 * for a client to gracefully terminate a session.
 */
struct LogoutRequest
{
    /** @brief Optional text message providing a reason for logout (Text, FIX Tag 58). */
    std::string text;
};

} // namespace fix
