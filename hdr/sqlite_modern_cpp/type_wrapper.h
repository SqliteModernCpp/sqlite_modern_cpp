#pragma once

#include <type_traits>
#include <string>
#include <memory>
#include <vector>

#ifdef __has_include
#if __cplusplus > 201402 && __has_include(<optional>)
#define MODERN_SQLITE_STD_OPTIONAL_SUPPORT
#elif __has_include(<experimental/optional>)
#define MODERN_SQLITE_EXPERIMENTAL_OPTIONAL_SUPPORT
#endif
#endif

#ifdef __has_include
#if __cplusplus > 201402 && __has_include(<variant>)
#define MODERN_SQLITE_STD_VARIANT_SUPPORT
#endif
#endif

#ifdef MODERN_SQLITE_STD_OPTIONAL_SUPPORT
#include <optional>
#endif

#ifdef MODERN_SQLITE_EXPERIMENTAL_OPTIONAL_SUPPORT
#include <experimental/optional>
#define MODERN_SQLITE_STD_OPTIONAL_SUPPORT
#endif

#ifdef MODERN_SQLITE_STD_VARIANT_SUPPORT
#include <variant>
#endif

#include <sqlite3.h>
#include "errors.h"

namespace sqlite {
	template<class T, int Type, class = void>
	struct has_sqlite_type : std::false_type {};

	template<class T, int Type>
	struct has_sqlite_type<T&, Type> : has_sqlite_type<T, Type> {};
	template<class T, int Type>
	struct has_sqlite_type<const T, Type> : has_sqlite_type<T, Type> {};
	template<class T, int Type>
	struct has_sqlite_type<volatile T, Type> : has_sqlite_type<T, Type> {};

