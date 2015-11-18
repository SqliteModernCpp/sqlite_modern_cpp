#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sqlite_modern_cpp.h>

using namespace sqlite;
using std::string;

struct TmpFile
{
    string fname;

    TmpFile()
    {
        char f[]="/tmp/sqlite_modern_cpp_test_XXXXXX";
        int fid = mkstemp(f);
        close(fid);

        fname = f;
    }

    ~TmpFile()
    {
        unlink(fname.c_str());
    }
};


class DBInterface {
    database db;

public:
    DBInterface( const string& fileName ) : db( fileName )
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

            db  << "select username, timestamp, ip, request from log_request where username = ?"
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
        TmpFile tmpF;
        DBInterface interf( tmpF.fname );
        interf.LogRequest( "test", "127.0.0.1", "hello world" );
        if ( !interf.TestData() ) {
            exit( EXIT_FAILURE );
        }
    }

    exit( EXIT_SUCCESS );
}