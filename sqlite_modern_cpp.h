#pragma once
#include<string>
#include<functional>
#include<stdexcept>
#include"sqlite3.h"


namespace sqlite {

	using namespace std;
	class database {
	private:
		sqlite3 * _db;
		bool _connected;
	public:
		class database_binder {
		private:
			sqlite3 * _db;
			wstring _sql;
			sqlite3_stmt* _stmt;
			int _inx;
			template<typename T> class type_call { };

			void _extract(function<void(void)> call_back){
				int hresult;
				while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW)
					call_back();

				if (hresult != SQLITE_DONE)
					throw exception(sqlite3_errmsg(_db));

				if (sqlite3_finalize(_stmt) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				_stmt = nullptr;
			}
			void _extract_single_value(function<void(void)> call_back){
				int hresult;
				if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW)
					call_back();

				if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW)
					throw exception("not every row extracted");

				if (hresult != SQLITE_DONE)
					throw exception(sqlite3_errmsg(_db));

				if (sqlite3_finalize(_stmt) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				_stmt = nullptr;
			}
			void _prepare(){
				if (sqlite3_prepare16_v2(_db, _sql.data(), -1, &_stmt, nullptr) != SQLITE_OK)
					throw std::exception(sqlite3_errmsg(_db));
			}
		protected:
			database_binder(sqlite3 * db, wstring const & sql) :
				_db(db),
				_sql(sql),
				_stmt(nullptr),
				_inx(1)
			{
				_prepare();
			}

			database_binder(sqlite3 * db, string const & sql) :
				_db(db),
				_sql(sql.begin(), sql.end()),
				_stmt(nullptr),
				_inx(1)
			{
				_prepare();
			}

		public:
			friend class database;
			~database_binder(){
				if (_stmt){
					int hresult;
					if ((hresult = sqlite3_step(_stmt)) != SQLITE_DONE){
						if (hresult == SQLITE_ROW)
							throw exception("not all rows readed");
						throw exception(sqlite3_errmsg(_db));
					}
					if (sqlite3_finalize(_stmt) != SQLITE_OK)
						throw exception(sqlite3_errmsg(_db));
					_stmt = nullptr;
				}
			}
#pragma region operator <<

			database_binder& operator <<(double val) {
				if (sqlite3_bind_double(_stmt, _inx, val) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				++_inx;
				return *this;
			}

			database_binder& operator <<(int val) {
				if (sqlite3_bind_int(_stmt, _inx, val) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				++_inx;
				return *this;
			}

			database_binder& operator <<(sqlite_int64 val) {
				if (sqlite3_bind_int64(_stmt, _inx, val) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				++_inx;
				return *this;
			}

			database_binder& operator <<(string const& txt) {
				if (sqlite3_bind_text(_stmt, _inx, txt.data(), -1, SQLITE_STATIC) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				++_inx;
				return *this;
			}

			database_binder& operator <<(wstring const& txt) {
				if (sqlite3_bind_text16(_stmt, _inx, txt.data(), -1, SQLITE_STATIC) != SQLITE_OK)
					throw exception(sqlite3_errmsg(_db));
				++_inx;
				return *this;
			}
#pragma endregion

#pragma region get_*

			string get_string(int i){
				if (sqlite3_column_type(_stmt, i) == SQLITE_NULL) return string();
				sqlite3_column_bytes(_stmt, i);
				return (char*)sqlite3_column_text(_stmt, i);
			}
			wstring get_wstring(int i){
				if (sqlite3_column_type(_stmt, i) == SQLITE_NULL) return wstring();
				sqlite3_column_bytes16(_stmt, i);
				return (wchar_t *)sqlite3_column_text16(_stmt, i);
			}

			int get_int(int i){
				if (sqlite3_column_type(_stmt, i) == SQLITE_NULL) return 0;
				return sqlite3_column_int(_stmt, i);
			}

			double get_double(int i){
				if (sqlite3_column_type(_stmt, i) == SQLITE_NULL)
					return 0;
				return sqlite3_column_double(_stmt, i);
			}
			sqlite3_int64 get_int64(int i){
				if (sqlite3_column_type(_stmt, i) == SQLITE_NULL)
					return 0;
				return sqlite3_column_int64(_stmt, i);
			}


			string get_col(int i, type_call<string const &> tc){ return get_string(i); }
			string get_col(int i, type_call<string &> tc){ return get_string(i); }
			string get_col(int i, type_call<string &&> tc){ return get_string(i); }
			string get_col(int i, type_call<string> tc){ return get_string(i); }

			wstring get_col(int i, type_call<wstring const &> tc){ return get_wstring(i); }
			wstring get_col(int i, type_call<wstring &> tc){ return get_wstring(i); }
			wstring get_col(int i, type_call<wstring &&> tc){ return get_wstring(i); }
			wstring get_col(int i, type_call<wstring> tc){ return get_wstring(i); }

			int get_col(int i, type_call<int> tc){ return get_int(i); }
			double get_col(int i, type_call<double> tc){ return get_double(i); }
			sqlite3_int64 get_col(int i, type_call<sqlite3_int64> tc){ return get_int64(i); }
#pragma endregion

#pragma region operator >>

			void operator>>(int & val){
				_extract_single_value([&]{
					val = get_col(0, type_call<int>());
				});
			}
			void operator>>(string & val){
				_extract_single_value([&]{
					val = get_col(0, type_call<string>());
				});
			}
			void operator>>(wstring & val){
				_extract_single_value([&]{
					val = get_col(0, type_call<wstring>());
				});
			}
			void operator>>(double & val){
				_extract_single_value([&]{
					val = get_col(0, type_call<double>());
				});
			}
			void operator>>(sqlite3_int64 & val){
				_extract_single_value([&]{
					val = get_col(0, type_call<sqlite3_int64>());
				});
			}


			template<typename T>
			void operator>>(function<void(T)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T>())
						);
				});
			}

