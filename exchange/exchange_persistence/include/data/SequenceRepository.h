/**
 * @brief Repository for managing sequence numbers.
 *
 * This class is responsible for managing the sequence numbers for FIX sessions.
 * It provides methods to get and update the sequence numbers.
 */

#pragma once

#include "data/DatabaseWorker.h"
#include <cstdint>
#include <string>
#include <tuple>

namespace data {

    /**
     * @brief Repository for managing sequence numbers.
     *
     * This class is responsible for managing the sequence numbers for FIX
     * sessions. It provides methods to get and update the sequence numbers.
     */
    class SequenceRepository {
    public:
        explicit SequenceRepository(DatabaseWorker* dbWorker);
        virtual ~SequenceRepository() = default;

        virtual void initDatabase();

        virtual std::tuple<uint32_t, uint32_t>
        getSequenceNumbers(const std::string& compId);

        virtual void updateSequenceNumbers(const std::string& compId,
                                           uint32_t inSeqNum,
                                           uint32_t outSeqNum);

        const std::tuple<uint32_t, uint32_t> defaultValues = {0, 1};
        const std::tuple<uint32_t, uint32_t> zeroValues = {0, 0};
            
    private:
        DatabaseWorker* mDb;
    };

} // namespace data
