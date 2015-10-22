#pragma once

#include <string>
#include <functional>
#include <stdexcept>
#include <ctime>
#include <tuple>

#include <sqlite3.h>

#include <sqlite_modern_cpp/utility/function_traits.h>

namespace sqlite {

struct sqlite_exception: public std::runtime_error {
	sqlite_exception(const char* msg):runtime_error(msg) {}
};

namespace exceptions
{
	//One more or less trivial derived error class for each SQLITE error.
	//Note the following are not errors so have no classes:
	//SQLITE_OK, SQLITE_NOTICE, SQLITE_WARNING, SQLITE_ROW, SQLITE_DONE
	//
	//Note these names are exact matches to the names of the SQLITE error codes.
	class error     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class internal  : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class perm      : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class abort     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class busy      : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class locked    : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class nomem     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class readonly  : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class interrupt : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class ioerr     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class corrupt   : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class notfound  : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class full      : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class cantopen  : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class protocol  : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class empty     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class schema    : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class toobig    : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class constraint: public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class mismatch  : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class misuse    : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class nolfs     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class auth      : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class format    : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class range     : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class notadb    : public sqlite_exception { using sqlite_exception::sqlite_exception;};

	//Some additional errors are here for the C++ interface
	class more_rows : public sqlite_exception { using sqlite_exception::sqlite_exception;};
	class no_rows   : public sqlite_exception { using sqlite_exception::sqlite_exception;};
}

class database;
class database_binder;

template<std::size_t> class binder;

template<typename T> database_binder&& operator <<(database_binder&& db,T const&& val);
template<typename T> void get_col_from_db(database_binder& db, int inx, T& val);

class sqlite3_statment
{
	private:
		sqlite3_stmt* _stmt;

	public:
		
		sqlite3_stmt** operator&()
		{
			return &_stmt;
		}

		operator sqlite3_stmt*()
		{
			return _stmt;
		}

		sqlite3_statment(sqlite3_stmt* s)
		:_stmt(s)
		{
		}

		~sqlite3_statment()
		{
			//Do not check for errors: an error code means that the 
			//*execution* of the statement failed somehow. We deal with errors
			//at that point so we don't need to know about errors here.
			//
			//Also, this is an RAII class to make sure we don't leak during exceptions
			//so there's a reasonably chance we're already in an exception here.
			
			sqlite3_finalize(_stmt);
		}
};

template<typename Tuple, int Element=0, bool Last=(std::tuple_size<Tuple>::value == Element)> struct tuple_iterate
{
	static void iterate(Tuple& t, database_binder& db)
	{
		get_col_from_db(db,Element, std::get<Element>(t));
		tuple_iterate<Tuple, Element+1>::iterate(t, db);
	}
};

template<typename Tuple, int Element> struct tuple_iterate<Tuple, Element, true>
{
	static void iterate(Tuple&, database_binder&)
	{
	}
};

class database_binder {
private:
	sqlite3* const _db;
	std::u16string _sql;
	sqlite3_statment _stmt;
	int _inx;
	
	bool execution_started = false;
	bool throw_exceptions = true;
	bool error_occured = false;

	void _extract(std::function<void(void)> call_back) {
		execution_started = true;
		int hresult;

		while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			call_back();
		}

		if (hresult != SQLITE_DONE) {
			throw_sqlite_error(hresult);
		}

		_stmt = nullptr;
	}
	void _extract_single_value(std::function<void(void)> call_back) {
		execution_started = true;
		int hresult;

		if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			call_back();
		}
		else if(hresult == SQLITE_DONE)
		{
			if(throw_exceptions)
				throw exceptions::no_rows("no rows to extract: exactly 1 row expected");
		}

