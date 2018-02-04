#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <sqlite_modern_cpp.h>
#include <catch.hpp>
using namespace sqlite;
using namespace std;

struct A {
	~A() {
		database db(":memory:");
		db << "CREATE TABLE person (id integer primary key not null, name TEXT not null);";

		try {
			auto stmt = db << "INSERT INTO person (id,name) VALUES (?,?)";
			throw 1;
		} catch (int) {
		}
	}
};

TEST_CASE("Nested prepered statements wont execute", "[nested_prepared_statements]") {
#ifdef __cpp_lib_uncaught_exceptions
	try {
		A a;
		throw 1;
	} catch(int) { }
#else
#endif
}
