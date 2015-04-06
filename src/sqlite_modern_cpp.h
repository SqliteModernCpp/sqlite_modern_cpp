#pragma once

#include <string>
#include <functional>
#include <stdexcept>

#include "sqlite3.h"

#include "utility/sfinae.h"
#include "utility/function_traits.h"

namespace sqlite {

class database;
template<std::size_t> class binder;

class database_binder {
private:
	sqlite3* const _db;
	std::u16string _sql;
	sqlite3_stmt* _stmt;
	int _inx;

	void _extract(std::function<void(void)> call_back) {
		int hresult;

		while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			call_back();
		}

		if (hresult != SQLITE_DONE) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		if (sqlite3_finalize(_stmt) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		_stmt = nullptr;
	}
	void _extract_single_value(std::function<void(void)> call_back) {
		int hresult;

		if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			call_back();
		}

		if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			throw std::runtime_error("not every row extracted");
		}

		if (hresult != SQLITE_DONE) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		if (sqlite3_finalize(_stmt) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		_stmt = nullptr;
	}
	void _prepare() {
		if (sqlite3_prepare16_v2(_db, _sql.data(), -1, &_stmt, nullptr) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
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
		/* Will be executed if no >>op is found */
		if (_stmt) {
			int hresult;
			while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) { }

			if (hresult != SQLITE_DONE) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			if (sqlite3_finalize(_stmt) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			_stmt = nullptr;
		}
	}

	database_binder& operator <<(double val) {
		if (sqlite3_bind_double(_stmt, _inx, val) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		++_inx;
		return *this;
	}
	database_binder& operator <<(float val) {
		if (sqlite3_bind_double(_stmt, _inx, double(val)) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		++_inx;
		return *this;
	}
	database_binder& operator <<(int val) {
		if (sqlite3_bind_int(_stmt, _inx, val) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		++_inx;
		return *this;
	}
	database_binder& operator <<(sqlite_int64 val) {
		if (sqlite3_bind_int64(_stmt, _inx, val) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		++_inx;
		return *this;
	}
	database_binder& operator <<(std::string const& txt) {
		if (sqlite3_bind_text(_stmt, _inx, txt.data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		++_inx;
		return *this;
	}
	database_binder& operator <<(std::u16string const& txt) {
		if (sqlite3_bind_text16(_stmt, _inx, txt.data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
			throw std::runtime_error(sqlite3_errmsg(_db));
		}

		++_inx;
		return *this;
	}

	void get_col_from_db(int inx, int& i) {
		if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
			i = 0;
		} else {
			i = sqlite3_column_int(_stmt, inx);
		}
	}
	void get_col_from_db(int inx, sqlite3_int64& i) {
		if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
			i = 0;
		} else {
			i = sqlite3_column_int64(_stmt, inx);
		}
	}
	void get_col_from_db(int inx, std::string& s) {
		if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
			s = std::string();
		} else {
			sqlite3_column_bytes(_stmt, inx);
			s = std::string((char*)sqlite3_column_text(_stmt, inx));
		}
	}
	void get_col_from_db(int inx, std::u16string& w) {
		if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
			w = std::u16string();
		} else {
			sqlite3_column_bytes16(_stmt, inx);
			w = std::u16string((char16_t *)sqlite3_column_text16(_stmt, inx));
		}
	}
	void get_col_from_db(int inx, double& d) {
		if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
			d = 0;
		} else {
			d = sqlite3_column_double(_stmt, inx);
		}
	}
	void get_col_from_db(int inx, float& f) {
		if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
			f = 0;
		} else {
			f = float(sqlite3_column_double(_stmt, inx));
		}
	}

	template <
		typename Result,
		utility::enable_if<is_sqlite_value<Result>::value> = 0
	>
	void operator>>(Result& value) {
		this->_extract_single_value([&value, this]{
			this->get_col_from_db(0, value);
		});
	}

	template <
		typename Function,
		utility::disable_if<is_sqlite_value<Function>::value> = 0
	>
	void operator>>(Function func) {
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
	template<
		typename    Function,
		typename... Values,
		utility::enable_if<(sizeof...(Values) < Count)> = 0
	>
	static void run(
		database_binder& db,
		Function&        function,
		Values&&...      values
	) {
		nth_argument_type<Function, sizeof...(Values)> value{};
		db.get_col_from_db(sizeof...(Values), value);

		run<Function>(db, function, std::forward<Values>(values)..., std::move(value));
	}

	template<
		typename    Function,
		typename... Values,
		utility::enable_if<(sizeof...(Values) == Count)> = 0
	>
	static void run(
		database_binder&,
		Function&        function,
		Values&&...      values
	) {
		function(std::move(values)...);
	}
};

}