			template<typename T1, typename T2>
			void operator>>(function<void(T1, T2)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>())
						);
				});
			}

			template<typename T1, typename T2, typename T3>
			void operator>>(function<void(T1, T2, T3)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4>
			void operator>>(function<void(T1, T2, T3, T4)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5>
			void operator>>(function<void(T1, T2, T3, T4, T5)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>()),
						get_col(9, type_call<T10>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>()),
						get_col(9, type_call<T10>()),
						get_col(10, type_call<T11>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>()),
						get_col(9, type_call<T10>()),
						get_col(10, type_call<T11>()),
						get_col(11, type_call<T12>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>()),
						get_col(9, type_call<T10>()),
						get_col(10, type_call<T11>()),
						get_col(11, type_call<T12>()),
						get_col(12, type_call<T13>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>()),
						get_col(9, type_call<T10>()),
						get_col(10, type_call<T11>()),
						get_col(11, type_call<T12>()),
						get_col(12, type_call<T13>()),
						get_col(13, type_call<T14>())
						);
				});
			}

			template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
			void operator>>(function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15)> call_back){
				_extract([&]{
					call_back(
						get_col(0, type_call<T1>()),
						get_col(1, type_call<T2>()),
						get_col(2, type_call<T3>()),
						get_col(3, type_call<T4>()),
						get_col(4, type_call<T5>()),
						get_col(5, type_call<T6>()),
						get_col(6, type_call<T7>()),
						get_col(7, type_call<T8>()),
						get_col(8, type_call<T9>()),
						get_col(9, type_call<T10>()),
						get_col(10, type_call<T11>()),
						get_col(11, type_call<T12>()),
						get_col(12, type_call<T13>()),
						get_col(13, type_call<T14>()),
						get_col(14, type_call<T15>())
						);
				});
			}
#pragma endregion
		};

		database(wstring const & db_name) : _db(nullptr), _connected(false) {
			_connected = sqlite3_open16(db_name.data(), &_db) == SQLITE_OK;
		}
		database(string const & db_name) : _db(nullptr), _connected(false) {
			_connected = sqlite3_open16(wstring(db_name.begin(), db_name.end()).data(), &_db) == SQLITE_OK;
		}

		~database(){
			if (_db){
				sqlite3_close_v2(_db);
				_db = nullptr;
			}
		}

		database_binder operator<<(string const& sql) const{
			return database_binder(_db, sql);
		}
		database_binder operator<<(wstring const& sql) const{
			return database_binder(_db, sql);
		}

		operator bool() const{
			return _connected;
		}
		operator string() {
			return sqlite3_errmsg(_db);
		}
		operator wstring() {
			return (wchar_t*)sqlite3_errmsg16(_db);
		}
	};

#define _sqlitepp
#define _sqliteppdouble ,double
#define _sqliteppfloat ,float
#define _sqliteppint ,int
#define _sqlitepplong ,long
#define _sqliteppconst ,const
#define _sqliteppstring ,string
#define _sqliteppwstring ,wstring
#define _sqliteppexpand(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) a1 _sqlitepp##a2 _sqlitepp##a3 _sqlitepp##a4 _sqlitepp##a5 _sqlitepp##a6 _sqlitepp##a7 _sqlitepp##a8 _sqlitepp##a9 _sqlitepp##a10 _sqlitepp##a11 _sqlitepp##a12 _sqlitepp##a13 _sqlitepp##a14 _sqlitepp##a15 
#define magic_mapper(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) (function<void(_sqliteppexpand(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15))>)[&](_sqliteppexpand(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15))

}
