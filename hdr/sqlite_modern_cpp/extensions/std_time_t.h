#include <ctime>

namespace sqlite {
	// std::time_t
	template<std::time_t> inline void get_col_from_db(database_binder& db, int inx, std::time_t& t) {
		if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
			t = 0;
		} else {
			t = sqlite3_column_int64(db._stmt, inx);
		}
	}
	template<std::time_t> inline database_binder&& operator <<(database_binder&& db, const std::time_t&& val) {
		//sqlite_int64 tmpval = static_cast<sqlite_int64>(val);
		auto ret = sqlite3_bind_int64(db._stmt, db._inx, val);
		if(ret != SQLITE_OK) db.throw_sqlite_error(ret);
		++db._inx;
		return std::move(db);
	}
}