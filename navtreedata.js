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
    [ "BetaTrader documentation", "index.html", "index" ],
    [ "Exchange | App Orchestrator & Daemon", "md_exchange_2exchange__app_2README.html", [
      [ "Overview", "md_exchange_2exchange__app_2README.html#autotoc_md7", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__app_2README.html#autotoc_md8", null ],
      [ "Architecture", "md_exchange_2exchange__app_2README.html#autotoc_md9", null ],
      [ "Class Diagram", "md_exchange_2exchange__app_2README.html#autotoc_md10", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__app_2README.html#autotoc_md11", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__app_2README.html#autotoc_md12", null ]
    ] ],
    [ "Core | FIX Protocol Tag Reference", "md_exchange_2exchange__fix_2FIX.html", [
      [ "I. Standard Header Fields (Mandatory Structure)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md14", null ],
      [ "2. FIX Session Control Messages (MsgType)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md15", [
        [ "Sequence Recovery and Idempotency Fields", "md_exchange_2exchange__fix_2FIX.html#autotoc_md16", null ]
      ] ],
      [ "III. Core Application Messages (Trade Flow)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md17", [
        [ "1. NewOrderSingle (MsgType=D)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md18", null ],
        [ "2. ExecutionReport (MsgType=8)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md19", null ],
        [ "3. BusinessMessageReject (MsgType=j)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md20", null ]
      ] ],
      [ "4. Market Data and Order Book Management", "md_exchange_2exchange__fix_2FIX.html#autotoc_md21", [
        [ "1. MarketDataRequest (MsgType=V) - Requesting the Order Book", "md_exchange_2exchange__fix_2FIX.html#autotoc_md22", null ],
        [ "2. MarketDataSnapshotFullRefresh (MsgType=W)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md23", null ],
        [ "3. MarketDataIncrementalRefresh (MsgType=X)", "md_exchange_2exchange__fix_2FIX.html#autotoc_md24", null ],
        [ "4. Market Data Entry Repeating Group (Tags within NoMDEntries(268))", "md_exchange_2exchange__fix_2FIX.html#autotoc_md25", null ]
      ] ],
      [ "5. Generic Repeating Group Constraints", "md_exchange_2exchange__fix_2FIX.html#autotoc_md26", null ]
    ] ],
    [ "Exchange | FIX Gateway & Session Manager", "md_exchange_2exchange__fix_2README.html", [
      [ "Overview", "md_exchange_2exchange__fix_2README.html#autotoc_md28", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__fix_2README.html#autotoc_md29", null ],
      [ "Architecture", "md_exchange_2exchange__fix_2README.html#autotoc_md30", null ],
      [ "Key Components", "md_exchange_2exchange__fix_2README.html#autotoc_md31", null ],
      [ "Session Lifecycle", "md_exchange_2exchange__fix_2README.html#autotoc_md32", null ],
      [ "Order Lifecycle", "md_exchange_2exchange__fix_2README.html#autotoc_md33", null ],
      [ "Building and Running", "md_exchange_2exchange__fix_2README.html#autotoc_md34", null ],
      [ "Future Enhancements and TODOs", "md_exchange_2exchange__fix_2README.html#autotoc_md35", null ]
    ] ],
    [ "Exchange | Matching Engine", "md_exchange_2exchange__matching_2README.html", [
      [ "Overview", "md_exchange_2exchange__matching_2README.html#autotoc_md37", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__matching_2README.html#autotoc_md38", null ],
      [ "Architecture", "md_exchange_2exchange__matching_2README.html#autotoc_md39", null ],
      [ "Class Diagram", "md_exchange_2exchange__matching_2README.html#autotoc_md40", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__matching_2README.html#autotoc_md41", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__matching_2README.html#autotoc_md42", null ]
    ] ],
    [ "Exchange | Persistence Layer", "md_exchange_2exchange__persistence_2README.html", [
      [ "Overview", "md_exchange_2exchange__persistence_2README.html#autotoc_md44", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__persistence_2README.html#autotoc_md45", null ],
      [ "Architecture", "md_exchange_2exchange__persistence_2README.html#autotoc_md46", null ],
      [ "Class Diagram", "md_exchange_2exchange__persistence_2README.html#autotoc_md47", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__persistence_2README.html#autotoc_md48", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__persistence_2README.html#autotoc_md49", null ]
    ] ],
    [ "Exchange | Event Publishing", "md_exchange_2exchange__publishers_2README.html", [
      [ "Overview", "md_exchange_2exchange__publishers_2README.html#autotoc_md51", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__publishers_2README.html#autotoc_md52", null ],
      [ "Architecture", "md_exchange_2exchange__publishers_2README.html#autotoc_md53", null ],
      [ "Class Diagram", "md_exchange_2exchange__publishers_2README.html#autotoc_md54", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__publishers_2README.html#autotoc_md55", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__publishers_2README.html#autotoc_md56", null ]
    ] ],
    [ "Exchange | Risk Management", "md_exchange_2exchange__risk_2README.html", [
      [ "Overview", "md_exchange_2exchange__risk_2README.html#autotoc_md58", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__risk_2README.html#autotoc_md59", null ],
      [ "Architecture", "md_exchange_2exchange__risk_2README.html#autotoc_md60", null ],
      [ "Class Diagram", "md_exchange_2exchange__risk_2README.html#autotoc_md61", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__risk_2README.html#autotoc_md62", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__risk_2README.html#autotoc_md63", null ]
    ] ],
    [ "Exchange | Command Routing & Partitioning", "md_exchange_2exchange__routing_2README.html", [
      [ "Overview", "md_exchange_2exchange__routing_2README.html#autotoc_md65", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__routing_2README.html#autotoc_md66", null ],
      [ "Architecture", "md_exchange_2exchange__routing_2README.html#autotoc_md67", null ],
      [ "Class Diagram", "md_exchange_2exchange__routing_2README.html#autotoc_md68", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__routing_2README.html#autotoc_md69", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__routing_2README.html#autotoc_md70", null ]
    ] ],
    [ "Exchange | State Management", "md_exchange_2exchange__state_2README.html", [
      [ "Overview", "md_exchange_2exchange__state_2README.html#autotoc_md72", null ],
      [ "Key Responsibilities", "md_exchange_2exchange__state_2README.html#autotoc_md73", null ],
      [ "Architecture", "md_exchange_2exchange__state_2README.html#autotoc_md74", null ],
      [ "Class Diagram", "md_exchange_2exchange__state_2README.html#autotoc_md75", null ],
      [ "Component Responsibilities", "md_exchange_2exchange__state_2README.html#autotoc_md76", null ],
      [ "Critical Design Conventions", "md_exchange_2exchange__state_2README.html#autotoc_md77", null ]
    ] ],
    [ "Exchange | Modular Trading Infrastructure", "md_exchange_2README.html", [
      [ "Architecture & System Flow", "md_exchange_2README.html#autotoc_md79", null ],
      [ "The Micro-Modules", "md_exchange_2README.html#autotoc_md80", null ],
      [ "Design Philosophy", "md_exchange_2README.html#autotoc_md81", null ],
      [ "Building and Testing", "md_exchange_2README.html#autotoc_md82", null ]
    ] ],
    [ "Client | Admin — Embedded Exchange Controller", "md_client_2client__admin_2README.html", [
      [ "Architecture", "md_client_2client__admin_2README.html#autotoc_md84", null ],
      [ "Key Components", "md_client_2client__admin_2README.html#autotoc_md85", [
        [ "1. <tt>ExchangeManager</tt>", "md_client_2client__admin_2README.html#autotoc_md86", null ],
        [ "2. <tt>ExchangePanel</tt>", "md_client_2client__admin_2README.html#autotoc_md87", null ]
      ] ],
      [ "Dependencies", "md_client_2client__admin_2README.html#autotoc_md88", null ]
    ] ],
    [ "Client | App Orchestrator", "md_client_2client__app_2README.html", [
      [ "Overview", "md_client_2client__app_2README.html#autotoc_md90", null ],
      [ "Key Responsibilities", "md_client_2client__app_2README.html#autotoc_md91", null ],
      [ "Architecture", "md_client_2client__app_2README.html#autotoc_md92", null ],
      [ "Class Diagram", "md_client_2client__app_2README.html#autotoc_md93", null ],
      [ "Component Responsibilities", "md_client_2client__app_2README.html#autotoc_md94", null ],
      [ "Critical Design Conventions", "md_client_2client__app_2README.html#autotoc_md95", null ],
      [ "Verification Runbook", "md_client_2client__app_2README.html#autotoc_md96", null ]
    ] ],
    [ "Client | Auth & Session Manager", "md_client_2client__auth_2README.html", [
      [ "Overview", "md_client_2client__auth_2README.html#autotoc_md98", null ],
      [ "Key Responsibilities", "md_client_2client__auth_2README.html#autotoc_md99", null ],
      [ "Architecture", "md_client_2client__auth_2README.html#autotoc_md100", null ],
      [ "Class Diagram", "md_client_2client__auth_2README.html#autotoc_md101", null ],
      [ "Component Responsibilities", "md_client_2client__auth_2README.html#autotoc_md102", null ],
      [ "Critical Design Conventions", "md_client_2client__auth_2README.html#autotoc_md103", null ]
    ] ],
    [ "Client | Execution Blotter", "md_client_2client__blotter_2README.html", [
      [ "Overview", "md_client_2client__blotter_2README.html#autotoc_md105", null ],
      [ "Key Responsibilities", "md_client_2client__blotter_2README.html#autotoc_md106", null ],
      [ "Architecture", "md_client_2client__blotter_2README.html#autotoc_md107", null ],
      [ "Class Diagram", "md_client_2client__blotter_2README.html#autotoc_md108", null ],
      [ "Component Responsibilities", "md_client_2client__blotter_2README.html#autotoc_md109", null ],
      [ "Critical Design Conventions", "md_client_2client__blotter_2README.html#autotoc_md110", null ]
    ] ],
    [ "Client | FIX Protocol Engine", "md_client_2client__fix_2README.html", [
      [ "Architecture", "md_client_2client__fix_2README.html#autotoc_md112", null ],
      [ "Key Components", "md_client_2client__fix_2README.html#autotoc_md113", [
        [ "1. <tt>FixClientSession</tt>", "md_client_2client__fix_2README.html#autotoc_md114", null ],
        [ "2. <tt>SeqNumStore</tt>", "md_client_2client__fix_2README.html#autotoc_md115", null ],
        [ "3. <tt>AuthManager</tt>", "md_client_2client__fix_2README.html#autotoc_md116", null ],
        [ "4. <tt>FixMessageParser</tt>", "md_client_2client__fix_2README.html#autotoc_md117", null ]
      ] ],
      [ "Usage and Extensions", "md_client_2client__fix_2README.html#autotoc_md118", null ]
    ] ],
    [ "Client | HTTP Gateway Wrapper", "md_client_2client__http_2README.html", [
      [ "Overview", "md_client_2client__http_2README.html#autotoc_md120", null ],
      [ "Key Responsibilities", "md_client_2client__http_2README.html#autotoc_md121", null ],
      [ "Architecture", "md_client_2client__http_2README.html#autotoc_md122", null ],
      [ "Class Diagram", "md_client_2client__http_2README.html#autotoc_md123", null ],
      [ "Component Responsibilities", "md_client_2client__http_2README.html#autotoc_md124", null ],
      [ "Critical Design Conventions", "md_client_2client__http_2README.html#autotoc_md125", null ]
    ] ],
    [ "Client | OHLC Aggregator", "md_client_2client__ohlc_2README.html", [
      [ "Overview", "md_client_2client__ohlc_2README.html#autotoc_md127", null ],
      [ "Key Responsibilities", "md_client_2client__ohlc_2README.html#autotoc_md128", null ],
      [ "Architecture", "md_client_2client__ohlc_2README.html#autotoc_md129", null ],
      [ "Class Diagram", "md_client_2client__ohlc_2README.html#autotoc_md130", null ],
      [ "Component Responsibilities", "md_client_2client__ohlc_2README.html#autotoc_md131", null ],
      [ "Critical Design Conventions", "md_client_2client__ohlc_2README.html#autotoc_md132", null ]
    ] ],
    [ "Client | L2 Orderbook", "md_client_2client__orderbook_2README.html", [
      [ "Overview", "md_client_2client__orderbook_2README.html#autotoc_md134", null ],
      [ "Key Responsibilities", "md_client_2client__orderbook_2README.html#autotoc_md135", null ],
      [ "Architecture", "md_client_2client__orderbook_2README.html#autotoc_md136", null ],
      [ "Class Diagram", "md_client_2client__orderbook_2README.html#autotoc_md137", null ],
      [ "Component Responsibilities", "md_client_2client__orderbook_2README.html#autotoc_md138", null ],
      [ "Critical Design Conventions", "md_client_2client__orderbook_2README.html#autotoc_md139", null ]
    ] ],
    [ "Client | Portfolio & Risk", "md_client_2client__portfolio_2README.html", [
      [ "Overview", "md_client_2client__portfolio_2README.html#autotoc_md141", null ],
      [ "Key Responsibilities", "md_client_2client__portfolio_2README.html#autotoc_md142", null ],
      [ "Architecture", "md_client_2client__portfolio_2README.html#autotoc_md143", null ],
      [ "Class Diagram", "md_client_2client__portfolio_2README.html#autotoc_md144", null ],
      [ "Component Responsibilities", "md_client_2client__portfolio_2README.html#autotoc_md145", null ],
      [ "Critical Design Conventions", "md_client_2client__portfolio_2README.html#autotoc_md146", null ]
    ] ],
    [ "Client | HFT Simulator", "md_client_2client__simulator_2README.html", [
      [ "Overview", "md_client_2client__simulator_2README.html#autotoc_md148", null ],
      [ "Key Responsibilities", "md_client_2client__simulator_2README.html#autotoc_md149", null ],
      [ "Architecture", "md_client_2client__simulator_2README.html#autotoc_md150", null ],
      [ "Class Diagram", "md_client_2client__simulator_2README.html#autotoc_md151", null ],
      [ "Component Responsibilities", "md_client_2client__simulator_2README.html#autotoc_md152", null ],
      [ "Critical Design Conventions", "md_client_2client__simulator_2README.html#autotoc_md153", null ]
    ] ],
    [ "Client | ImGui Trader Terminal", "md_client_2client__ui_2README.html", [
      [ "Architecture", "md_client_2client__ui_2README.html#autotoc_md155", null ],
      [ "Core Components", "md_client_2client__ui_2README.html#autotoc_md156", [
        [ "1. <tt>UIManager</tt>", "md_client_2client__ui_2README.html#autotoc_md157", null ],
        [ "2. <tt>Theme</tt>", "md_client_2client__ui_2README.html#autotoc_md158", null ],
        [ "3. <tt>ConnectionPanel</tt>", "md_client_2client__ui_2README.html#autotoc_md159", null ]
      ] ],
      [ "Planned Components", "md_client_2client__ui_2README.html#autotoc_md160", [
        [ "<tt>OrderbookModel</tt>", "md_client_2client__ui_2README.html#autotoc_md161", null ],
        [ "<tt>MDSubscriptionManager</tt>", "md_client_2client__ui_2README.html#autotoc_md162", null ],
        [ "<tt>UIEventQueue</tt>", "md_client_2client__ui_2README.html#autotoc_md163", null ]
      ] ],
      [ "Setup & Dependencies", "md_client_2client__ui_2README.html#autotoc_md164", null ]
    ] ],
    [ "Client | Unified Trading Application", "md_client_2README.html", [
      [ "Architecture & Data Flow", "md_client_2README.html#autotoc_md166", null ],
      [ "The Micro-Modules", "md_client_2README.html#autotoc_md167", null ],
      [ "Design Philosophy", "md_client_2README.html#autotoc_md168", null ],
      [ "Building the Client", "md_client_2README.html#autotoc_md169", null ]
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
"App_8cpp.html",
"Protocol_8h.html#a5a2414d17e551f5e63b70c5dd22f88cf",
"classclient__app_1_1App.html#abfaf8ffb58df80b45706b60b6ea7bff3",
"classcommon_1_1Order.html#a743f41f3a7752dc477a7d2a113ef1d78",
"classfix_1_1ExecutionReport.html#a97b9964f9e8970ce4edc377a0cf1bb38",
"classfix__client_1_1FixClientSession.html#a5b832fbd593eaef29a638c7c5566892e",
"classtrading__core_1_1Command.html#acb6a54c0cf25f4944829b42d0ceb8f1a",
"classtrading__core_1_1Partition.html#afac510a5b5795124c88f820024e46154",
"dir_83861ae5f3f823e747dcdf71d319ec22.html",
"md_exchange_2exchange__fix_2README.html#autotoc_md31",
"namespacefix.html#ae4fe8876b55357d4aac35b7d406de7ffa9dffbf69ffba8bc38bc4e01abf4b1675",
"structfix_1_1MarketDataSnapshotFullRefresh.html#a827c68c00058d731789cecdac8915422"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';