#include <exchange_state/TradeIDGenerator.h>
#include <future>

namespace trading_core {

    TradeIDGenerator::TradeIDGenerator(data::TradeIDRepository* tradeIdRepo)
        : mCurrentId(0), mTradeIDRepo(tradeIdRepo)
    {
        loadInitialState();
    }

    TradeIDGenerator::~TradeIDGenerator()
    {
        saveState();
    }

    common::TradeID TradeIDGenerator::nextId()
    {
        return ++mCurrentId;
    }

    void TradeIDGenerator::loadInitialState()
    {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        mTradeIDRepo->getCurrentTradeID([this, promise](common::TradeID id) {
            mCurrentId = id;
            promise->set_value();
        });

        future.wait();
    }

    void TradeIDGenerator::saveState()
    {
        mTradeIDRepo->setCurrentTradeID(mCurrentId);
    }

} // namespace trading_core
