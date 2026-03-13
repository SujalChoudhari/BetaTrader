#include "data/SequenceRepository.h"
#include "data/Query.h"
#include "logging/Logger.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <future>
#include <mutex>

namespace data {

    SequenceRepository::SequenceRepository(DatabaseWorker* dbWorker)
        : mDb(dbWorker)
    {}

    void SequenceRepository::initDatabase()
    {
        if (!mDb) return;
        mDb->enqueue([](SQLite::Database& db) {
            db.exec(data::query::createSequenceTable);
            LOG_INFO("Initialized FIX_Sequences table in database.");
        });
    }

    std::tuple<uint32_t, uint32_t>
    SequenceRepository::getSequenceNumbers(const std::string& compId)
    {
        if (!mDb) return zeroValues;

        auto promise = std::make_shared<
                std::promise<std::tuple<uint32_t, uint32_t>>>();
        auto future = promise->get_future();

        mDb->enqueue([this, compId, promise](SQLite::Database& db) {
            try {
                SQLite::Statement query(db,
                                        data::query::getSequenceNumberQuery);
                query.bind(1, compId);

                if (query.executeStep()) {
                    uint32_t inSeqNum = query.getColumn(0).getInt();
                    uint32_t outSeqNum = query.getColumn(1).getInt();
                    promise->set_value({inSeqNum, outSeqNum});
                }
                else {
                    promise->set_value(defaultValues);
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("Failed to query sequences for {}: {}", compId,
                          e.what());
                promise->set_value(defaultValues);
            }
        });

        return future.get();
    }

    void SequenceRepository::updateSequenceNumbers(const std::string& compId,
                                                   uint32_t inSeqNum,
                                                   uint32_t outSeqNum)
    {
        if (!mDb) return;

        mDb->enqueue([compId, inSeqNum, outSeqNum](SQLite::Database& db) {
            try {
                SQLite::Statement insert(
                        db, data::query::updateSequenceNumberQuery);

                insert.bind(1, compId);
                insert.bind(2, inSeqNum);
                insert.bind(3, outSeqNum);

                insert.exec();
            }
            catch (const std::exception& e) {
                LOG_ERROR("Failed to update sequences for {}: {}", compId,
                          e.what());
            }
        });
    }

} // namespace data
