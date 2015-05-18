#include <boost/optional.hpp>

namespace sqlite {
	// boost::optional
	template <typename BoostOptionalT> database_binder& operator <<(database_binder& db, const boost::optional<BoostOptionalT>& val) {
		if(val) {
			return operator << (*val);
		}

		if(sqlite3_bind_null(db._stmt, db._inx) != SQLITE_OK) {
			db.throw_sqlite_error();
		}

		++db._inx;
		return db;
	}
	template <typename BoostOptionalT> void get_col_from_db(database_binder& db, int inx, boost::optional<BoostOptionalT>& o) {
		if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
			o.reset();
		} else {
			BoostOptionalT v;
			get_col_from_db(inx, v);
			o = std::move(v);
		}
	}
}