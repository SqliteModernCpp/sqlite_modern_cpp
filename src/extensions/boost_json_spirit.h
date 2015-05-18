#include <json_spirit.h>

namespace sqlite {
	// json_spirit::mArray
	template<> void get_col_from_db(database_binder& db, int inx, json_spirit::mArray& a) {
		
		json_spirit::mValue tmp;
		std::string str((char*)sqlite3_column_blob(db._stmt, inx), sqlite3_column_bytes(db._stmt, inx));
		json_spirit::read(str, tmp);
		
		a = tmp.get_array();
	}

	template<> database_binder& operator <<(database_binder& db, const json_spirit::mArray& val) {
		auto tmp = json_spirit::write(val);
		if(sqlite3_bind_blob(db._stmt, db._inx, tmp.c_str() , tmp.size() , SQLITE_TRANSIENT) != SQLITE_OK) {
			db.throw_sqlite_error();
		}

		++db._inx;
		return db;
	}
	
	// json_spirit::mObject
	template<> void get_col_from_db(database_binder& db, int inx, json_spirit::mObject& a) {

		json_spirit::mValue tmp;
		std::string str((char*)sqlite3_column_blob(db._stmt, inx), sqlite3_column_bytes(db._stmt, inx));
		json_spirit::read(str, tmp);

		a = tmp.get_obj();
	}

	template<> database_binder& operator <<(database_binder& db, const json_spirit::mObject& val) {
		auto tmp = json_spirit::write(val);
		if(sqlite3_bind_blob(db._stmt, db._inx, tmp.c_str(), tmp.size(), SQLITE_TRANSIENT) != SQLITE_OK) {
			db.throw_sqlite_error();
		}

		++db._inx;
		return db;
	}
}