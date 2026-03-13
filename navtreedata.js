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
    [ "Core | Data Persistence & SQL Engine", "d1/de3/md_core_2data_2README.html", [
      [ "Overview", "d1/de3/md_core_2data_2README.html#autotoc_md19", null ],
      [ "Architecture", "d1/de3/md_core_2data_2README.html#autotoc_md20", null ],
      [ "Class Diagram", "d1/de3/md_core_2data_2README.html#autotoc_md21", null ],
      [ "Component Responsibilities", "d1/de3/md_core_2data_2README.html#autotoc_md22", null ],
      [ "Lifecycle of a Save Request", "d1/de3/md_core_2data_2README.html#autotoc_md23", null ],
      [ "Database Schema", "d1/de3/md_core_2data_2README.html#autotoc_md24", [
        [ "<tt>trades</tt> Table", "d1/de3/md_core_2data_2README.html#autotoc_md25", null ],
        [ "<tt>orders</tt> Table", "d1/de3/md_core_2data_2README.html#autotoc_md26", null ],
        [ "<tt>FIX_Sequences</tt> Table", "d1/de3/md_core_2data_2README.html#autotoc_md27", null ],
        [ "<tt>database_audit_log</tt> Table", "d1/de3/md_core_2data_2README.html#autotoc_md28", null ]
      ] ]
    ] ],
    [ "Core | FIX Protocol Tag Reference", "d5/d7d/md_core_2fix_2FIX.html", [
      [ "I. Standard Header Fields (Mandatory Structure)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md30", null ],
      [ "2. FIX Session Control Messages (MsgType)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md31", [
        [ "Sequence Recovery and Idempotency Fields", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md32", null ]
      ] ],
      [ "III. Core Application Messages (Trade Flow)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md33", [
        [ "1. NewOrderSingle (MsgType=D)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md34", null ],
        [ "2. ExecutionReport (MsgType=8)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md35", null ],
        [ "3. BusinessMessageReject (MsgType=j)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md36", null ]
      ] ],
      [ "4. Market Data and Order Book Management", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md37", [
        [ "1. MarketDataRequest (MsgType=V) - Requesting the Order Book", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md38", null ],
        [ "2. MarketDataSnapshotFullRefresh (MsgType=W)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md39", null ],
        [ "3. MarketDataIncrementalRefresh (MsgType=X)", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md40", null ],
        [ "4. Market Data Entry Repeating Group (Tags within NoMDEntries(268))", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md41", null ]
      ] ],
      [ "5. Generic Repeating Group Constraints", "d5/d7d/md_core_2fix_2FIX.html#autotoc_md42", null ]
    ] ],
    [ "Core | FIX Gateway & Session Manager", "de/da8/md_core_2fix_2README.html", [
      [ "Overview", "de/da8/md_core_2fix_2README.html#autotoc_md44", null ],
      [ "Key Responsibilities", "de/da8/md_core_2fix_2README.html#autotoc_md45", null ],
      [ "Architecture", "de/da8/md_core_2fix_2README.html#autotoc_md46", null ],
      [ "Key Components", "de/da8/md_core_2fix_2README.html#autotoc_md47", null ],
      [ "Session Lifecycle", "de/da8/md_core_2fix_2README.html#autotoc_md48", null ],
      [ "Order Lifecycle", "de/da8/md_core_2fix_2README.html#autotoc_md49", null ],
      [ "Building and Running", "de/da8/md_core_2fix_2README.html#autotoc_md50", null ],
      [ "Future Enhancements and TODOs", "de/da8/md_core_2fix_2README.html#autotoc_md51", null ]
    ] ],
    [ "Core | Matching Engine & Order Management", "db/d57/md_core_2trading__core_2README.html", [
      [ "Overview", "db/d57/md_core_2trading__core_2README.html#autotoc_md53", null ],
      [ "Key Responsibilities", "db/d57/md_core_2trading__core_2README.html#autotoc_md54", null ],
      [ "Architecture", "db/d57/md_core_2trading__core_2README.html#autotoc_md55", null ],
      [ "Class Diagram", "db/d57/md_core_2trading__core_2README.html#autotoc_md56", null ],
      [ "Component Responsibilities", "db/d57/md_core_2trading__core_2README.html#autotoc_md57", null ],
      [ "Critical Design Conventions", "db/d57/md_core_2trading__core_2README.html#autotoc_md58", null ]
    ] ],
    [ "Client | FIX Protocol Engine", "d1/d9b/md_client_2client__fix_2README.html", [
      [ "Architecture", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md60", null ],
      [ "Key Components", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md61", [
        [ "1. <tt>FixClientSession</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md62", null ],
        [ "2. <tt>SeqNumStore</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md63", null ],
        [ "3. <tt>AuthManager</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md64", null ],
        [ "4. <tt>FixMessageParser</tt>", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md65", null ]
      ] ],
      [ "Design Decisions", "d1/d9b/md_client_2client__fix_2README.html#autotoc_md66", null ]
    ] ],
    [ "Client | High-Concurrency Stress Tester", "d4/daa/md_client_2client__simulator_2README.html", [
      [ "Architecture", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md68", null ],
      [ "Performance Components", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md69", [
        [ "1. <tt>RateController</tt>", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md70", null ],
        [ "2. <tt>MetricsCollector</tt>", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md71", null ],
        [ "3. Workload Profiles", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md72", null ]
      ] ],
      [ "Architecture", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md73", null ],
      [ "Running a Benchmark", "d4/daa/md_client_2client__simulator_2README.html#autotoc_md74", null ]
    ] ],
    [ "Client | ImGui Trader Terminal", "da/d44/md_client_2client__ui_2README.html", [
      [ "Architecture", "da/d44/md_client_2client__ui_2README.html#autotoc_md76", null ],
      [ "Core Architecture", "da/d44/md_client_2client__ui_2README.html#autotoc_md77", [
        [ "1. <tt>OrderbookModel</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md78", null ],
        [ "2. <tt>MDSubscriptionManager</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md79", null ],
        [ "3. <tt>UIEventQueue</tt>", "da/d44/md_client_2client__ui_2README.html#autotoc_md80", null ],
        [ "4. Panels & Visualization", "da/d44/md_client_2client__ui_2README.html#autotoc_md81", null ]
      ] ],
      [ "UI-FIX Connectivity", "da/d44/md_client_2client__ui_2README.html#autotoc_md82", null ],
      [ "Setup & Dependencies", "da/d44/md_client_2client__ui_2README.html#autotoc_md83", null ]
    ] ],
    [ "Client | Unified Trading Application", "d1/db4/md_client_2README.html", [
      [ "Overview", "d1/db4/md_client_2README.html#autotoc_md85", null ],
      [ "Key Responsibilities", "d1/db4/md_client_2README.html#autotoc_md86", null ],
      [ "Getting Started", "d1/db4/md_client_2README.html#autotoc_md87", [
        [ "Build", "d1/db4/md_client_2README.html#autotoc_md88", null ],
        [ "Run Trader Terminal", "d1/db4/md_client_2README.html#autotoc_md89", null ],
        [ "Run Load Simulator", "d1/db4/md_client_2README.html#autotoc_md90", null ]
      ] ],
      [ "Architecture", "d1/db4/md_client_2README.html#autotoc_md91", null ],
      [ "Interaction Flows", "d1/db4/md_client_2README.html#autotoc_md92", [
        [ "Secure Auth & Subscription", "d1/db4/md_client_2README.html#autotoc_md93", null ]
      ] ],
      [ "Design Conventions", "d1/db4/md_client_2README.html#autotoc_md94", null ],
      [ "Further Reading", "d1/db4/md_client_2README.html#autotoc_md95", null ]
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
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ]
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
"d2/dba/classtrading__core_1_1CancelOrder.html#a61d1800c294c652146c86bbfa9c52b34",
"d4/d13/namespaceerrors.html#a1911761ccabd6ca0c258fc9fe8a59afc",
"d5/d5c/classfix_1_1ExecutionReport.html#ae8f7df8b36418be13c27797c58d15965",
"d7/dd9/classtrading__core_1_1NewOrder.html#acb6a54c0cf25f4944829b42d0ceb8f1a",
"da/d44/md_client_2client__ui_2README.html#autotoc_md79",
"dc/dda/Protocol_8h.html#a5d893037d9f3db7091b9b24bb1974224",
"functions_r.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';