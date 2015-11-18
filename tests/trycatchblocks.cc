#include <iostream>
#include <string>
#include <sqlite_modern_cpp.h>

using namespace sqlite;
using std::string;

class DBInterface {
    database db;

public:
    DBInterface( void ) : db( "log.db" )
    {
    }

    void LogRequest( const string& username, const string& ip, const string& request ) 
    {
        try {
            auto timestamp = std::to_string( time( nullptr ) );

            db  <<
                "create table if not exists log_request ("
                "   _id integer primary key autoincrement not null,"
                "   username text,"
                "   timestamp text,"
                "   ip text,"
                "   request text"
                ");";
            db  << "INSERT INTO log_request (username, timestamp, ip, request) VALUES (?,?,?,?);" 
                << username
                << timestamp
                << ip
                << request;
        } catch ( const std::exception& e ) {
            std::cout << e.what() << std::endl;
        }
    }

    bool TestData( void ) {
        try {
            string username, timestamp, ip, request;

            db  << "select username, timestamp, ip, request from log_request where name = ?"
                << "test"
                >> std::tie(username, timestamp, ip, request);
                    
            if ( username == "test" && ip == "127.0.0.1" && request == "hello world" ) {
                return true;
            }
        } catch ( const std::exception& e ) {
            std::cout << e.what() << std::endl;
        }

        return false;
    }
};

int main( void ) 
{
    // --------------------------------------------------------------------------
    // -- Test if writing to disk works properly from within a catch block.
    // --------------------------------------------------------------------------
    try {
        throw "hello";
    }
    catch ( ... ) {
        DBInterface interf;
        interf.LogRequest( "test", "127.0.0.1", "hello world" );
        if ( !interf.TestData() ) {
            exit( EXIT_FAILURE );
        }
    }

    exit( EXIT_SUCCESS );
}