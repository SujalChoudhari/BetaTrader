#include "data/AuthRepository.h"
#include "common_data/DataRunBookDefinations.h"
#include "data/Query.h"
#include "logging/Runbook.h"

namespace data {

    AuthRepository::AuthRepository(DatabaseWorker* dbWorker): mDb(dbWorker)
    {
        AuthRepository::initDatabase();
    }

    void AuthRepository::initDatabase()
    {
        mDb->enqueue([](SQLite::Database& db) {
            try {
                SQLite::Statement createQuery(
                        db, data::query::createClientTableQuery);
                createQuery.exec();
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA13,
                          "Error in AuthRepository::initDatabase: {}",
                          std::string_view(e.what()));
            }
        });
    }

    void AuthRepository::loadValidClients(
            std::function<void(std::vector<std::string>)> callback)
    {
        mDb->enqueue([callback](const SQLite::Database& db) {
            std::vector<std::string> clients;
            try {
                SQLite::Statement query(db, query::loadClientsQuery);

                while (query.executeStep()) {
                    clients.push_back(query.getColumn(0).getText());
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA14,
                          "Error in AuthRepository::loadValidClients: {}",
                          std::string_view(e.what()));
            }
            callback(clients);
        });
    }

    void AuthRepository::insertNewClient(std::string senderCompId,
                                         bool isActive)
    {
        mDb->enqueue([senderCompId, isActive](const SQLite::Database& db) {
            SQLite::Statement insertQuery(db, data::query::insertClientQuery);

            insertQuery.bind(1, senderCompId);
            insertQuery.bind(2, isActive);
            insertQuery.exec();
            LOG_INFO("Created a new client {}", senderCompId);
        });
    }

    void AuthRepository::removeAllClients()
    {
        mDb->enqueue([](const SQLite::Database& db) {
            SQLite::Statement createQuery(db,
                                          data::query::truncateClientsQuery);
            createQuery.exec();
            LOG_INFO("Removed all clients");
        });
    }
} // namespace data
