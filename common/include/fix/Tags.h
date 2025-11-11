#pragma once

namespace fix {
    enum class Tag {
        // Header
        BeginString = 8,
        BodyLength = 9,
        MsgType = 35,
        SenderCompID = 49,
        TargetCompID = 56,
        MsgSeqNum = 34,
        SendingTime = 52,

        // Body
        ClOrdID = 11,
        Symbol = 55,
        Side = 54,
        OrdType = 40,
        OrderQty = 38,
        Price = 44,
        TimeInForce = 59,
        StopPrice = 99,
        SettlType = 63,
        SettlDate = 64,
        OrderID = 37,
        ExecID = 17,
        OrdStatus = 39,
        Text = 58,
        CumQty = 14,
        LeavesQty = 151,
        LastPx = 31,
        LastQty = 32,
        TransactTime = 60,

        // Trailer
        CheckSum = 10,
    };
}