	// int
	template<>
	struct has_sqlite_type<int, SQLITE_INTEGER> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const int& val) {
		return sqlite3_bind_int(stmt, inx, val);
	}
	inline void store_result_in_db(sqlite3_context* db, const int& val) {
		sqlite3_result_int(db, val);
	}
	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, int& val) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			val = 0;
		} else {
			val = sqlite3_column_int(stmt, inx);
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
	template<>
	struct has_sqlite_type<sqlite_int64, SQLITE_INTEGER, void> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const sqlite_int64& val) {
		return sqlite3_bind_int64(stmt, inx, val);
	}
	inline void store_result_in_db(sqlite3_context* db, const sqlite_int64& val) {
		sqlite3_result_int64(db, val);
	}
	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, sqlite3_int64& i) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			i = 0;
		} else {
			i = sqlite3_column_int64(stmt, inx);
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
	template<>
	struct has_sqlite_type<float, SQLITE_FLOAT, void> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const float& val) {
		return sqlite3_bind_double(stmt, inx, double(val));
	}
	inline void store_result_in_db(sqlite3_context* db, const float& val) {
		sqlite3_result_double(db, val);
	}
	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, float& f) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			f = 0;
		} else {
			f = float(sqlite3_column_double(stmt, inx));
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
	template<>
	struct has_sqlite_type<double, SQLITE_FLOAT, void> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const double& val) {
		return sqlite3_bind_double(stmt, inx, val);
	}
	inline void store_result_in_db(sqlite3_context* db, const double& val) {
		sqlite3_result_double(db, val);
	}
	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, double& d) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			d = 0;
		} else {
			d = sqlite3_column_double(stmt, inx);
		}
	}
	inline void get_val_from_db(sqlite3_value *value, double& d) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			d = 0;
		} else {
			d = sqlite3_value_double(value);
		}
	}

	/* for nullptr support */
	template<>
	struct has_sqlite_type<std::nullptr_t, SQLITE_NULL, void> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, std::nullptr_t) {
		return sqlite3_bind_null(stmt, inx);
	}
	inline void store_result_in_db(sqlite3_context* db, std::nullptr_t) {
		sqlite3_result_null(db);
	}

	// std::string
	template<>
	struct has_sqlite_type<std::string, SQLITE3_TEXT, void> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const std::string& val) {
		return sqlite3_bind_text(stmt, inx, val.data(), -1, SQLITE_TRANSIENT);
	}

	// Convert char* to string to trigger op<<(..., const std::string )
	template<std::size_t N> inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const char(&STR)[N]) { return bind_col_in_db(stmt, inx, std::string(STR, N-1)); }

	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, std::string & s) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			s = std::string();
		} else {
			sqlite3_column_bytes(stmt, inx);
			s = std::string(reinterpret_cast<char const *>(sqlite3_column_text(stmt, inx)));
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

	inline void store_result_in_db(sqlite3_context* db, const std::string& val) {
		sqlite3_result_text(db, val.data(), -1, SQLITE_TRANSIENT);
	}
	// std::u16string
	template<>
	struct has_sqlite_type<std::u16string, SQLITE3_TEXT, void> : std::true_type {};

	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const std::u16string& val) {
		return sqlite3_bind_text16(stmt, inx, val.data(), -1, SQLITE_TRANSIENT);
	}

	// Convert char* to string to trigger op<<(..., const std::string )
	template<std::size_t N> inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const char16_t(&STR)[N]) { return bind_col_in_db(stmt, inx, std::u16string(STR, N-1)); }

	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, std::u16string & w) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			w = std::u16string();
		} else {
			sqlite3_column_bytes16(stmt, inx);
			w = std::u16string(reinterpret_cast<char16_t const *>(sqlite3_column_text16(stmt, inx)));
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

	inline void store_result_in_db(sqlite3_context* db, const std::u16string& val) {
		sqlite3_result_text16(db, val.data(), -1, SQLITE_TRANSIENT);
	}

	// Other integer types
	template<class Integral>
	struct has_sqlite_type<Integral, SQLITE_INTEGER, typename std::enable_if<std::is_integral<Integral>::value>::type> : std::true_type {};

	template<class Integral, class = typename std::enable_if<std::is_integral<Integral>::value>::type>
	inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const Integral& val) {
		return bind_col_in_db(stmt, inx, static_cast<sqlite3_int64>(val));
	}
	template<class Integral, class = std::enable_if<std::is_integral<Integral>::type>>
	inline void store_result_in_db(sqlite3_context* db, const Integral& val) {
		store_result_in_db(db, static_cast<sqlite3_int64>(val));
	}
	template<class Integral, class = typename std::enable_if<std::is_integral<Integral>::value>::type>
	inline void get_col_from_db(sqlite3_stmt* stmt, int inx, Integral& val) {
		sqlite3_int64 i;
		get_col_from_db(stmt, inx, i);
		val = i;
	}
	template<class Integral, class = typename std::enable_if<std::is_integral<Integral>::value>::type>
	inline void get_val_from_db(sqlite3_value *value, Integral& val) {
		sqlite3_int64 i;
		get_val_from_db(value, i);
		val = i;
	}

	// vector<T, A>
	template<typename T, typename A>
	struct has_sqlite_type<std::vector<T, A>, SQLITE_BLOB, void> : std::true_type {};

	template<typename T, typename A> inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const std::vector<T, A>& vec) {
		void const* buf = reinterpret_cast<void const *>(vec.data());
		int bytes = vec.size() * sizeof(T);
		return sqlite3_bind_blob(stmt, inx, buf, bytes, SQLITE_TRANSIENT);
	}
	template<typename T, typename A> inline void store_result_in_db(sqlite3_context* db, const std::vector<T, A>& vec) {
		void const* buf = reinterpret_cast<void const *>(vec.data());
		int bytes = vec.size() * sizeof(T);
		sqlite3_result_blob(db, buf, bytes, SQLITE_TRANSIENT);
	}
	template<typename T, typename A> inline void get_col_from_db(sqlite3_stmt* stmt, int inx, std::vector<T, A>& vec) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			vec.clear();
		} else {
			int bytes = sqlite3_column_bytes(stmt, inx);
			T const* buf = reinterpret_cast<T const *>(sqlite3_column_blob(stmt, inx));
			vec = std::vector<T, A>(buf, buf + bytes/sizeof(T));
		}
	}
	template<typename T, typename A> inline void get_val_from_db(sqlite3_value *value, std::vector<T, A>& vec) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			vec.clear();
		} else {
			int bytes = sqlite3_value_bytes(value);
			T const* buf = reinterpret_cast<T const *>(sqlite3_value_blob(value));
			vec = std::vector<T, A>(buf, buf + bytes/sizeof(T));
		}
	}

	/* for unique_ptr<T> support */
	template<typename T, int Type>
	struct has_sqlite_type<std::unique_ptr<T>, Type, void> : has_sqlite_type<T, Type> {};
	template<typename T>
	struct has_sqlite_type<std::unique_ptr<T>, SQLITE_NULL, void> : std::true_type {};

	template<typename T> inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const std::unique_ptr<T>& val) {
		return val ? bind_col_in_db(stmt, inx, *val) : bind_col_in_db(stmt, inx, nullptr);
	}
	template<typename T> inline void get_col_from_db(sqlite3_stmt* stmt, int inx, std::unique_ptr<T>& _ptr_) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			_ptr_ = nullptr;
		} else {
			auto underling_ptr = new T();
			get_col_from_db(stmt, inx, *underling_ptr);
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

	// std::optional support for NULL values
#ifdef MODERN_SQLITE_STD_OPTIONAL_SUPPORT
#ifdef MODERN_SQLITE_EXPERIMENTAL_OPTIONAL_SUPPORT
	template<class T>
	using optional = std::experimental::optional<T>;
#else
	template<class T>
	using optional = std::optional<T>;
#endif

	template<typename T, int Type>
	struct has_sqlite_type<optional<T>, Type, void> : has_sqlite_type<T, Type> {};
	template<typename T>
	struct has_sqlite_type<optional<T>, SQLITE_NULL, void> : std::true_type {};

	template <typename OptionalT> inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const optional<OptionalT>& val) {
		return val ? bind_col_in_db(stmt, inx, *val) : bind_col_in_db(stmt, inx, nullptr);
	}
	template <typename OptionalT> inline void store_result_in_db(sqlite3_context* db, const optional<OptionalT>& val) {
		if(val)
			store_result_in_db(db, *val);
		else
			sqlite3_result_null(db);
	}

	template <typename OptionalT> inline void get_col_from_db(sqlite3_stmt* stmt, int inx, optional<OptionalT>& o) {
		if(sqlite3_column_type(stmt, inx) == SQLITE_NULL) {
			#ifdef MODERN_SQLITE_EXPERIMENTAL_OPTIONAL_SUPPORT
			o = std::experimental::nullopt;
			#else
			o.reset();
			#endif
		} else {
			OptionalT v;
			get_col_from_db(stmt, inx, v);
			o = std::move(v);
		}
	}
	template <typename OptionalT> inline void get_val_from_db(sqlite3_value *value, optional<OptionalT>& o) {
		if(sqlite3_value_type(value) == SQLITE_NULL) {
			#ifdef MODERN_SQLITE_EXPERIMENTAL_OPTIONAL_SUPPORT
			o = std::experimental::nullopt;
			#else
			o.reset();
			#endif
		} else {
			OptionalT v;
			get_val_from_db(value, v);
			o = std::move(v);
		}
	}
