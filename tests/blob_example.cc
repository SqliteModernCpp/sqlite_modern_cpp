#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sqlite_modern_cpp.h>
using namespace  sqlite;
using namespace std;

int main()
{
	try
	{
		database db(":memory:");

		db << "CREATE TABLE person (name TEXT, numbers BLOB);";
		db << "INSERT INTO person VALUES (?, ?)" << "bob" << blob(vector<int> { 1, 2, 3});
		db << "INSERT INTO person VALUES (?, ?)" << "jack" << blob(vector<char> { '1', '2', '3'});
		db << "INSERT INTO person VALUES (?, ?)" << "sara" << blob(vector<double> { 1.0, 2.0, 3.0});

		// Extract to lvalue blob
		vector<int> numbers_bob;
		db << "SELECT numbers from person where name = ?;" << "bob" >> blob(numbers_bob);


		if(numbers_bob.size() != 3 || numbers_bob[0] != 1 || numbers_bob[1] != 2 || numbers_bob[2] != 3) {
			cout << "Bad result on line " << __LINE__ << endl;
			exit(EXIT_FAILURE);
		}

		// Extract to lambda blob parameter
		vector<char> numbers_jack;
		db << "SELECT numbers from person where name = ?;" << "jack" >> [](blob_t<vector<char>> numbers_jack) {
			if(numbers_jack.vec.size() != 3 || numbers_jack.vec[0] != '1'
	        || numbers_jack.vec[1] != '2' || numbers_jack.vec[2] != '3') {
				cout << "Bad result on line " << __LINE__ << endl;
				exit(EXIT_FAILURE);
			}
		};

		// Extract to rvalue blob 
		auto numbers_sara = db << "SELECT numbers from person where name = ?;" << "sara" >> blob(vector<double>());

		if(numbers_sara.size() != 3 || numbers_sara[0] != 1 || numbers_sara[1] != 2 || numbers_sara[2] != 3) {
			cout << "Bad result on line " << __LINE__ << endl;
			exit(EXIT_FAILURE);
		}
	}
	catch(sqlite_exception e)
	{
		cout << "Unexpected error " << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	catch(...)
	{
		cout << "Unknown error\n";
		exit(EXIT_FAILURE);
	}

	cout << "OK\n";
	exit(EXIT_SUCCESS);
}
