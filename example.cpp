#include<iostream>
#include<string>
#include<tuple>
#include "sqlite_modern_cpp.h"
using namespace  sqlite;
using namespace std;


int main(){
	try {
		// creates a database file 'dbfile.db' if not exists
		database db("dbfile.db");

		// executes the query and creates a 'user' table
		db <<
			"create table if not exists user ("
			"	age int,"
			"	name text,"
			"	weight real"
			");";

		// inserts a new user and binds the values to ?
		// note that only types allowed for bindings are :
		//		int ,long, long long, float, double
		//		string , wstring
		db << "insert into user (age,name,weight) values (?,?,?);"
			<< 20
			<< "bob"
			<< 83.0;

		db << "insert into user (age,name,weight) values (?,?,?);"
			<< 21
			<< L"jack"
			<< 68.5;

		// slects from table user on a condition ( age > 18 ) and executes
		// the lambda for every row returned .

		db << "select age,name,weight from user where age > ? ;"
			<< 18
			>> [&](int age, string name, double weight) {
			cout << age << ' ' << name << ' ' << weight << endl;
		};

		// selects the count(*) of table user
		// note that you can extract a single culumn single row answer only to : int,long,long,float,double,string,wstring
		int count = 0;
		db << "select count(*) from user" >> count;
		cout << "cout : " << count << endl;

		// this also works and the returned value will automatically converted to string
		string scount;
		db << "select count(*) from user" >> scount;
		cout << "scount : " << scount << endl;
	}
	catch (exception& e){
		cout << e.what() << endl;
	}
}