#endif

#ifdef MODERN_SQLITE_STD_VARIANT_SUPPORT
	namespace detail {
		template<class T, class U>
		struct tag_trait : U { using tag = T; };
	}

	template<int Type, class ...Options>
	struct has_sqlite_type<std::variant<Options...>, Type, void> : std::disjunction<detail::tag_trait<Options, has_sqlite_type<Options, Type>>...> {};

	namespace detail {
		template<int Type, typename ...Options, typename Callback, typename first_compatible = has_sqlite_type<std::variant<Options...>, Type>>
		inline void variant_select_type(Callback &&callback) {
			if constexpr(first_compatible::value)
				callback(typename first_compatible::tag());
			else
				throw errors::mismatch("The value is unsupported by this variant.", "", SQLITE_MISMATCH);
		}
		template<typename ...Options, typename Callback> inline void variant_select(int type, Callback &&callback) {
			switch(type) {
				case SQLITE_NULL:
					variant_select_type<SQLITE_NULL, Options...>(std::forward<Callback>(callback));
					break;
				case SQLITE_INTEGER:
					variant_select_type<SQLITE_INTEGER, Options...>(std::forward<Callback>(callback));
					break;
				case SQLITE_FLOAT:
					variant_select_type<SQLITE_FLOAT, Options...>(std::forward<Callback>(callback));
					break;
				case SQLITE_TEXT:
					variant_select_type<SQLITE_TEXT, Options...>(std::forward<Callback>(callback));
					break;
				case SQLITE_BLOB:
					variant_select_type<SQLITE_BLOB, Options...>(std::forward<Callback>(callback));
					break;
				default:;
			}
		}
	}
	template <typename ...Args> inline int bind_col_in_db(sqlite3_stmt* stmt, int inx, const std::variant<Args...>& val) {
		return std::visit([&](auto &&opt) {return bind_col_in_db(stmt, inx, std::forward<decltype(opt)>(opt));}, val);
	}
	template <typename ...Args> inline void store_result_in_db(sqlite3_context* db, const std::variant<Args...>& val) {
		std::visit([&](auto &&opt) {store_result_in_db(db, std::forward<decltype(opt)>(opt));}, val);
	}
	template <typename ...Args> inline void get_col_from_db(sqlite3_stmt* stmt, int inx, std::variant<Args...>& val) {
		detail::variant_select<Args...>(sqlite3_column_type(stmt, inx), [&](auto v) {
			get_col_from_db(stmt, inx, v);
			val = std::move(v);
		});
	}
	template <typename ...Args> inline void get_val_from_db(sqlite3_value *value, std::variant<Args...>& val) {
		detail::variant_select<Args...>(sqlite3_value_type(value), [&](auto v) {
			get_val_from_db(value, v);
			val = std::move(v);
		});
	}
#endif
}
