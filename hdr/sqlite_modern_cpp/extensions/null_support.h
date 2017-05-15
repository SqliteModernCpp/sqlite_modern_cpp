#pragma once

using namespace sqlite;
using namespace std;

namespace sqlite {
	// custom null value
	struct null_value {
		bool value = true;
		void operator=(bool val) { value = val; }
		operator bool() const { return value; }
	};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
	template<> inline database_binder::chain_type& operator <<(database_binder::chain_type& db, const sqlite::null_value& val) {
		int hresult;
		if((hresult = sqlite3_bind_null(db->_stmt.get(), db->_inx)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult);
		}
		++db->_inx;
		return db;
	}
#pragma GCC diagnostic pop

	template<> inline void get_col_from_db(database_binder& db, int inx, sqlite::null_value& d) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			d = true;
		} else {
			d = false;
		}
	}
}