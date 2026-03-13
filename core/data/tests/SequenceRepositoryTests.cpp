#include "data/DatabaseWorker.h"
#include "data/SequenceRepository.h"
#include <gtest/gtest.h>

using namespace data;

class SequenceRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dbWorker = std::make_unique<DatabaseWorker>(":memory:");
        seqRepository = std::make_unique<SequenceRepository>(dbWorker.get());
        seqRepository->initDatabase();
    }

    std::unique_ptr<DatabaseWorker> dbWorker;
    std::unique_ptr<SequenceRepository> seqRepository;
};

TEST_F(SequenceRepositoryTest, InitDatabase)
{
    dbWorker->enqueue([](SQLite::Database& db) {
        ASSERT_TRUE(db.tableExists("FIX_Sequences"));
    });
    dbWorker->sync();
}

TEST_F(SequenceRepositoryTest, DefaultSequencesForNewClient)
{
    dbWorker->sync(); // Ensure init is done
    auto [inSeq, outSeq] = seqRepository->getSequenceNumbers("TEST_COMP_ID");
    
    // Default should be 0 for incoming, 1 for outgoing
    EXPECT_EQ(inSeq, 0);
    EXPECT_EQ(outSeq, 1);
}

TEST_F(SequenceRepositoryTest, UpdateAndRetrieveSequences)
{
    dbWorker->sync();
    
    // First update
    seqRepository->updateSequenceNumbers("CLIENT1", 10, 20);
    dbWorker->sync();
    
    auto [inSeq1, outSeq1] = seqRepository->getSequenceNumbers("CLIENT1");
    EXPECT_EQ(inSeq1, 10);
    EXPECT_EQ(outSeq1, 20);
    
    // Second update (on conflict replace)
    seqRepository->updateSequenceNumbers("CLIENT1", 11, 21);
    dbWorker->sync();
    
    auto [inSeq2, outSeq2] = seqRepository->getSequenceNumbers("CLIENT1");
    EXPECT_EQ(inSeq2, 11);
    EXPECT_EQ(outSeq2, 21);
    
    // Another client
    seqRepository->updateSequenceNumbers("CLIENT2", 5, 5);
    dbWorker->sync();
    
    auto [inSeq3, outSeq3] = seqRepository->getSequenceNumbers("CLIENT2");
    EXPECT_EQ(inSeq3, 5);
    EXPECT_EQ(outSeq3, 5);
    
    // Making sure the first client is undisturbed
    auto [inSeq4, outSeq4] = seqRepository->getSequenceNumbers("CLIENT1");
    EXPECT_EQ(inSeq4, 11);
    EXPECT_EQ(outSeq4, 21);
}
