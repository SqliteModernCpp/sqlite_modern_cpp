#pragma once

#include <string>
#include <functional>
#include <stdexcept>
#include <ctime>

#include "sqlite3.h"

#include "utility/function_traits.h"

namespace sqlite {


struct sqlite_exception: public std::runtime_error {
	sqlite_exception(const char* msg):runtime_error(msg) {}
};

class database;
class database_binder;

template<std::size_t> class binder;

template<typename T> database_binder& operator <<(database_binder& db,const T&& val);
template<typename T> database_binder& operator <<(database_binder& db,const T& val);
template<typename T> void get_col_from_db(database_binder& db, int inx, T& val);


class database_binder {
private:
	sqlite3* const _db;
	std::u16string _sql;
	sqlite3_stmt* _stmt;
	int _inx;

	bool throw_exceptions = true;
	bool error_occured = false;

	void _extract(std::function<void(void)> call_back) {
		int hresult;

		while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			call_back();
		}

		if (hresult != SQLITE_DONE) {
			throw_sqlite_error();
		}

		if (sqlite3_finalize(_stmt) != SQLITE_OK) {
			throw_sqlite_error();
		}

		_stmt = nullptr;
	}
	void _extract_single_value(std::function<void(void)> call_back) {
		int hresult;

		if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			call_back();
		}

		if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			throw_custom_error("not all rows extracted");
		}

		if (hresult != SQLITE_DONE) {
			throw_sqlite_error();
		}

		if (sqlite3_finalize(_stmt) != SQLITE_OK) {
			throw_sqlite_error();
		}

		_stmt = nullptr;
	}

	void _prepare() {
		if (sqlite3_prepare16_v2(_db, _sql.data(), -1, &_stmt, nullptr) != SQLITE_OK) {
			throw_sqlite_error();
		}
	}

	template <typename Type>
	using is_sqlite_value = std::integral_constant<
		bool,
		   std::is_floating_point<Type>::value
		|| std::is_integral<Type>::value
		|| std::is_same<std::string, Type>::value
		|| std::is_same<std::u16string, Type>::value
		|| std::is_same<sqlite_int64, Type>::value
	>;

	template<typename T> friend database_binder& operator <<(database_binder& ddb,const T&& val);
	template<typename T> friend database_binder& operator <<(database_binder& ddb,const T& val);
	template<typename T> friend void get_col_from_db(database_binder& ddb, int inx, T& val);

protected:
	database_binder(sqlite3* db, std::u16string const & sql):
		_db(db),
		_sql(sql),
		_stmt(nullptr),
		_inx(1) {
		_prepare();
	}

	database_binder(sqlite3* db, std::string const & sql):
		database_binder(db, std::u16string(sql.begin(), sql.end())) { }

public:
	friend class database;

	~database_binder() {
		throw_exceptions = false;
		/* Will be executed if no >>op is found */
		if (_stmt) {
			int hresult;

			while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) { }

			if (hresult != SQLITE_DONE) {
				throw_sqlite_error();
			}

			if (sqlite3_finalize(_stmt) != SQLITE_OK) {
				throw_sqlite_error();
			}

			_stmt = nullptr;
		}
	}

	void throw_sqlite_error() {
		if(throw_exceptions) {
			throw sqlite_exception(sqlite3_errmsg(_db));
		}
		error_occured = true;
	}

	void throw_custom_error(const char* str) {
		if(throw_exceptions) {
			throw std::runtime_error(str);
		}
		error_occured = true;
	}

	template <typename Result>
	typename std::enable_if<is_sqlite_value<Result>::value, void>::type operator>>(
		Result& value) {
		this->_extract_single_value([&value, this]{
			get_col_from_db(*this,0, value);
		});
	}

	template <typename Function>
	typename std::enable_if<!is_sqlite_value<Function>::value, void>::type operator>>(
		Function func) {
		typedef utility::function_traits<Function> traits;

		this->_extract([&func, this]() {
			binder<traits::arity>::run(*this, func);
		});
	}
};

class database {
private:
	sqlite3 * _db;
	bool _connected;
	bool _ownes_db;

public:
	database(std::u16string const & db_name):
		_db(nullptr),
		_connected(false),
		_ownes_db(true) {
		_connected = sqlite3_open16(db_name.data(), &_db) == SQLITE_OK;
	}

	database(std::string const & db_name):
		database(std::u16string(db_name.begin(), db_name.end())) { }

	database(sqlite3* db):
		_db(db),
		_connected(SQLITE_OK),
		_ownes_db(false) { }

	~database() {
		if (_db && _ownes_db) {
			sqlite3_close_v2(_db);
			_db = nullptr;
		}
	}

	database_binder operator<<(std::string const& sql) const {
		return database_binder(_db, sql);
	}
	database_binder operator<<(std::u16string const& sql) const {
		return database_binder(_db, sql);
	}

