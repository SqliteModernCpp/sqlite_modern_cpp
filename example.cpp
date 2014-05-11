#include<iostream>
#include "sqlite_modern_cpp.h"
using namespace  sqlite;
using namespace std;

/*
template <typename T>
struct function_traits
: public function_traits<decltype(&T::operator())>
{};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
enum { arity = sizeof...(Args) };
// arity is the number of arguments.

typedef ReturnType result_type;

template <size_t i>
struct arg
{
typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
// the i-th argument is equivalent to the i-th tuple element of a tuple
// composed of those arguments.
};
};

class database_bind {};

template<int N>
class A {
template<typename F>
static void run(F l);
};

template<>
struct A<1> {
template<typename F>
static void run(F l) {
typedef function_traits<decltype(l)> traits;
typedef typename traits::arg<0>::type type_1;

type_1 col_1;
get_from_db(0,col_1);

l(col_1);
}
};
template<>
struct A<2> {
template<typename F>
static void run(F l) {
typedef function_traits<decltype(l)> traits;
typedef typename traits::arg<0>::type type_1;
typedef typename traits::arg<1>::type type_2;

type_1 col_1;
type_2 col_2;
get_from_db(0,col_1);
get_from_db(1,col_2);

l(col_1, col_2);
}
};


void get_from_db(int col_inx, string& str){
// code to get a column from with col_inx from the database
// for simplicity
str = "str_col";
}
void get_from_db(int col_inx, int& i){
// just for simplicity
i = 12;
}


template<typename F>
void operator>>(database_bind dbb, F l)
{
typedef function_traits<decltype(l)> traits;
A<traits::arity>::run(l);
}

*/

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