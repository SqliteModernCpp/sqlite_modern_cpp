#pragma once

#include <string>
#include <functional>
#include <stdexcept>
#include <ctime>
#include <tuple>
#include <memory>
#include <vector>
#include <locale>
#include <codecvt>

#ifdef __has_include
#if __cplusplus > 201402 && __has_include(<optional>)
#define MODERN_SQLITE_STD_OPTIONAL_SUPPORT
#endif
#endif

#ifdef MODERN_SQLITE_STD_OPTIONAL_SUPPORT
#include <optional>
#endif

#ifdef _MODERN_SQLITE_BOOST_OPTIONAL_SUPPORT
#include <boost/optional.hpp>
#endif

#include <sqlite3.h>

#include "sqlite_modern_cpp/utility/function_traits.h"
#include "sqlite_modern_cpp/utility/uncaught_exceptions.h"

namespace sqlite {

	class sqlite_exception: public std::runtime_error {
	public:
		sqlite_exception(const char* msg, std::string sql, int code = -1): runtime_error(msg), code(code), sql(sql) {}
		sqlite_exception(int code, std::string sql): runtime_error(sqlite3_errstr(code)), code(code), sql(sql) {}
		int get_code() { return code; }
		std::string get_sql() { return sql; }
	private:
		int code;
		std::string sql;
	};

	namespace exceptions {
		//One more or less trivial derived error class for each SQLITE error.
		//Note the following are not errors so have no classes:
		//SQLITE_OK, SQLITE_NOTICE, SQLITE_WARNING, SQLITE_ROW, SQLITE_DONE
		//
		//Note these names are exact matches to the names of the SQLITE error codes.
		class error: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class internal: public sqlite_exception{ using sqlite_exception::sqlite_exception; };
		class perm: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class abort: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class busy: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class locked: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class nomem: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class readonly: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class interrupt: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class ioerr: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class corrupt: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class notfound: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class full: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class cantopen: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class protocol: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class empty: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class schema: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class toobig: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class constraint: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class mismatch: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class misuse: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class nolfs: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class auth: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class format: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class range: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class notadb: public sqlite_exception { using sqlite_exception::sqlite_exception; };

		//Some additional errors are here for the C++ interface
		class more_rows: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class no_rows: public sqlite_exception { using sqlite_exception::sqlite_exception; };

		static void throw_sqlite_error(const int& error_code, const std::string &sql = "") {
			if(error_code == SQLITE_ERROR) throw exceptions::error(error_code, sql);
			else if(error_code == SQLITE_INTERNAL) throw exceptions::internal(error_code, sql);
			else if(error_code == SQLITE_PERM) throw exceptions::perm(error_code, sql);
			else if(error_code == SQLITE_ABORT) throw exceptions::abort(error_code, sql);
			else if(error_code == SQLITE_BUSY) throw exceptions::busy(error_code, sql);
			else if(error_code == SQLITE_LOCKED) throw exceptions::locked(error_code, sql);
			else if(error_code == SQLITE_NOMEM) throw exceptions::nomem(error_code, sql);
			else if(error_code == SQLITE_READONLY) throw exceptions::readonly(error_code, sql);
			else if(error_code == SQLITE_INTERRUPT) throw exceptions::interrupt(error_code, sql);
			else if(error_code == SQLITE_IOERR) throw exceptions::ioerr(error_code, sql);
			else if(error_code == SQLITE_CORRUPT) throw exceptions::corrupt(error_code, sql);
			else if(error_code == SQLITE_NOTFOUND) throw exceptions::notfound(error_code, sql);
			else if(error_code == SQLITE_FULL) throw exceptions::full(error_code, sql);
			else if(error_code == SQLITE_CANTOPEN) throw exceptions::cantopen(error_code, sql);
			else if(error_code == SQLITE_PROTOCOL) throw exceptions::protocol(error_code, sql);
			else if(error_code == SQLITE_EMPTY) throw exceptions::empty(error_code, sql);
			else if(error_code == SQLITE_SCHEMA) throw exceptions::schema(error_code, sql);
			else if(error_code == SQLITE_TOOBIG) throw exceptions::toobig(error_code, sql);
			else if(error_code == SQLITE_CONSTRAINT) throw exceptions::constraint(error_code, sql);
			else if(error_code == SQLITE_MISMATCH) throw exceptions::mismatch(error_code, sql);
			else if(error_code == SQLITE_MISUSE) throw exceptions::misuse(error_code, sql);
			else if(error_code == SQLITE_NOLFS) throw exceptions::nolfs(error_code, sql);
			else if(error_code == SQLITE_AUTH) throw exceptions::auth(error_code, sql);
			else if(error_code == SQLITE_FORMAT) throw exceptions::format(error_code, sql);
			else if(error_code == SQLITE_RANGE) throw exceptions::range(error_code, sql);
			else if(error_code == SQLITE_NOTADB) throw exceptions::notadb(error_code, sql);
			else throw sqlite_exception(error_code, sql);
		}
	}


