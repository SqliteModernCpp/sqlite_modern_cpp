#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <stdexcept>
#include <sqlite_modern_cpp.h>
#include <sqlite_modern_cpp/log.h>
#include <catch.hpp>
using namespace sqlite;
using namespace std;

TEST_CASE("error_log works", "[log]") {
  bool error_detected = false;
  error_log(
  	[&](errors::constraint e) {
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
}
