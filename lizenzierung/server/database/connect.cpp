#include <iostream>
#include <driver/mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

int main(void) {
	cout << endl;
	cout << "Running 'SELECT 'Hello World!' AS _message'" << endl;

	try {
	  sql::Driver *driver;
	  sql::Connection *con;
	  sql::Statement *stmt;
	  sql::ResultSet *res;
	  sql::PreparedStatement *prep_stmt;
	  string server = "tcp://127.0.0.1:3306";
	  string user = "root";
	  string password = "";
	  string hserial="26608804100996bbaf61abc4c8418482eacaa4d4564fdaf433f75dac2ff99a6b1096f49d6";

	  /* Create a connection */
	  driver = get_driver_instance();
	  con = driver->connect(server,user,password);
	  /* Connect to the MySQL test database */
	  con->setSchema("test");

	  //stmt = con->createStatement();
	  prep_stmt = con->prepareStatement("SELECT SKEY FROM user WHERE HSERIAL=? LIMIT 1");
	  prep_stmt->setString(1, hserial);
	  res = prep_stmt->executeQuery();
	  
	  while (res->next()) {
	    cout << "\t... MySQL replies: ";
	    /* Access column data by alias or column name */
	    cout << res->getString("SKEY") << endl;
	    cout << "\t... MySQL says it again: ";
	    /* Access column fata by numeric offset, 1 is the first column */
	    cout << res->getString(1) << endl;
	  }
	  delete res;
	  delete stmt;
	  delete con;

	} catch (sql::SQLException &e) {
	  cout << "# ERR: SQLException in " << __FILE__;
	  cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	  cout << "# ERR: " << e.what();
	  cout << " (MySQL error code: " << e.getErrorCode();
	  cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	cout << endl;
	return 0;
}