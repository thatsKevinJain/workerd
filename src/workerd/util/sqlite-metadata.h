// Copyright (c) 2024 Cloudflare, Inc.
// Licensed under the Apache 2.0 license found in the LICENSE file or at:
//     https://opensource.org/licenses/Apache-2.0

#pragma once

#include "sqlite.h"

namespace workerd {

// Class which implements a simple metadata kv storage and cache on top of SQLite.  Currently used
// to store:
//
// * Durable Object alarm times (hardcoded as key = 1).
//
// * A local development bookmark used to simulate the getCurrentBookmark API used by D1 (hardcoded
//   as key = 2).  The local development bookmark is not used in production.
//
// The table is named `_cf_METADATA`. The naming is designed so that if the application is allowed to
// perform direct SQL queries, we can block it from accessing any table prefixed with `_cf_`.
class SqliteMetadata final: private SqliteDatabase::ResetListener {
 public:
  explicit SqliteMetadata(SqliteDatabase& db);

  // Return currently set alarm time, or none.
  kj::Maybe<kj::Date> getAlarm();

  // Sets current alarm time, or none.
  void setAlarm(kj::Maybe<kj::Date> currentTime);

  // Return the current local development bookmark, or none if no bookmark has been set.
  kj::Maybe<uint64_t> getLocalDevelopmentBookmark();

  // Set the current ersatz bookmark.
  void setLocalDevelopmentBookmark(uint64_t);

 private:
  struct Uninitialized {};
  struct Initialized {
    SqliteDatabase& db;

    SqliteDatabase::Statement stmtGetAlarm = db.prepare(R"(
      SELECT value FROM _cf_METADATA WHERE key = 1
    )");
    SqliteDatabase::Statement stmtSetAlarm = db.prepare(R"(
      INSERT INTO _cf_METADATA VALUES(1, ?)
        ON CONFLICT DO UPDATE SET value = excluded.value;
    )");

    SqliteDatabase::Statement stmtGetLocalDevelopmentBookmark = db.prepare(R"(
      SELECT value FROM _cf_METADATA WHERE key = 2
    )");
    SqliteDatabase::Statement stmtSetLocalDevelopmentBookmark = db.prepare(R"(
      INSERT INTO _cf_METADATA VALUES(2, ?)
        ON CONFLICT DO UPDATE SET value = excluded.value;
    )");

    Initialized(SqliteDatabase& db): db(db) {}
  };

  bool tableCreated;
  kj::OneOf<Uninitialized, Initialized> dbState = Uninitialized{};

  struct Cache {
    kj::Maybe<kj::Date> alarmTime;
  };
  kj::Maybe<Cache> cacheState;

  kj::Maybe<kj::Date> getAlarmUncached();
  void setAlarmUncached(kj::Maybe<kj::Date> currentTime);

  Initialized& ensureInitialized();
  // Make sure the metadata table is created and prepared statements are ready. Not called until the
  // first write.

  // ResetListener interface:
  void beforeSqliteReset() override;
};

}  // namespace workerd
