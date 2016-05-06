#include <iostream>
#include <string>
#include <sqlite_modern_cpp.h>
#include <sqlite_modern_cpp/extensions/null_support.h>

int main() {

	try {
		database db(":memory:");
		db << "CREATE TABLE tbl (id integer, name string);";
		db << "INSERT INTO tbl VALUES (?, ?);" << 1 << "hello";
		db << "INSERT INTO tbl VALUES (?, ?);" << 2 << null_value();

		db << "select id,name from tbl where id = 1" >> [](int id, null_value name_null) {
			if(name_null) exit(EXIT_FAILURE);
		};

		db << "select id,name from tbl where id = 2" >> [](int id, null_value name_null) {
			if(!name_null) exit(EXIT_FAILURE);
		};


	} catch(sqlite_exception e) {
		cout << "Sqlite error " << e.what() << endl;
		exit(EXIT_FAILURE);
	} catch(...) {
		cout << "Unknown error\n";
		exit(EXIT_FAILURE);
	}

	cout << "OK\n";
	exit(EXIT_SUCCESS);
}
