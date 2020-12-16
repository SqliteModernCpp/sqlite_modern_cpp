
#include "collectors.h"
#include <list>
#include <algorithm>

using namespace std;
using namespace sqlite;

struct user
{
	int id;
	string name;

	user(int _id, string _name):id(_id),name(_name)
	{
		clog<<"object created : "<<*this<<"\n";
	}

	friend std::ostream& operator <<(std::ostream& out, const user& a)
	{
		out<<"<"<<a.id<<","<<a.name<<">";
		return out;
	}
};


database init_db()
{
	database db(":memory:");
	db << "CREATE TABLE tbl (id integer, name string);";
	db << "INSERT INTO tbl VALUES (?, ?);" << 1 << "hello";
	db << "INSERT INTO tbl VALUES (?, ?);" << 2 << "world";

	return db;
}


void test_single_user(database &db)
{
	auto may_be_user = db<<"SELECT * FROM tbl WHERE id = 1 ;"
						>> collect<int,string>::as<shared_ptr,user>();
	assert(may_be_user);
	assert(may_be_user->name == "hello");
}

void test_multiple_user(database &db)
{
	auto all_users = db<<"SELECT * FROM tbl ;"
						>> collect<int,string>::as<list,user>();
	assert(not all_users.empty());
	assert(any_of(all_users.begin(), all_users.end(), [](user u)
	{
		return u.name == "hello";
	}));
}

void test_multiple_usernames(database &db)
{
	auto names = db<<"SELECT name FROM tbl ORDER BY id DESC;"
						>> collect<string>::as<vector,string>();
	assert(not names.empty());
	assert(names[0] == "world");
	assert(names[1] == "hello");
}

void test_append_results_to_container(database &db)
{
	list<int> ids = {-1,0,11};

	db<<"SELECT id FROM tbl ;"
		>>collect<int>::to(ids);

	list<int> expected = { -1,0,11, 1, 2};
	assert(ids == expected);
}

int main()
{
	database db=init_db();
	
	test_single_user(db);
	test_multiple_user(db);
	test_multiple_usernames(db);
	
	test_append_results_to_container(db);
	
	return 0;
}