	class database;
	class database_binder;



	template<std::size_t> class binder;

	typedef std::shared_ptr<sqlite3> connection_type;

	template<typename Tuple, int Element = 0, bool Last = (std::tuple_size<Tuple>::value == Element)> struct tuple_iterate {
		static void iterate(Tuple& t, database_binder& db) {
			get_col_from_db(db, Element, std::get<Element>(t));
			tuple_iterate<Tuple, Element + 1>::iterate(t, db);
		}
	};

	template<typename Tuple, int Element> struct tuple_iterate<Tuple, Element, true> {
		static void iterate(Tuple&, database_binder&) {}
	};

	struct database_binder_hooks {
		virtual bool pre_step(const database_binder& binder) { return true; } // usually noop but could stop query if necessary
		virtual bool post_callback(bool user_return) { return user_return; } // checking user return code
		virtual bool post_step(int sql_result) { return sql_result == SQLITE_ROW; } // checking return code and stop if necessary
		virtual void error_throw() {} // manage how and if errors are handled
	};

	class database_binder {
	protected:
		std::shared_ptr<sqlite3> _db;
		std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> _stmt;
		std::shared_ptr<database_binder_hooks> _hooks;

	public:
		// database_binder is not copyable
		database_binder() = delete;
		database_binder(const database_binder& other) = delete;
		database_binder& operator=(const database_binder&) = delete;

		database_binder(database_binder&& other):
			_db(std::move(other._db)),
			_stmt(std::move(other._stmt)), _hooks(std::move(other._hooks)),
			_inx(other._inx), execution_started(other.execution_started) {}

		void reset() {
			sqlite3_reset(_stmt.get());
			sqlite3_clear_bindings(_stmt.get());
			_inx = 1;
			used(false);
		}

		void execute() {
			int hresult = SQLITE_ERROR;


			do {
				if(_hooks && !_hooks->pre_step(*this)) break;
				hresult = sqlite3_step(_stmt.get());
				if(_hooks && _hooks->post_step(hresult)) {};
			} while(hresult == SQLITE_ROW);

			if(hresult != SQLITE_DONE) {
				exceptions::throw_sqlite_error(hresult, sql());
			}
			used(true); /* prevent from executing again when it goes out of scope */
		}

		std::string sql() const {
#if SQLITE_VERSION_NUMBER >= 3014000
			auto sqlite_deleter = [](void *ptr) {sqlite3_free(ptr); };
			std::unique_ptr<char, decltype(sqlite_deleter)> str(sqlite3_expanded_sql(_stmt.get()), sqlite_deleter);
			return str ? str.get() : original_sql();
#else
			return original_sql();
#endif
		}

		std::string original_sql() const {
			return sqlite3_sql(_stmt.get());
		}

		void used(bool state) { execution_started = state; }
		bool used() const { return execution_started; }

	private:
		utility::UncaughtExceptionDetector _has_uncaught_exception;

		int _inx;

		bool execution_started = false;

		void _extract(std::function<bool(void)> call_back) {
			execution_started = true;
			int hresult;

			while(true) {
				if(_hooks && !_hooks->pre_step(*this)) break;
				hresult = sqlite3_step(_stmt.get());
				if(_hooks && !_hooks->post_callback(call_back())) break;
				if(_hooks) {
					if(!_hooks->post_step(hresult))break;
				} else {
					if(hresult != SQLITE_ROW) break;
				}
			}

			if(hresult != SQLITE_DONE) {
				exceptions::throw_sqlite_error(hresult, sql());
			}
			//reset();
		}

		void _extract_single_value(std::function<bool(void)> call_back) {
			execution_started = true;
			int hresult;

			if((hresult = sqlite3_step(_stmt.get())) == SQLITE_ROW) {
				call_back();
			} else if(hresult == SQLITE_DONE) {
				throw exceptions::no_rows("no rows to extract: exactly 1 row expected", sql(), SQLITE_DONE);
			}

			if((hresult = sqlite3_step(_stmt.get())) == SQLITE_ROW) {
				throw exceptions::more_rows("not all rows extracted", sql(), SQLITE_ROW);
			}

			if(hresult != SQLITE_DONE) {
				exceptions::throw_sqlite_error(hresult, sql());
			}
			reset();
		}

#ifdef _MSC_VER
		sqlite3_stmt* _prepare(const std::u16string& sql) {
			return _prepare(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(reinterpret_cast<const wchar_t*>(sql.c_str())));
		}
#else
		sqlite3_stmt* _prepare(const std::u16string& sql) {
			return _prepare(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(sql));
		}
#endif

