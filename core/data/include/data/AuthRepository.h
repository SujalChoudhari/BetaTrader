/**
 * @file AuthRepository.h
 * @ingroup data_layer
 * @brief Repository for authorized FIX clients.
 */

#pragma once
#include "data/DatabaseWorker.h"
#include <functional>
#include <string>
#include <vector>

namespace data {
    /**
     * @class AuthRepository
     * @brief Persists and retrieves FIX Client authentication lists.
     */
    class AuthRepository {
    public:
        explicit AuthRepository(DatabaseWorker* dbWorker);
        virtual ~AuthRepository() = default;

        virtual void initDatabase();
        
        // Loads active clients and fires the callback
        virtual void loadValidClients(std::function<void(std::vector<std::string>)> callback);

    private:
        DatabaseWorker* mDb;
    };
} // namespace data
