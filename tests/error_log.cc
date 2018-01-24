#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <stdexcept>
#include <catch.hpp>
#include <sqlite_modern_cpp.h>
#include <sqlite_modern_cpp/log.h>
using namespace sqlite;
using namespace std;

TEST_CASE("error_log works with multiple handlers", "[log]") {
  bool error_detected = false;
  error_log(
  	[&](errors::constraint) {
    	cerr << "Wrong error detected!" << endl;
  	},
  	[&](errors::constraint_primarykey e) {
    	cerr << e.get_code() << '/' << e.get_extended_code() << ": " << e.what() << endl;
    	error_detected = true;
  	}
  );
  database db(":memory:");
  db << "CREATE TABLE person (id integer primary key not null, name TEXT);";

  try {
    db << "INSERT INTO person (id,name) VALUES (?,?)" << 1 << "jack";
    // inserting again to produce error
    db << "INSERT INTO person (id,name) VALUES (?,?)" << 1 << "jack";
  } catch (errors::constraint& e) { }

  REQUIRE(error_detected);
}
