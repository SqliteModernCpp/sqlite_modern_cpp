#include <iostream>
#include <cstdlib>
#include <sqlite_modern_cpp.h>
#include <catch.hpp>
using namespace sqlite;
using namespace std;

TEST_CASE("binding named parameters works", "[named]") {
    database db(":memory:");

    db << "CREATE TABLE foo (a,b);";

    int a = 1;
    db << "INSERT INTO foo VALUES (:first,:second)" << ":second"_sqlparam(2) << ":first"_sqlparam(a);

    db << "SELECT b FROM foo WHERE a=?;" << 1 >> a;

    REQUIRE(a == 2);
}