		sqlite3_stmt* _prepare(const std::string& sql) {
			int hresult;
			sqlite3_stmt* tmp = nullptr;
			hresult = sqlite3_prepare_v2(_db.get(), sql.data(), -1, &tmp, nullptr);
			if((hresult) != SQLITE_OK) exceptions::throw_sqlite_error(hresult, sql);
			return tmp;
		}

		template <typename Type>
		struct is_sqlite_value: public std::integral_constant<
			bool,
			std::is_floating_point<Type>::value
			|| std::is_integral<Type>::value
			|| std::is_same<std::string, Type>::value
			|| std::is_same<std::u16string, Type>::value
			|| std::is_same<sqlite_int64, Type>::value
		> {};
		template <typename Type, typename Allocator>
		struct is_sqlite_value< std::vector<Type, Allocator> >: public std::integral_constant<
			bool,
			std::is_floating_point<Type>::value
			|| std::is_integral<Type>::value
			|| std::is_same<sqlite_int64, Type>::value
		> {};


		template<typename T> friend database_binder& operator <<(database_binder& db, const T& val);
		template<typename T> friend void get_col_from_db(database_binder& db, int inx, T& val);
		/* for vector<T, A> support */
		template<typename T, typename A> friend database_binder& operator <<(database_binder& db, const std::vector<T, A>& val);
		template<typename T, typename A> friend void get_col_from_db(database_binder& db, int inx, std::vector<T, A>& val);
		/* for nullptr & unique_ptr support */
		friend database_binder& operator <<(database_binder& db, std::nullptr_t);
		template<typename T> friend database_binder& operator <<(database_binder& db, const std::unique_ptr<T>& val);
		template<typename T> friend void get_col_from_db(database_binder& db, int inx, std::unique_ptr<T>& val);
		template<typename T> friend T operator++(database_binder& db, int);
		// Overload instead of specializing function templates (http://www.gotw.ca/publications/mill17.htm)
		friend database_binder& operator<<(database_binder& db, const int& val);
		friend void get_col_from_db(database_binder& db, int inx, int& val);
		friend database_binder& operator <<(database_binder& db, const sqlite_int64&  val);
		friend void get_col_from_db(database_binder& db, int inx, sqlite3_int64& i);
		friend database_binder& operator <<(database_binder& db, const float& val);
		friend void get_col_from_db(database_binder& db, int inx, float& f);
		friend database_binder& operator <<(database_binder& db, const double& val);
		friend void get_col_from_db(database_binder& db, int inx, double& d);
		friend void get_col_from_db(database_binder& db, int inx, std::string & s);
		friend database_binder& operator <<(database_binder& db, const std::string& txt);
		friend void get_col_from_db(database_binder& db, int inx, std::u16string & w);
		friend database_binder& operator <<(database_binder& db, const std::u16string& txt);

#ifdef MODERN_SQLITE_STD_OPTIONAL_SUPPORT
		template <typename OptionalT> friend database_binder& operator <<(database_binder& db, const std::optional<OptionalT>& val);
		template <typename OptionalT> friend void get_col_from_db(database_binder& db, int inx, std::optional<OptionalT>& o);
#endif

#ifdef _MODERN_SQLITE_BOOST_OPTIONAL_SUPPORT
		template <typename BoostOptionalT> friend database_binder& operator <<(database_binder& db, const boost::optional<BoostOptionalT>& val);
		template <typename BoostOptionalT> friend void get_col_from_db(database_binder& db, int inx, boost::optional<BoostOptionalT>& o);
#endif

	public:

		database_binder(std::shared_ptr<sqlite3> db, std::u16string const & sql, std::shared_ptr<database_binder_hooks> hooks):
			_db(db),
			_stmt(_prepare(sql), sqlite3_finalize),
			_inx(1), _hooks(hooks) {}

		database_binder(std::shared_ptr<sqlite3> db, std::string const & sql, std::shared_ptr<database_binder_hooks> hooks):
			_db(db),
			_stmt(_prepare(sql), sqlite3_finalize),
			_inx(1), _hooks(hooks) {}

		~database_binder() noexcept(false) {
			/* Will be executed if no >>op is found, but not if an exception
			is in mid flight */
			if(!execution_started && !_has_uncaught_exception && _stmt) {
				execute();
			}
		}

