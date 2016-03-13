#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sqlite_modern_cpp.h>
using namespace  sqlite;
using namespace std;

// Add more tests!
int main() {

	{ // Test uint64_t support
		sqlite::database db(":memory:");

		db << "create table test (value integer)";

		uint64_t value = std::numeric_limits<uint64_t>::max();

		db << "insert into test(value) values(?)" << value;

		uint64_t ret = 0;

		db << "select value from test limit 1" >> ret;

		if(ret != std::numeric_limits<uint64_t>::max()) { exit(EXIT_FAILURE); }
	}

	exit(EXIT_SUCCESS);
}
