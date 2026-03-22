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

        virtual void loadValidClients(
                std::function<void(std::vector<std::string>)> callback);

        virtual void insertNewClient(std::string senderCompId, bool isActive);
        virtual void removeAllClients();

    private:
        DatabaseWorker* mDb;
    };
} // namespace data
