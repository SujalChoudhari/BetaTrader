#pragma once

namespace fix {
    enum class Tag {
        /// \brief Begin String (FIX.x.y)
        BeginString = 8,
        /// \brief Message Body Length
        BodyLength = 9,
        /// \brief Message Type
        MsgType = 35,
        /// \brief Sender's Company ID
        SenderCompID = 49,
        /// \brief Target's Company ID
        TargetCompID = 56,
        /// \brief Message Sequence Number
        MsgSeqNum = 34,
        /// \brief Time of message transmission
        SendingTime = 52,

        /// \brief Client Order ID
        ClOrdID = 11,
        /// \brief Original Client Order ID
        OrigClOrdID = 41,
        /// \brief Security Symbol
        Symbol = 55,
        /// \brief Side of order (Buy/Sell)
        Side = 54,
        /// \brief Order Type (Market, Limit, etc.)
        OrdType = 40,
        /// \brief Quantity ordered
        OrderQty = 38,
        /// \brief Price of order
        Price = 44,
        /// \brief Time in Force
        TimeInForce = 59,
        /// \brief Stop Price
        StopPrice = 99,
        /// \brief Settlement Type
        SettlType = 63,
        /// \brief Settlement Date
        SettlDate = 64,
        /// \brief Order ID
        OrderID = 37,
        /// \brief Execution ID
        ExecID = 17,
        /// \brief Order Status
        OrdStatus = 39,
        /// \brief Free format text
        Text = 58,
        /// \brief Cumulative Quantity
        CumQty = 14,
        /// \brief Quantity remaining
        LeavesQty = 151,
        /// \brief Last Traded Price
        LastPx = 31,
        /// \brief Last Traded Quantity
        LastQty = 32,
        /// \brief Transaction Time
        TransactTime = 60,

        // Market Data Tags
        /// \brief Market Data Request ID
        MDReqID = 262,
        /// \brief Subscription Request Type
        SubscriptionRequestType = 263,
        /// \brief Market Depth
        MarketDepth = 264,
        /// \brief Number of Related Symbols
        NoRelatedSym = 146,
        /// \brief Market Data Update Action
        MDUpdateAction = 279,
        /// \brief Market Data Entry Type
        MDEntryType = 269,
        /// \brief Market Data Entry Price
        MDEntryPx = 270,
        /// \brief Market Data Entry Size
        MDEntrySize = 271,
        /// \brief Market Data Entry Time
        MDEntryTime = 273,
        /// \brief Number of Market Data Entries
        NoMDEntries = 268,
        /// \brief Market Data Entry Position Number
        MDEntryPositionNo = 290,

        /// \brief Checksum
        CheckSum = 10,
    };
}