		template <typename Result>
		typename std::enable_if<is_sqlite_value<Result>::value, void>::type operator >> (
			Result& value) {
			this->_extract_single_value([&value, this] {
				get_col_from_db(*this, 0, value);
			});
		}

		template<typename... Types>
		void operator >> (std::tuple<Types...>&& values) {
			this->_extract_single_value([&values, this] {
				tuple_iterate<std::tuple<Types...>>::iterate(values, *this);
			});
		}

		template <typename Function>
		typename std::enable_if<!is_sqlite_value<Function>::value, void>::type operator >> (
			Function&& func) {
			typedef utility::function_traits<Function> traits;

			this->_extract([&func, this]() {
				return binder<traits::arity>::run(*this, func);
			});
		}
	};

	namespace sql_function_binder {
		template<
			typename    ContextType,
			std::size_t Count,
			typename    Functions
		>
			inline void step(
				sqlite3_context* db,
				int              count,
				sqlite3_value**  vals
			);

		template<
			std::size_t Count,
			typename    Functions,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) && sizeof...(Values) < Count), void>::type step(
				sqlite3_context* db,
				int              count,
				sqlite3_value**  vals,
				Values&&...      values
			);

		template<
			std::size_t Count,
			typename    Functions,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) == Count), void>::type step(
				sqlite3_context* db,
				int,
				sqlite3_value**,
				Values&&...      values
			);

		template<
			typename    ContextType,
			typename    Functions
		>
			inline void final(sqlite3_context* db);

		template<
			std::size_t Count,
			typename    Function,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) < Count), void>::type scalar(
				sqlite3_context* db,
				int              count,
				sqlite3_value**  vals,
				Values&&...      values
			);

		template<
			std::size_t Count,
			typename    Function,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) == Count), void>::type scalar(
				sqlite3_context* db,
				int,
				sqlite3_value**,
				Values&&...      values
			);
	}

	enum class OpenFlags {
		READONLY = SQLITE_OPEN_READONLY,
		READWRITE = SQLITE_OPEN_READWRITE,
		CREATE = SQLITE_OPEN_CREATE,
		NOMUTEX = SQLITE_OPEN_NOMUTEX,
		FULLMUTEX = SQLITE_OPEN_FULLMUTEX,
		SHAREDCACHE = SQLITE_OPEN_SHAREDCACHE,
		PRIVATECACH = SQLITE_OPEN_PRIVATECACHE,
		URI = SQLITE_OPEN_URI
	};
	OpenFlags operator|(const OpenFlags& a, const OpenFlags& b) {
		return static_cast<OpenFlags>(static_cast<int>(a) | static_cast<int>(b));
	};
	enum class Encoding {
		ANY = SQLITE_ANY,
		UTF8 = SQLITE_UTF8,
		UTF16 = SQLITE_UTF16
	};
	struct sqlite_config {
		OpenFlags flags = OpenFlags::READWRITE | OpenFlags::CREATE;
		const char *zVfs = nullptr;
		std::shared_ptr<database_binder_hooks> hooks;
		Encoding encoding = Encoding::ANY;
	};

	class database {
	protected:
		std::shared_ptr<sqlite3> _db;
		std::shared_ptr<database_binder_hooks> _hooks;

	public:
		database(const std::string &db_name, const sqlite_config &config = {}): _db(nullptr) {
			sqlite3* tmp = nullptr;
			auto ret = sqlite3_open_v2(db_name.data(), &tmp, static_cast<int>(config.flags), config.zVfs);
			_db = std::shared_ptr<sqlite3>(tmp, [=](sqlite3* ptr) { sqlite3_close_v2(ptr); }); // this will close the connection eventually when no longer needed.
			if(ret != SQLITE_OK) exceptions::throw_sqlite_error(ret);
			if(config.encoding == Encoding::UTF16)
				*this << R"(PRAGMA encoding = "UTF-16";)";
			if(config.hooks) _hooks = config.hooks;
		}

		database(const std::u16string &db_name, const sqlite_config &config = {}): _db(nullptr) {
#ifdef _MSC_VER
			auto db_name_utf8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(reinterpret_cast<const wchar_t*>(db_name.c_str()));
#else
			auto db_name_utf8 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(db_name);
#endif
			sqlite3* tmp = nullptr;
			auto ret = sqlite3_open_v2(db_name_utf8.data(), &tmp, static_cast<int>(config.flags), config.zVfs);
			_db = std::shared_ptr<sqlite3>(tmp, [=](sqlite3* ptr) { sqlite3_close_v2(ptr); }); // this will close the connection eventually when no longer needed.
			if(ret != SQLITE_OK) exceptions::throw_sqlite_error(ret);
			if(config.encoding != Encoding::UTF8)
				*this << R"(PRAGMA encoding = "UTF-16";)";
		}

		database(std::shared_ptr<sqlite3> db):
			_db(db) {}

		database_binder operator<<(const std::string& sql) {
			return database_binder(_db, sql, _hooks);
		}

		database_binder operator<<(const char* sql) {
			return *this << std::string(sql);
		}
		database_binder operator<<(const std::u16string& sql) {
			return database_binder(_db, sql, _hooks);
		}

		database_binder operator<<(const char16_t* sql) {
			return *this << std::u16string(sql);
		}

		connection_type connection() const { return _db; }

		sqlite3_int64 last_insert_rowid() const {
			return sqlite3_last_insert_rowid(_db.get());
		}

		template <typename Function>
		void define(const std::string &name, Function&& func) {
			typedef utility::function_traits<Function> traits;

			auto funcPtr = new auto(std::forward<Function>(func));
			if(int result = sqlite3_create_function_v2(
				_db.get(), name.c_str(), traits::arity, SQLITE_UTF8, funcPtr,
				sql_function_binder::scalar<traits::arity, typename std::remove_reference<Function>::type>,
				nullptr, nullptr, [](void* ptr) {
				delete static_cast<decltype(funcPtr)>(ptr);
			}))
				exceptions::throw_sqlite_error(result);
		}

		template <typename StepFunction, typename FinalFunction>
		void define(const std::string &name, StepFunction&& step, FinalFunction&& final) {
			typedef utility::function_traits<StepFunction> traits;
			using ContextType = typename std::remove_reference<typename traits::template argument<0>>::type;

			auto funcPtr = new auto(std::make_pair(std::forward<StepFunction>(step), std::forward<FinalFunction>(final)));
			if(int result = sqlite3_create_function_v2(
				_db.get(), name.c_str(), traits::arity - 1, SQLITE_UTF8, funcPtr, nullptr,
				sql_function_binder::step<ContextType, traits::arity, typename std::remove_reference<decltype(*funcPtr)>::type>,
				sql_function_binder::final<ContextType, typename std::remove_reference<decltype(*funcPtr)>::type>,
				[](void* ptr) {
				delete static_cast<decltype(funcPtr)>(ptr);
			}))
				exceptions::throw_sqlite_error(result);
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
			static typename std::enable_if<(sizeof...(Values) < Boundary), bool>::type run(
				database_binder& db,
				Function&&       function,
				Values&&...      values
			) {
			typename std::remove_cv<typename std::remove_reference<nth_argument_type<Function, sizeof...(Values)>>::type>::type value{};
			get_col_from_db(db, sizeof...(Values), value);

			return run<Function>(db, function, std::forward<Values>(values)..., std::move(value));
		}

		template<
			typename    Function,
			typename... Values
		>
			static bool user_function_wrapper(
				std::true_type,
				Function&&       function,
				Values&&...      values) {
			return function(std::move(values)...);
		}

		template<
			typename    Function,
			typename... Values
		>
			static bool user_function_wrapper(
				std::false_type,
				Function&&       function,
				Values&&...      values) {
			function(std::move(values)...);
			return true;
		}

		template<
			typename    Function,
			typename... Values,
			std::size_t Boundary = Count
		>
			static typename std::enable_if<(sizeof...(Values) == Boundary), bool>::type run(
				database_binder&,
				Function&&       function,
				Values&&...      values
			) {
			return user_function_wrapper(std::is_same<bool, decltype(function(values...))>(), function, std::forward<Values>(values)...);
		}



	};

	// int
	inline database_binder& operator<<(database_binder& db, const int& val) {
		int hresult;
		if((hresult = sqlite3_bind_int(db._stmt.get(), db._inx, val)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}
		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, const int& val) {
		sqlite3_result_int(db, val);
	}
	inline void get_col_from_db(database_binder& db, int inx, int& val) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			val = 0;
		} else {
			val = sqlite3_column_int(db._stmt.get(), inx);
		}
	}
	inline void get_val_from_db(sqlite3_value *value, int& val) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			val = 0;
		} else {
			val = sqlite3_value_int(value);
		}
	}

	// sqlite_int64
	inline database_binder& operator <<(database_binder& db, const sqlite_int64&  val) {
		int hresult;
		if((hresult = sqlite3_bind_int64(db._stmt.get(), db._inx, val)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, const sqlite_int64& val) {
		sqlite3_result_int64(db, val);
	}
	inline void get_col_from_db(database_binder& db, int inx, sqlite3_int64& i) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			i = 0;
		} else {
			i = sqlite3_column_int64(db._stmt.get(), inx);
		}
	}
	inline void get_val_from_db(sqlite3_value *value, sqlite3_int64& i) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			i = 0;
		} else {
			i = sqlite3_value_int64(value);
		}
	}

	// float
	inline database_binder& operator <<(database_binder& db, const float& val) {
		int hresult;
		if((hresult = sqlite3_bind_double(db._stmt.get(), db._inx, double(val))) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, const float& val) {
		sqlite3_result_double(db, val);
	}
	inline void get_col_from_db(database_binder& db, int inx, float& f) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			f = 0;
		} else {
			f = float(sqlite3_column_double(db._stmt.get(), inx));
		}
	}
	inline void get_val_from_db(sqlite3_value *value, float& f) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			f = 0;
		} else {
			f = float(sqlite3_value_double(value));
		}
	}

	// double
	inline database_binder& operator <<(database_binder& db, const double& val) {
		int hresult;
		if((hresult = sqlite3_bind_double(db._stmt.get(), db._inx, val)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, const double& val) {
		sqlite3_result_double(db, val);
	}
	inline void get_col_from_db(database_binder& db, int inx, double& d) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			d = 0;
		} else {
			d = sqlite3_column_double(db._stmt.get(), inx);
		}
	}
	inline void get_val_from_db(sqlite3_value *value, double& d) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			d = 0;
		} else {
			d = sqlite3_value_double(value);
		}
	}

	// vector<T, A>
	template<typename T, typename A> inline database_binder& operator<<(database_binder& db, const std::vector<T, A>& vec) {
		void const* buf = reinterpret_cast<void const *>(vec.data());
		int bytes = vec.size() * sizeof(T);
		int hresult;
		if((hresult = sqlite3_bind_blob(db._stmt.get(), db._inx, buf, bytes, SQLITE_TRANSIENT)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}
		++db._inx;
		return db;
	}
	template<typename T, typename A> inline void store_result_in_db(sqlite3_context* db, const std::vector<T, A>& vec) {
		void const* buf = reinterpret_cast<void const *>(vec.data());
		int bytes = vec.size() * sizeof(T);
		sqlite3_result_blob(db, buf, bytes, SQLITE_TRANSIENT);
	}
	template<typename T, typename A> inline void get_col_from_db(database_binder& db, int inx, std::vector<T, A>& vec) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			vec.clear();
		} else {
			int bytes = sqlite3_column_bytes(db._stmt.get(), inx);
			T const* buf = reinterpret_cast<T const *>(sqlite3_column_blob(db._stmt.get(), inx));
			vec = std::vector<T, A>(buf, buf + bytes / sizeof(T));
		}
	}
	template<typename T, typename A> inline void get_val_from_db(sqlite3_value *value, std::vector<T, A>& vec) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			vec.clear();
		} else {
			int bytes = sqlite3_value_bytes(value);
			T const* buf = reinterpret_cast<T const *>(sqlite3_value_blob(value));
			vec = std::vector<T, A>(buf, buf + bytes / sizeof(T));
		}
	}

	/* for nullptr support */
	inline database_binder& operator <<(database_binder& db, std::nullptr_t) {
		int hresult;
		if((hresult = sqlite3_bind_null(db._stmt.get(), db._inx)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}
		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, std::nullptr_t) {
		sqlite3_result_null(db);
	}
	/* for nullptr support */
	template<typename T> inline database_binder& operator <<(database_binder& db, const std::unique_ptr<T>& val) {
		if(val)
			db << *val;
		else
			db << nullptr;
		return db;
	}

	/* for unique_ptr<T> support */
	template<typename T> inline void get_col_from_db(database_binder& db, int inx, std::unique_ptr<T>& _ptr_) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			_ptr_ = nullptr;
		} else {
			auto underling_ptr = new T();
			get_col_from_db(db, inx, *underling_ptr);
			_ptr_.reset(underling_ptr);
		}
	}
	template<typename T> inline void get_val_from_db(sqlite3_value *value, std::unique_ptr<T>& _ptr_) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			_ptr_ = nullptr;
		} else {
			auto underling_ptr = new T();
			get_val_from_db(value, *underling_ptr);
			_ptr_.reset(underling_ptr);
		}
	}

	// std::string
	inline void get_col_from_db(database_binder& db, int inx, std::string & s) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			s = std::string();
		} else {
			sqlite3_column_bytes(db._stmt.get(), inx);
			s = std::string(reinterpret_cast<char const *>(sqlite3_column_text(db._stmt.get(), inx)));
		}
	}
	inline void get_val_from_db(sqlite3_value *value, std::string & s) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			s = std::string();
		} else {
			sqlite3_value_bytes(value);
			s = std::string(reinterpret_cast<char const *>(sqlite3_value_text(value)));
		}
	}

	// Convert char* to string to trigger op<<(..., const std::string )
	template<std::size_t N> inline database_binder& operator <<(database_binder& db, const char(&STR)[N]) { return db << std::string(STR); }
	template<std::size_t N> inline database_binder& operator <<(database_binder& db, const char16_t(&STR)[N]) { return db << std::u16string(STR); }

	inline database_binder& operator <<(database_binder& db, const std::string& txt) {
		int hresult;
		if((hresult = sqlite3_bind_text(db._stmt.get(), db._inx, txt.data(), -1, SQLITE_TRANSIENT)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, const std::string& val) {
		sqlite3_result_text(db, val.data(), -1, SQLITE_TRANSIENT);
	}
	// std::u16string
	inline void get_col_from_db(database_binder& db, int inx, std::u16string & w) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			w = std::u16string();
		} else {
			sqlite3_column_bytes16(db._stmt.get(), inx);
			w = std::u16string(reinterpret_cast<char16_t const *>(sqlite3_column_text16(db._stmt.get(), inx)));
		}
	}
	inline void get_val_from_db(sqlite3_value *value, std::u16string & w) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			w = std::u16string();
		} else {
			sqlite3_value_bytes16(value);
			w = std::u16string(reinterpret_cast<char16_t const *>(sqlite3_value_text16(value)));
		}
	}


	inline database_binder& operator <<(database_binder& db, const std::u16string& txt) {
		int hresult;
		if((hresult = sqlite3_bind_text16(db._stmt.get(), db._inx, txt.data(), -1, SQLITE_TRANSIENT)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	inline void store_result_in_db(sqlite3_context* db, const std::u16string& val) {
		sqlite3_result_text16(db, val.data(), -1, SQLITE_TRANSIENT);
	}
	// std::optional support for NULL values
#ifdef MODERN_SQLITE_STD_OPTIONAL_SUPPORT
	template <typename OptionalT> inline database_binder& operator <<(database_binder& db, const std::optional<OptionalT>& val) {
		if(val) {
			return operator << (std::move(db), std::move(*val));
		}
		int hresult;
		if((hresult = sqlite3_bind_null(db._stmt.get(), db._inx)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	template <typename OptionalT> inline void store_result_in_db(sqlite3_context* db, const std::optional<OptionalT>& val) {
		if(val) {
			store_result_in_db(db, *val);
		}
		sqlite3_result_null(db);
	}

	template <typename OptionalT> inline void get_col_from_db(database_binder& db, int inx, std::optional<OptionalT>& o) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			o.reset();
		} else {
			OptionalT v;
			get_col_from_db(db, inx, v);
			o = std::move(v);
		}
	}
	template <typename OptionalT> inline void get_val_from_db(sqlite3_value *value, std::optional<OptionalT>& o) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			o.reset();
		} else {
			OptionalT v;
			get_val_from_db(value, v);
			o = std::move(v);
		}
	}
#endif

	// boost::optional support for NULL values
#ifdef _MODERN_SQLITE_BOOST_OPTIONAL_SUPPORT
	template <typename BoostOptionalT> inline database_binder& operator <<(database_binder& db, const boost::optional<BoostOptionalT>& val) {
		if(val) {
			return operator << (std::move(db), std::move(*val));
		}
		int hresult;
		if((hresult = sqlite3_bind_null(db._stmt.get(), db._inx)) != SQLITE_OK) {
			exceptions::throw_sqlite_error(hresult, db.sql());
		}

		++db._inx;
		return db;
	}
	template <typename BoostOptionalT> inline void store_result_in_db(sqlite3_context* db, const boost::optional<BoostOptionalT>& val) {
		if(val) {
			store_result_in_db(db, *val);
		}
		sqlite3_result_null(db);
	}

	template <typename BoostOptionalT> inline void get_col_from_db(database_binder& db, int inx, boost::optional<BoostOptionalT>& o) {
		if(sqlite3_column_type(db._stmt.get(), inx) == SQLITE_NULL) {
			o.reset();
		} else {
			BoostOptionalT v;
			get_col_from_db(db, inx, v);
			o = std::move(v);
		}
	}
	template <typename BoostOptionalT> inline void get_val_from_db(sqlite3_value *value, boost::optional<BoostOptionalT>& o) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			o.reset();
		} else {
			BoostOptionalT v;
			get_val_from_db(value, v);
			o = std::move(v);
		}
	}
