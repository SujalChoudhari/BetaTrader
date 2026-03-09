#include "data/AuthRepository.h"
#include "data/DataRunBookDefinations.h"
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
                // Create table
                SQLite::Statement createQuery(db, data::query::createClientTableQuery);
                createQuery.exec();

                // Check if empty
                int count = db.execAndGet("SELECT COUNT(*) FROM clients").getInt();
                
                // Seed default test clients for initial setup
                if (count == 0) {
                    SQLite::Transaction transaction(db);
                    
                    SQLite::Statement insertQuery(db, data::query::insertClientQuery);
                    
                    insertQuery.bind(1, "TRADER_BOB");
                    insertQuery.bind(2, 1); // active
                    insertQuery.exec();
                    insertQuery.reset();

                    insertQuery.bind(1, "ALICE_FIRM");
                    insertQuery.bind(2, 1); // active
                    insertQuery.exec();
                    
                    transaction.commit();
                    LOG_INFO("AuthRepository: Seeded default clients (TRADER_BOB, ALICE_FIRM).");
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA13, "Error in AuthRepository::initDatabase: {}", std::string_view(e.what()));
            }
        });
    }

    void AuthRepository::loadValidClients(std::function<void(std::vector<std::string>)> callback)
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
                LOG_ERROR(errors::EDATA14, "Error in AuthRepository::loadValidClients: {}", std::string_view(e.what()));
            }
            callback(clients);
        });
    }

} // namespace data
