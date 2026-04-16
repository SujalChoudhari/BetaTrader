/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "BetaTrader", "index.html", [
    [ "BetaTrader | High-Performance FX Trading Engine", "index.html", "index" ],
    [ "Exchange | App Orchestrator & Daemon", "d0/d4d/md_exchange_2exchange__app_2README.html", [
      [ "Overview", "d0/d4d/md_exchange_2exchange__app_2README.html#autotoc_md18", null ],
      [ "Key Responsibilities", "d0/d4d/md_exchange_2exchange__app_2README.html#autotoc_md19", null ],
      [ "Architecture", "d0/d4d/md_exchange_2exchange__app_2README.html#autotoc_md20", null ],
      [ "Class Diagram", "d0/d4d/md_exchange_2exchange__app_2README.html#autotoc_md21", null ],
      [ "Component Responsibilities", "d0/d4d/md_exchange_2exchange__app_2README.html#autotoc_md22", null ],
      [ "Critical Design Conventions", "d0/d4d/md_exchange_2exchange__app_2README.html#autotoc_md23", null ]
    ] ],
    [ "Core | FIX Protocol Tag Reference", "de/de2/md_exchange_2exchange__fix_2FIX.html", [
      [ "I. Standard Header Fields (Mandatory Structure)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md25", null ],
      [ "2. FIX Session Control Messages (MsgType)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md26", [
        [ "Sequence Recovery and Idempotency Fields", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md27", null ]
      ] ],
      [ "III. Core Application Messages (Trade Flow)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md28", [
        [ "1. NewOrderSingle (MsgType=D)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md29", null ],
        [ "2. ExecutionReport (MsgType=8)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md30", null ],
        [ "3. BusinessMessageReject (MsgType=j)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md31", null ]
      ] ],
      [ "4. Market Data and Order Book Management", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md32", [
        [ "1. MarketDataRequest (MsgType=V) - Requesting the Order Book", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md33", null ],
        [ "2. MarketDataSnapshotFullRefresh (MsgType=W)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md34", null ],
        [ "3. MarketDataIncrementalRefresh (MsgType=X)", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md35", null ],
        [ "4. Market Data Entry Repeating Group (Tags within NoMDEntries(268))", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md36", null ]
      ] ],
      [ "5. Generic Repeating Group Constraints", "de/de2/md_exchange_2exchange__fix_2FIX.html#autotoc_md37", null ]
    ] ],
    [ "Exchange | FIX Gateway & Session Manager", "d4/d86/md_exchange_2exchange__fix_2README.html", [
      [ "Overview", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md39", null ],
      [ "Key Responsibilities", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md40", null ],
      [ "Architecture", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md41", null ],
      [ "Key Components", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md42", null ],
      [ "Session Lifecycle", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md43", null ],
      [ "Order Lifecycle", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md44", null ],
      [ "Building and Running", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md45", null ],
      [ "Future Enhancements and TODOs", "d4/d86/md_exchange_2exchange__fix_2README.html#autotoc_md46", null ]
    ] ],
    [ "Exchange | Matching Engine", "d7/d3b/md_exchange_2exchange__matching_2README.html", [
      [ "Overview", "d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md48", null ],
      [ "Key Responsibilities", "d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md49", null ],
      [ "Architecture", "d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md50", null ],
      [ "Class Diagram", "d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md51", null ],
      [ "Component Responsibilities", "d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md52", null ],
      [ "Critical Design Conventions", "d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md53", null ]
    ] ],
    [ "Exchange | Persistence Layer", "d5/d20/md_exchange_2exchange__persistence_2README.html", [
      [ "Overview", "d5/d20/md_exchange_2exchange__persistence_2README.html#autotoc_md55", null ],
      [ "Key Responsibilities", "d5/d20/md_exchange_2exchange__persistence_2README.html#autotoc_md56", null ],
      [ "Architecture", "d5/d20/md_exchange_2exchange__persistence_2README.html#autotoc_md57", null ],
      [ "Class Diagram", "d5/d20/md_exchange_2exchange__persistence_2README.html#autotoc_md58", null ],
      [ "Component Responsibilities", "d5/d20/md_exchange_2exchange__persistence_2README.html#autotoc_md59", null ],
      [ "Critical Design Conventions", "d5/d20/md_exchange_2exchange__persistence_2README.html#autotoc_md60", null ]
    ] ],
    [ "Exchange | Event Publishing", "d9/d3f/md_exchange_2exchange__publishers_2README.html", [
      [ "Overview", "d9/d3f/md_exchange_2exchange__publishers_2README.html#autotoc_md62", null ],
      [ "Key Responsibilities", "d9/d3f/md_exchange_2exchange__publishers_2README.html#autotoc_md63", null ],
      [ "Architecture", "d9/d3f/md_exchange_2exchange__publishers_2README.html#autotoc_md64", null ],
      [ "Class Diagram", "d9/d3f/md_exchange_2exchange__publishers_2README.html#autotoc_md65", null ],
      [ "Component Responsibilities", "d9/d3f/md_exchange_2exchange__publishers_2README.html#autotoc_md66", null ],
      [ "Critical Design Conventions", "d9/d3f/md_exchange_2exchange__publishers_2README.html#autotoc_md67", null ]
    ] ],
    [ "Exchange | Risk Management", "df/da5/md_exchange_2exchange__risk_2README.html", [
      [ "Overview", "df/da5/md_exchange_2exchange__risk_2README.html#autotoc_md69", null ],
      [ "Key Responsibilities", "df/da5/md_exchange_2exchange__risk_2README.html#autotoc_md70", null ],
      [ "Architecture", "df/da5/md_exchange_2exchange__risk_2README.html#autotoc_md71", null ],
      [ "Class Diagram", "df/da5/md_exchange_2exchange__risk_2README.html#autotoc_md72", null ],
      [ "Component Responsibilities", "df/da5/md_exchange_2exchange__risk_2README.html#autotoc_md73", null ],
      [ "Critical Design Conventions", "df/da5/md_exchange_2exchange__risk_2README.html#autotoc_md74", null ]
    ] ],
    [ "Exchange | Command Routing & Partitioning", "d0/dd7/md_exchange_2exchange__routing_2README.html", [
      [ "Overview", "d0/dd7/md_exchange_2exchange__routing_2README.html#autotoc_md76", null ],
      [ "Key Responsibilities", "d0/dd7/md_exchange_2exchange__routing_2README.html#autotoc_md77", null ],
      [ "Architecture", "d0/dd7/md_exchange_2exchange__routing_2README.html#autotoc_md78", null ],
      [ "Class Diagram", "d0/dd7/md_exchange_2exchange__routing_2README.html#autotoc_md79", null ],
      [ "Component Responsibilities", "d0/dd7/md_exchange_2exchange__routing_2README.html#autotoc_md80", null ],
      [ "Critical Design Conventions", "d0/dd7/md_exchange_2exchange__routing_2README.html#autotoc_md81", null ]
    ] ],
    [ "Exchange | State Management", "da/d27/md_exchange_2exchange__state_2README.html", [
      [ "Overview", "da/d27/md_exchange_2exchange__state_2README.html#autotoc_md83", null ],
      [ "Key Responsibilities", "da/d27/md_exchange_2exchange__state_2README.html#autotoc_md84", null ],
      [ "Architecture", "da/d27/md_exchange_2exchange__state_2README.html#autotoc_md85", null ],
      [ "Class Diagram", "da/d27/md_exchange_2exchange__state_2README.html#autotoc_md86", null ],
      [ "Component Responsibilities", "da/d27/md_exchange_2exchange__state_2README.html#autotoc_md87", null ],
      [ "Critical Design Conventions", "da/d27/md_exchange_2exchange__state_2README.html#autotoc_md88", null ]
    ] ],
    [ "Exchange | Modular Trading Infrastructure", "d9/d64/md_exchange_2README.html", [
      [ "Architecture & System Flow", "d9/d64/md_exchange_2README.html#autotoc_md90", null ],
      [ "The Micro-Modules", "d9/d64/md_exchange_2README.html#autotoc_md91", null ],
      [ "Design Philosophy", "d9/d64/md_exchange_2README.html#autotoc_md92", null ],
      [ "Building and Testing", "d9/d64/md_exchange_2README.html#autotoc_md93", null ]
    ] ],
    [ "Client | Admin — Embedded Exchange Controller", "da/dd9/md_client_2client__admin_2README.html", [
      [ "Architecture", "da/dd9/md_client_2client__admin_2README.html#autotoc_md95", null ],
      [ "Key Components", "da/dd9/md_client_2client__admin_2README.html#autotoc_md96", [
        [ "1. <tt>ExchangeManager</tt>", "da/dd9/md_client_2client__admin_2README.html#autotoc_md97", null ],
        [ "2. <tt>ExchangePanel</tt>", "da/dd9/md_client_2client__admin_2README.html#autotoc_md98", null ]
      ] ],
      [ "Dependencies", "da/dd9/md_client_2client__admin_2README.html#autotoc_md99", null ]
    ] ],
    [ "Client | App Orchestrator", "dd/daa/md_client_2client__app_2README.html", [
      [ "Overview", "dd/daa/md_client_2client__app_2README.html#autotoc_md101", null ],
      [ "Key Responsibilities", "dd/daa/md_client_2client__app_2README.html#autotoc_md102", null ],
      [ "Architecture", "dd/daa/md_client_2client__app_2README.html#autotoc_md103", null ],
      [ "Class Diagram", "dd/daa/md_client_2client__app_2README.html#autotoc_md104", null ],
      [ "Component Responsibilities", "dd/daa/md_client_2client__app_2README.html#autotoc_md105", null ],
      [ "Critical Design Conventions", "dd/daa/md_client_2client__app_2README.html#autotoc_md106", null ]
    ] ],
    [ "Client | Auth & Session Manager", "d9/dd9/md_client_2client__auth_2README.html", [
      [ "Overview", "d9/dd9/md_client_2client__auth_2README.html#autotoc_md108", null ],
      [ "Key Responsibilities", "d9/dd9/md_client_2client__auth_2README.html#autotoc_md109", null ],
      [ "Architecture", "d9/dd9/md_client_2client__auth_2README.html#autotoc_md110", null ],
      [ "Class Diagram", "d9/dd9/md_client_2client__auth_2README.html#autotoc_md111", null ],
      [ "Component Responsibilities", "d9/dd9/md_client_2client__auth_2README.html#autotoc_md112", null ],
      [ "Critical Design Conventions", "d9/dd9/md_client_2client__auth_2README.html#autotoc_md113", null ]
    ] ],
    [ "Client | Execution Blotter", "d2/df2/md_client_2client__blotter_2README.html", [
      [ "Overview", "d2/df2/md_client_2client__blotter_2README.html#autotoc_md115", null ],
      [ "Key Responsibilities", "d2/df2/md_client_2client__blotter_2README.html#autotoc_md116", null ],
      [ "Architecture", "d2/df2/md_client_2client__blotter_2README.html#autotoc_md117", null ],
      [ "Class Diagram", "d2/df2/md_client_2client__blotter_2README.html#autotoc_md118", null ],
      [ "Component Responsibilities", "d2/df2/md_client_2client__blotter_2README.html#autotoc_md119", null ],
      [ "Critical Design Conventions", "d2/df2/md_client_2client__blotter_2README.html#autotoc_md120", null ]
    ] ],
    [ "Client | FIX Protocol Engine", "d1/d9b/md_client_2client__fix_2README.html", [
      [ "Architecture", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md122", null ],
      [ "Key Components", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md123", [
        [ "1. <tt>FixClientSession</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md124", null ],
        [ "2. <tt>SeqNumStore</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md125", null ],
        [ "3. <tt>AuthManager</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md126", null ],
        [ "4. <tt>FixMessageParser</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md127", null ]
      ] ],
      [ "Usage and Extensions", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md128", null ]
    ] ],
    [ "Client | HTTP Gateway Wrapper", "dc/d7f/md_client_2client__http_2README.html", [
      [ "Overview", "dc/d7f/md_client_2client__http_2README.html#autotoc_md130", null ],
      [ "Key Responsibilities", "dc/d7f/md_client_2client__http_2README.html#autotoc_md131", null ],
      [ "Architecture", "dc/d7f/md_client_2client__http_2README.html#autotoc_md132", null ],
      [ "Class Diagram", "dc/d7f/md_client_2client__http_2README.html#autotoc_md133", null ],
      [ "Component Responsibilities", "dc/d7f/md_client_2client__http_2README.html#autotoc_md134", null ],
      [ "Critical Design Conventions", "dc/d7f/md_client_2client__http_2README.html#autotoc_md135", null ]
    ] ],
    [ "Client | OHLC Aggregator", "dd/df5/md_client_2client__ohlc_2README.html", [
      [ "Overview", "dd/df5/md_client_2client__ohlc_2README.html#autotoc_md137", null ],
      [ "Key Responsibilities", "dd/df5/md_client_2client__ohlc_2README.html#autotoc_md138", null ],
      [ "Architecture", "dd/df5/md_client_2client__ohlc_2README.html#autotoc_md139", null ],
      [ "Class Diagram", "dd/df5/md_client_2client__ohlc_2README.html#autotoc_md140", null ],
      [ "Component Responsibilities", "dd/df5/md_client_2client__ohlc_2README.html#autotoc_md141", null ],
      [ "Critical Design Conventions", "dd/df5/md_client_2client__ohlc_2README.html#autotoc_md142", null ]
    ] ],
    [ "Client | L2 Orderbook", "d1/de7/md_client_2client__orderbook_2README.html", [
      [ "Overview", "d1/de7/md_client_2client__orderbook_2README.html#autotoc_md144", null ],
      [ "Key Responsibilities", "d1/de7/md_client_2client__orderbook_2README.html#autotoc_md145", null ],
      [ "Architecture", "d1/de7/md_client_2client__orderbook_2README.html#autotoc_md146", null ],
      [ "Class Diagram", "d1/de7/md_client_2client__orderbook_2README.html#autotoc_md147", null ],
      [ "Component Responsibilities", "d1/de7/md_client_2client__orderbook_2README.html#autotoc_md148", null ],
      [ "Critical Design Conventions", "d1/de7/md_client_2client__orderbook_2README.html#autotoc_md149", null ]
    ] ],
    [ "Client | Portfolio & Risk", "d3/d6d/md_client_2client__portfolio_2README.html", [
      [ "Overview", "d3/d6d/md_client_2client__portfolio_2README.html#autotoc_md151", null ],
      [ "Key Responsibilities", "d3/d6d/md_client_2client__portfolio_2README.html#autotoc_md152", null ],
      [ "Architecture", "d3/d6d/md_client_2client__portfolio_2README.html#autotoc_md153", null ],
      [ "Class Diagram", "d3/d6d/md_client_2client__portfolio_2README.html#autotoc_md154", null ],
      [ "Component Responsibilities", "d3/d6d/md_client_2client__portfolio_2README.html#autotoc_md155", null ],
      [ "Critical Design Conventions", "d3/d6d/md_client_2client__portfolio_2README.html#autotoc_md156", null ]
    ] ],
    [ "Client | HFT Simulator", "d4/daa/md_client_2client__simulator_2README.html", [
      [ "Overview", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md158", null ],
      [ "Key Responsibilities", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md159", null ],
      [ "Architecture", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md160", null ],
      [ "Class Diagram", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md161", null ],
      [ "Component Responsibilities", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md162", null ],
      [ "Critical Design Conventions", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md163", null ]
    ] ],
    [ "Client | ImGui Trader Terminal", "da/d44/md_client_2client__ui_2README.html", [
      [ "Architecture", "da/d44/md_client_2client__ui_2README.html#autotoc_md165", null ],
      [ "Core Components", "da/d44/md_client_2client__ui_2README.html#autotoc_md166", [
        [ "1. <tt>UIManager</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md167", null ],
        [ "2. <tt>Theme</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md168", null ],
        [ "3. <tt>ConnectionPanel</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md169", null ]
      ] ],
      [ "Planned Components", "da/d44/md_client_2client__ui_2README.html#autotoc_md170", [
        [ "<tt>OrderbookModel</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md171", null ],
        [ "<tt>MDSubscriptionManager</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md172", null ],
        [ "<tt>UIEventQueue</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md173", null ]
      ] ],
      [ "Setup & Dependencies", "da/d44/md_client_2client__ui_2README.html#autotoc_md174", null ]
    ] ],
    [ "Client | Unified Trading Application", "d1/db4/md_client_2README.html", [
      [ "Architecture & Data Flow", "d1/db4/md_client_2README.html#autotoc_md176", null ],
      [ "The Micro-Modules", "d1/db4/md_client_2README.html#autotoc_md177", null ],
      [ "Design Philosophy", "d1/db4/md_client_2README.html#autotoc_md178", null ],
      [ "Building the Client", "d1/db4/md_client_2README.html#autotoc_md179", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"d2/d74/classfix_1_1FixServer.html#a8cd7e3c691a9966ff115db389ceaf2d1",
"d3/d7f/classtrading__core_1_1WorkerThread.html#a53a245b261d8859df49766ca47c41c6e",
"d4/df8/classdata_1_1SequenceRepository.html#a73ccae8778b87433a87061afa926c582",
"d5/da8/structorderbook_1_1OrderBook_1_1Snapshot.html#afbfeb12eb15c0fe4be1fbc7ff77332ad",
"d7/d3b/md_exchange_2exchange__matching_2README.html#autotoc_md51",
"d9/d00/structfix_1_1SessionState.html#a77d59cc8067664eb7ccf7d5277b09474",
"da/d27/md_exchange_2exchange__state_2README.html#autotoc_md86",
"dc/d18/namespacefix.html#a5d893037d9f3db7091b9b24bb1974224",
"dd/dc1/structfix_1_1MarketDataSnapshotFullRefresh.html#a2234b71bb3c460b091f990bf66b10574",
"functions_func_h.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';