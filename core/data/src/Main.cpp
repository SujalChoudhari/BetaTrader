//
// Created by sujal on 26-10-2025.
//


#include "Constants.h"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Statement.h"

void initialiseDB() {
    const SQLite::Database db(data::databasePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    SQLite::Statement query(db, "CREATE TABLE IF NOT EXISTS trade_id (current_id INTEGER NOT NULL) ");

    query.exec();
}

int main(int argc, char *argv[]) {
    initialiseDB();
}
