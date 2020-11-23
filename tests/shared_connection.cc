#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sqlite_modern_cpp.h>

using namespace  sqlite;
using namespace std;

int main() {
	try {

		database db(":memory:");

		{

			auto con = db.connection();

			{
				database db2(con);
				int test = 0;
				db2 << "select 1" >> test;
				if(test != 1) exit(EXIT_FAILURE);
			}

			int test = 0;
			db << "select 1" >> test;
			if(test != 1) exit(EXIT_FAILURE);

		}


	} catch(const sqlite_exception& e) {
		cout << "Unexpected error " << e.what() << endl;
		exit(EXIT_FAILURE);
	} catch(...) {
		cout << "Unknown error\n";
		exit(EXIT_FAILURE);
	}

	cout << "OK\n";
	exit(EXIT_SUCCESS);
}
