#pragma once

#include "logging/Runbook.h"

namespace errors {
    inline constexpr runbook::ErrorDefinition EDATA1{
        "EDATA1",
        "Error in Async DB worker",
        "Check the query executed, this might be an SQL or Worker error"
    };

    inline constexpr runbook::ErrorDefinition EDATA2{
        "EDATA2",
        "Failed to create the 'TradeID' database table.",
        "Check the underlying database connection and the SQL syntax for the trade ID table creation query."
    };

    inline constexpr runbook::ErrorDefinition EDATA3{
        "EDATA3",
        "Failed to retrieve the current TradeID from the database.",
        "Verify the database connection is active and the SQL query for retrieving the TradeID is correct. This may indicate data corruption."
    };

    inline constexpr runbook::ErrorDefinition EDATA4{
        "EDATA4",
        "Attempt to update the TradeID in the database failed.",
        "Confirm that the process has write permissions to the database file/server and the update query syntax is valid."
    };

    inline constexpr runbook::ErrorDefinition EDATA5{
        "EDATA5",
        "Failed to truncate or reset the TradeID table.",
        "Check database permissions for TRUNCATE/DELETE operations and verify the query execution environment."
    };

    inline constexpr runbook::ErrorDefinition EDATA6{
        "EDATA6",
        "Failed to create the 'Trade' database table.",
        "Check the underlying database connection and the SQL syntax for the trade table creation query."
    };

    inline constexpr runbook::ErrorDefinition EDATA7{
        "EDATA6",
        "Failed to add a trade in the 'Trade' database table.",
        "Check the underlying database connection and the SQL syntax for the trade table creation query."
    };
}
