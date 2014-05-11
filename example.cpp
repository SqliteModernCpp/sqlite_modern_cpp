#include<iostream>
#include "sqlite_modern_cpp.h"
using namespace  sqlite;
using namespace std;

int main(){
	try {
		// creates a database file 'dbfile.db' if not exists
		database db("dbfile.db");

		// executes the query and creates a 'user' table
		db <<
			"create table user ("
			"	age int,"
			"	name text,"
			"	weight real"
			");";

		// inserts a new user and binds the values to ?
		// note that only types allowed for bindings are 
		//		1 - numeric types( int ,long ,long long, float, double , ... )
		//		2 - string , wstring
		db << "insert into user (age,name,weight) values (?,?,?);"
			<< 20
			<< "bob"
			<< 83.0;

		db << "insert into user (age,name,weight) values (?,?,?);"
			<< 21
			<< L"jak"
			<< 68.5;

		// slects from table user on a condition ( age > 18 ) and executes 
		// the body of magid_mapper for every row returned .
		// node : magic_mapper is just a simple macro , the next sample is
		// equivalent to this one without the use of magic_mapper macro
		db << "select age,name,weight from user where age > ? ;"
			<< 18
			>> magic_mapper(int age, string name, double weight) {
				cout << age << ' ' << name << ' ' << weight << endl;
			};

		db << "select age,name,weight from user where age > ? ;"
			<< 18
			>> function<void(int,string,double)>([&](int age, string name, double weight) {
				cout << age << ' ' << name << ' ' << weight << endl;
			});

		// i am currently working on a solution to avoid magic mapper
		// i future i want to this syntax also work 
		/*
			db << "select age,name,weight from user where age > ? ;"
				<< 18
				>> [&](int age, string name, double weight) {
					cout << age << ' ' << name << ' ' << weight << endl;
				};
		*/

	}
	catch (exception& e){
		cout << e.what() << endl;
	}
}