		if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
			if(throw_exceptions)
				throw exceptions::more_rows("not all rows extracted");
		}

		if (hresult != SQLITE_DONE) {
			throw_sqlite_error(hresult);
		}

		_stmt = nullptr;
	}

	void _prepare() {
		int hresult;
		if ((hresult = sqlite3_prepare16_v2(_db, _sql.data(), -1, &_stmt, nullptr)) != SQLITE_OK) {
			throw_sqlite_error(hresult);
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

	template<typename T> friend database_binder&& operator <<(database_binder&& ddb,T const&& val);
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

	~database_binder() noexcept(false){

		/* Will be executed if no >>op is found, but not if an exception
		   is in mud flight */
		if (!execution_started && !std::current_exception()) {
			int hresult;

			while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) { }

			if (hresult != SQLITE_DONE) {
				throw_sqlite_error(hresult);
			}
		}
	}

	void throw_sqlite_error(int error_code) {
		if(throw_exceptions) {
			if(error_code == SQLITE_ERROR     ) throw exceptions::error     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_INTERNAL  ) throw exceptions::internal  (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_PERM      ) throw exceptions::perm      (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_ABORT     ) throw exceptions::abort     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_BUSY      ) throw exceptions::busy      (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_LOCKED    ) throw exceptions::locked    (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_NOMEM     ) throw exceptions::nomem     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_READONLY  ) throw exceptions::readonly  (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_INTERRUPT ) throw exceptions::interrupt (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_IOERR     ) throw exceptions::ioerr     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_CORRUPT   ) throw exceptions::corrupt   (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_NOTFOUND  ) throw exceptions::notfound  (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_FULL      ) throw exceptions::full      (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_CANTOPEN  ) throw exceptions::cantopen  (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_PROTOCOL  ) throw exceptions::protocol  (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_EMPTY     ) throw exceptions::empty     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_SCHEMA    ) throw exceptions::schema    (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_TOOBIG    ) throw exceptions::toobig    (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_CONSTRAINT) throw exceptions::constraint(sqlite3_errmsg(_db));
			else if(error_code == SQLITE_MISMATCH  ) throw exceptions::mismatch  (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_MISUSE    ) throw exceptions::misuse    (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_NOLFS     ) throw exceptions::nolfs     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_AUTH      ) throw exceptions::auth      (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_FORMAT    ) throw exceptions::format    (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_RANGE     ) throw exceptions::range     (sqlite3_errmsg(_db));
			else if(error_code == SQLITE_NOTADB    ) throw exceptions::notadb    (sqlite3_errmsg(_db));
			else throw sqlite_exception(sqlite3_errmsg(_db));
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



	template<typename... Types>
	void operator>>(std::tuple<Types...>&& values){
		this->_extract_single_value([&values, this]{
			tuple_iterate<std::tuple<Types...>>::iterate(values, *this);
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
template<> database_binder&& operator <<(database_binder&& db,int const&& val) {
	int hresult;
	if((hresult = sqlite3_bind_int(db._stmt, db._inx, val)) != SQLITE_OK) {
		db.throw_sqlite_error(hresult);
	}
	++db._inx;
	return std::move(db);
}
template<> void get_col_from_db(database_binder& db, int inx, int& val) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		val = 0;
	} else {
		val = sqlite3_column_int(db._stmt, inx);
	}
}

// sqlite_int64
template<> database_binder&& operator <<(database_binder&& db, sqlite_int64 const&& val) {
	int hresult;
	if((hresult = sqlite3_bind_int64(db._stmt, db._inx, val)) != SQLITE_OK) {
		db.throw_sqlite_error(hresult);
	}

	++db._inx;
	return std::move(db);
}
template<> void get_col_from_db(database_binder& db,int inx, sqlite3_int64& i) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		i = 0;
	} else {
		i = sqlite3_column_int64(db._stmt, inx);
	}
}

// float
template<> database_binder&& operator <<(database_binder&& db,float const&& val) {
	int hresult;
	if((hresult = sqlite3_bind_double(db._stmt, db._inx, double(val))) != SQLITE_OK) {
		db.throw_sqlite_error(hresult);
	}

	++db._inx;
	return std::move(db);
}
template<> void get_col_from_db(database_binder& db, int inx, float& f) {
	if(sqlite3_column_type(db._stmt, inx) == SQLITE_NULL) {
		f = 0;
	} else {
		f = float(sqlite3_column_double(db._stmt, inx));
	}
}

// double
template<> database_binder&& operator <<(database_binder&& db,double const&& val) {
	int hresult;
	if((hresult = sqlite3_bind_double(db._stmt, db._inx, val)) != SQLITE_OK) {
		db.throw_sqlite_error(hresult);
	}

	++db._inx;
	return std::move(db);
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
template<> database_binder&& operator <<(database_binder&& db, std::string const&& txt) {
	int hresult;
	if((hresult = sqlite3_bind_text(db._stmt, db._inx, txt.data(), -1, SQLITE_TRANSIENT)) != SQLITE_OK) {
		db.throw_sqlite_error(hresult);
	}

	++db._inx;
	return std::move(db);
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
template<> database_binder&& operator <<(database_binder&& db, std::u16string const&& txt) {
	int hresult;
	if((hresult = sqlite3_bind_text16(db._stmt, db._inx, txt.data(), -1, SQLITE_TRANSIENT)) != SQLITE_OK) {
		db.throw_sqlite_error(hresult);
	}

	++db._inx;
	return std::move(db);
}

/* call the rvalue functions */
template<typename T> database_binder&& operator <<(database_binder&& db, T const& val) { return std::move(db) << std::move(val); }

/*special case for string literals*/
template<std::size_t N> database_binder&& operator <<(database_binder&& db, const char(&STR)[N])
	{ return std::move(db) << std::string(STR); }
template<std::size_t N> database_binder&& operator <<(database_binder&& db, const char16_t(&STR)[N])
	{ return std::move(db) << std::u16string(STR); }
}
