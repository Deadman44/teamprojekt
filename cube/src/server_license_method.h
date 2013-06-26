
#include <sstream>

class serverToLicenseSystem
{
public:
    std::stringstream ss;	// Hier wird die vollstaendige Antwort des Servers gespeichert
    std::string response;	// Hier wird der HTTP Body der Antwort gespeichert
	std::string clientTicket;
		
	serverToLicenseSystem(boost::asio::io_service& io_service,
      boost::asio::ssl::context& context,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context)
  {

  }
  // Zertifikat 
  bool verify_certificate(bool preverified,
      boost::asio::ssl::verify_context& ctx)
  {
  }

  void handle_connect(const boost::system::error_code& error)
  {
    
  }

  void handle_handshake(const boost::system::error_code& error)
  {
   
  }

  void handle_write(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
 
  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
 
  }


std::string urlencode(const std::string &c)
{
    
   
}

std::string char2hex( char dec )
{
   
}

private:
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  char request_;
  char reply_;
  std::string license;

  
};