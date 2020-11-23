#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sqlite_modern_cpp.h>
using namespace  sqlite;
using namespace std;

int main()
{
	try
	{
		database db(":memory:");

		db << "CREATE TABLE foo (a integer, b string);\n";
		db << "INSERT INTO foo VALUES (?, ?)" << 1 << "hello";
		db << "INSERT INTO foo VALUES (?, ?)" << 2 << "world";

		string str;
		db << "SELECT b from FOO where a=?;" << 2L >> str;

		if(str != "world")
		{
			cout << "Bad result on line " << __LINE__ << endl;
			exit(EXIT_FAILURE);
		}

		std::string sql("select 1+1");
		long test = 0;
		db << sql >> test;
		
		if(test != 2) exit(EXIT_FAILURE);
		
	}
	catch(const sqlite_exception& e)
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