	operator bool() const {
		return _connected;
	}

	sqlite3_int64 last_insert_rowid() const {
		return sqlite3_last_insert_rowid(_db);
	}
};

template<std::size_t Count>
class binder {
private:
	template <
		typename    Function,
		std::size_t Index
	>
	using nth_argument_type = typename utility::function_traits<
		Function
	>::template argument<Index>;

public:
	// `Boundary` needs to be defaulted to `Count` so that the `run` function
	// template is not implicitly instantiated on class template instantiation.
	// Look up section 14.7.1 _Implicit instantiation_ of the ISO C++14 Standard
	// and the [dicussion](https://github.com/aminroosta/sqlite_modern_cpp/issues/8)
	// on Github.

	template<
		typename    Function,
		typename... Values,
		std::size_t Boundary = Count
	>
	static typename std::enable_if<(sizeof...(Values) < Boundary), void>::type run(
		database_binder& db,
		Function&        function,
		Values&&...      values
	) {
		nth_argument_type<Function, sizeof...(Values)> value{};
		get_col_from_db(db,sizeof...(Values), value);

		run<Function>(db, function, std::forward<Values>(values)..., std::move(value));
	}

	template<
		typename    Function,
		typename... Values,
		std::size_t Boundary = Count
	>
	static typename std::enable_if<(sizeof...(Values) == Boundary), void>::type run(
		database_binder&,
		Function&        function,
		Values&&...      values
	) {
		function(std::move(values)...);
	}
};

// int
template<> database_binder& operator <<(database_binder& db,const int&& val) {
	if(sqlite3_bind_int(db._stmt, db._inx, val) != SQLITE_OK) {
		db.throw_sqlite_error();
	}
	++db._inx;
	return db;
}
template<> void get_col_from_db(database_binder& db, int inx, int& val) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		val = 0;
	} else {
		val = sqlite3_column_int(db._stmt, inx);
	}
}

// sqlite_int64
template<> database_binder& operator <<(database_binder& db, const sqlite_int64&& val) {
	if(sqlite3_bind_int64(db._stmt, db._inx, val) != SQLITE_OK) {
		db.throw_sqlite_error();
	}

	++db._inx;
	return db;
}
template<> void get_col_from_db(database_binder& db,int inx, sqlite3_int64& i) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		i = 0;
	} else {
		i = sqlite3_column_int64(db._stmt, inx);
	}
}

// float
template<> database_binder& operator <<(database_binder& db,const float&& val) {
	if(sqlite3_bind_double(db._stmt, db._inx, double(val)) != SQLITE_OK) {
		db.throw_sqlite_error();
	}

	++db._inx;
	return db;
}
template<> void get_col_from_db(database_binder& db, int inx, float& f) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		f = 0;
	} else {
		f = float(sqlite3_column_double(db._stmt, inx));
	}
}

// double
template<> database_binder& operator <<(database_binder& db,const double&& val) {
	if(sqlite3_bind_double(db._stmt, db._inx, val) != SQLITE_OK) {
		db.throw_sqlite_error();
	}

	++db._inx;
	return db;
}
template<> void get_col_from_db(database_binder& db, int inx, double& d) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		d = 0;
	} else {
		d = sqlite3_column_double(db._stmt, inx);
	}
}

// std::string
template<> void get_col_from_db(database_binder& db, int inx,std::string & s) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		s = std::string();
	} else {
		sqlite3_column_bytes(db._stmt, inx);
		s = std::string((char*)sqlite3_column_text(db._stmt, inx));
	}
}
template<> database_binder& operator <<(database_binder& db, std::string const&& txt) {
	if(sqlite3_bind_text(db._stmt, db._inx, txt.data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
		db.throw_sqlite_error();
	}

	++db._inx;
	return db;
}
// std::u16string
template<> void get_col_from_db(database_binder& db, int inx, std::u16string & w) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		w = std::u16string();
	} else {
		sqlite3_column_bytes16(db._stmt, inx);
		w = std::u16string((char16_t *)sqlite3_column_text16(db._stmt, inx));
	}
}
template<> database_binder& operator <<(database_binder& db, std::u16string const&& txt) {
	if(sqlite3_bind_text16(db._stmt, db._inx, txt.data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
		db.throw_sqlite_error();
	}

	++db._inx;
	return db;
}

/* call the rvalue functions */
template<typename T> database_binder& operator <<(database_binder& db, const T& val) { return db << std::move(val); }

/*special case for string literals*/
template<std::size_t N> database_binder& operator <<(database_binder& db, const char(&STR)[N]) { return db << std::string(STR); }
template<std::size_t N> database_binder& operator <<(database_binder& db, const char16_t(&STR)[N]) { return db << std::u16string(STR); }
}
