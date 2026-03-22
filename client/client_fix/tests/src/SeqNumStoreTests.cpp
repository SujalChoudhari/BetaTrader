#include <gtest/gtest.h>
#include "fix_client/SeqNumStore.h"
#include <filesystem>
#include <fstream>
#include "logging/Logger.h"

class SeqNumStoreTests : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        logging::Logger::Init("seq_store_test_logger", "logs/seq_store_test.log", true, false);
    }
    void SetUp() override {
        std::filesystem::remove_all("test_seq_store");
    }

    void TearDown() override {
        std::filesystem::remove_all("test_seq_store");
    }
};

TEST_F(SeqNumStoreTests, InitializationCreatesFile) {
    fix_client::SeqNumStore store("UNIT_TEST", "test_seq_store");
    EXPECT_TRUE(std::filesystem::exists("test_seq_store/UNIT_TEST.seq"));
    EXPECT_EQ(store.getNextTargetSeqNum(), 1);
    EXPECT_EQ(store.getNextSenderSeqNum(), 1);
}

TEST_F(SeqNumStoreTests, SequencePersistsAcrossInstances) {
    {
        fix_client::SeqNumStore store("UNIT_TEST", "test_seq_store");
        store.setSeqNums(100, 200);
    } // file is saved

    {
        // Re-open should load the saved state
        fix_client::SeqNumStore store("UNIT_TEST", "test_seq_store");
        EXPECT_EQ(store.getNextTargetSeqNum(), 100);
        EXPECT_EQ(store.getNextSenderSeqNum(), 200);
    }
}

TEST_F(SeqNumStoreTests, LargeSequenceNumbers) {
    uint32_t inSeq = 4294967295; // Max uint32
    uint32_t outSeq = 12345678;
    
    {
        fix_client::SeqNumStore store("LARGE_TEST", "test_seq_store");
        store.setSeqNums(inSeq, outSeq);
    }
    
    {
        fix_client::SeqNumStore store("LARGE_TEST", "test_seq_store");
        EXPECT_EQ(store.getNextTargetSeqNum(), inSeq);
        EXPECT_EQ(store.getNextSenderSeqNum(), outSeq);
    }
}

TEST_F(SeqNumStoreTests, MultipleStoresInSameDirectory) {
    fix_client::SeqNumStore store1("CLIENT_A", "test_seq_store");
    fix_client::SeqNumStore store2("CLIENT_B", "test_seq_store");
    
    store1.setSeqNums(10, 20);
    store2.setSeqNums(30, 40);
    
    EXPECT_EQ(store1.getNextTargetSeqNum(), 10);
    EXPECT_EQ(store2.getNextTargetSeqNum(), 30);
    
    EXPECT_TRUE(std::filesystem::exists("test_seq_store/CLIENT_A.seq"));
    EXPECT_TRUE(std::filesystem::exists("test_seq_store/CLIENT_B.seq"));
}

TEST_F(SeqNumStoreTests, ResetClearsSequence) {
    fix_client::SeqNumStore store("UNIT_TEST", "test_seq_store");
    store.setSeqNums(50, 60);
    store.reset();
    EXPECT_EQ(store.getNextTargetSeqNum(), 1);
    EXPECT_EQ(store.getNextSenderSeqNum(), 1);
}

TEST_F(SeqNumStoreTests, CorruptionRecoverable) {
    // Write garbage to the sequence file manually
    std::filesystem::create_directories("test_seq_store");
    std::ofstream ofs("test_seq_store/UNIT_TEST.seq");
    ofs << "GARBAGE DATA\n";
    ofs.close();

    // Store should gracefully recover and set to 1, 1
    fix_client::SeqNumStore store("UNIT_TEST", "test_seq_store");
    EXPECT_EQ(store.getNextTargetSeqNum(), 1);
    EXPECT_EQ(store.getNextSenderSeqNum(), 1);
}

