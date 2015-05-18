#include <boost/uuid/uuid.hpp>

namespace sqlite {
	// boost::uuid
	template<> database_binder& operator <<(database_binder& db, const boost::uuids::uuid& uuid) {

		if(sqlite3_bind_blob(db._stmt, db._inx, uuid.begin(), uuid.size(), SQLITE_TRANSIENT) != SQLITE_OK) {
			db.throw_sqlite_error();
		}

		++db._inx;
		return db;
	}
	template <> void get_col_from_db(database_binder& db, int inx, boost::uuids::uuid& uuid) {
		if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
			uuid = boost::uuids::uuid();
		} else {
			const auto bytes = uuid.size();
			if(sqlite3_column_bytes(db._stmt, inx) != bytes) {
				db.throw_custom_error("UUID size does not match!");
			}
			memcpy(uuid.begin(), sqlite3_column_blob(db._stmt, inx), bytes);
		}
	}
}