#endif

	// Some ppl are lazy so we have a operator for proper prep. statemant handling.
	void inline operator++(database_binder& db, int) { db.execute(); db.reset(); }

	// Convert the rValue binder to a reference and call first op<<, its needed for the call that creates the binder (be carefull of recursion here!)
	template<typename T> database_binder& operator << (database_binder&& db, const T& val) { return db << val; }

	namespace sql_function_binder {
		template<class T>
		struct AggregateCtxt {
			T obj;
			bool constructed = true;
		};

		template<
			typename ContextType,
			std::size_t Count,
			typename    Functions
		>
			inline void step(
				sqlite3_context* db,
				int              count,
				sqlite3_value**  vals
			) {
			auto ctxt = static_cast<AggregateCtxt<ContextType>*>(sqlite3_aggregate_context(db, sizeof(AggregateCtxt<ContextType>)));
			if(!ctxt) return;
			try {
				if(!ctxt->constructed) new(ctxt) AggregateCtxt<ContextType>();
				step<Count, Functions>(db, count, vals, ctxt->obj);
				return;
			} catch(sqlite_exception &e) {
				sqlite3_result_error_code(db, e.get_code());
				sqlite3_result_error(db, e.what(), -1);
			} catch(std::exception &e) {
				sqlite3_result_error(db, e.what(), -1);
			} catch(...) {
				sqlite3_result_error(db, "Unknown error", -1);
			}
			if(ctxt && ctxt->constructed)
				ctxt->~AggregateCtxt();
		}

		template<
			std::size_t Count,
			typename    Functions,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) && sizeof...(Values) < Count), void>::type step(
				sqlite3_context* db,
				int              count,
				sqlite3_value**  vals,
				Values&&...      values
			) {
			typename std::remove_cv<
				typename std::remove_reference<
				typename utility::function_traits<
				typename Functions::first_type
							>::template argument<sizeof...(Values)>
				>::type
			>::type value{};
			get_val_from_db(vals[sizeof...(Values)-1], value);

			step<Count, Functions>(db, count, vals, std::forward<Values>(values)..., std::move(value));
		}

		template<
			std::size_t Count,
			typename    Functions,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) == Count), void>::type step(
				sqlite3_context* db,
				int,
				sqlite3_value**,
				Values&&...      values
			) {
			static_cast<Functions*>(sqlite3_user_data(db))->first(std::forward<Values>(values)...);
		};

		template<
			typename    ContextType,
			typename    Functions
		>
			inline void final(sqlite3_context* db) {
			auto ctxt = static_cast<AggregateCtxt<ContextType>*>(sqlite3_aggregate_context(db, sizeof(AggregateCtxt<ContextType>)));
			try {
				if(!ctxt) return;
				if(!ctxt->constructed) new(ctxt) AggregateCtxt<ContextType>();
				store_result_in_db(db,
					static_cast<Functions*>(sqlite3_user_data(db))->second(ctxt->obj));
			} catch(sqlite_exception &e) {
				sqlite3_result_error_code(db, e.get_code());
				sqlite3_result_error(db, e.what(), -1);
			} catch(std::exception &e) {
				sqlite3_result_error(db, e.what(), -1);
			} catch(...) {
				sqlite3_result_error(db, "Unknown error", -1);
			}
			if(ctxt && ctxt->constructed)
				ctxt->~AggregateCtxt();
		}

		template<
			std::size_t Count,
			typename    Function,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) < Count), void>::type scalar(
				sqlite3_context* db,
				int              count,
				sqlite3_value**  vals,
				Values&&...      values
			) {
			typename std::remove_cv<
				typename std::remove_reference<
				typename utility::function_traits<Function>::template argument<sizeof...(Values)>
				>::type
			>::type value{};
			get_val_from_db(vals[sizeof...(Values)], value);

			scalar<Count, Function>(db, count, vals, std::forward<Values>(values)..., std::move(value));
		}

		template<
			std::size_t Count,
			typename    Function,
			typename... Values
		>
			inline typename std::enable_if<(sizeof...(Values) == Count), void>::type scalar(
				sqlite3_context* db,
				int,
				sqlite3_value**,
				Values&&...      values
			) {
			try {
				store_result_in_db(db,
					(*static_cast<Function*>(sqlite3_user_data(db)))(std::forward<Values>(values)...));
			} catch(sqlite_exception &e) {
				sqlite3_result_error_code(db, e.get_code());
				sqlite3_result_error(db, e.what(), -1);
			} catch(std::exception &e) {
				sqlite3_result_error(db, e.what(), -1);
			} catch(...) {
				sqlite3_result_error(db, "Unknown error", -1);
			}
		}
	}
}
