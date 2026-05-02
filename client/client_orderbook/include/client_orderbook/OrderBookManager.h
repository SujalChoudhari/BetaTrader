#pragma once

#include "client_orderbook/OrderBookModel.h"
#include <map>
#include <memory>
#include <string>
#include <shared_mutex>
#include <mutex>
#include <vector>

namespace client_orderbook {

/**
 * @class OrderBookManager
 * @brief Manages multiple OrderBookModel instances for different symbols.
 */
class OrderBookManager {
public:
    /**
     * @brief Gets or creates an OrderBookModel for the given symbol.
     */
    std::shared_ptr<OrderBookModel> getModel(const std::string& symbol) {
        std::unique_lock lock(mMutex);
        auto it = mModels.find(symbol);
        if (it == mModels.end()) {
            auto model = std::make_shared<OrderBookModel>(symbol);
            mModels[symbol] = model;
            return model;
        }
        return it->second;
    }

    /**
     * @brief Returns all currently tracked models.
     */
    std::vector<std::shared_ptr<OrderBookModel>> getAllModels() const {
        std::shared_lock lock(mMutex);
        std::vector<std::shared_ptr<OrderBookModel>> result;
        for (auto const& [symbol, model] : mModels) {
            result.push_back(model);
        }
        return result;
    }

    /**
     * @brief Removes all models and their data.
     */
    void clearAll() {
        std::unique_lock lock(mMutex);
        mModels.clear();
    }

private:
    std::map<std::string, std::shared_ptr<OrderBookModel>> mModels;
    mutable std::shared_mutex mMutex;
};

} // namespace client_orderbook
