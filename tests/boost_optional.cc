#include <iostream>

#define _MODERN_SQLITE_BOOST_OPTIONAL_SUPPORT
#include <sqlite_modern_cpp.h>

using namespace sqlite;
using namespace std;

void insert(database& db, bool is_null) {
	int id = 1;
	boost::optional<int> val;
	if(!is_null) val = 5;

	db << "delete from test where id = 1";
	db << "insert into test(id,val) values(?,?)" << id << val;
}

void select(database& db, bool should_be_null) {
	db << "select id,val from test" >> [&](long long id, boost::optional<int> val) {
		id = id;
		if(should_be_null) {
			if(val) exit(EXIT_FAILURE);
		} else {
			if(!val) exit(EXIT_FAILURE);
		}
	};
}

struct TmpFile {
	string fname;

	TmpFile() {
		char f[] = "/tmp/sqlite_modern_cpp_test_XXXXXX";
		int fid = mkstemp(f);
		close(fid);

		fname = f;
	}

	~TmpFile() {
		unlink(fname.c_str());
	}
};

int main() {
	try {
		// creates a database file 'dbfile.db' if it does not exists.
		TmpFile file;
		database db(file.fname);

		db << "drop table if exists test";
		db <<
			"create table if not exists test ("
			"   id integer primary key,"
			"   val int"
			");";

		insert(db, true);
		select(db, true);

		insert(db, false);
		select(db, false);

	} catch(exception& e) {
		cout << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
