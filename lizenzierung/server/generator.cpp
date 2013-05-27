#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include <iostream> 
#include <string> 

boost::asio::io_service io_service; 
boost::asio::ip::tcp::resolver resolver(io_service); 
boost::asio::ip::tcp::socket sock(io_service); 
boost::array<char, 4096> buffer;
std::vector<std::string> lines;

std::stringstream ss;	// Hier wird die vollstaendige Antwort des Servers gespeichert
std::string response;	// Hier wird der HTTP Body der Antwort gespeichert

void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) 
{ 
  if (!ec) 
  { 
	
	ss << std::string(buffer.data(), bytes_transferred);
	char del = '\n';
	std::string item;
	bool headerEnd = false;
	while(std::getline(ss, item, del)) {
		std::cout << item << std::endl;
		if(headerEnd and item != "\r") {
			response.append(item);
			response.append("\n");
		}
		// HTTP Body beginnt mit nächster Zeile
		if(item.compare("Content-Type: text/html\r")==0)
			headerEnd = true;
		
	}
	sock.async_read_some(boost::asio::buffer(buffer), read_handler); 
  } 
} 

void connect_handler(const boost::system::error_code &ec) 
{ 
  if (!ec) 
  { 
    //boost::asio::write(sock, boost::asio::buffer("GET /~markus/cube_license_frontend/cCheck_License_Key.php HTTP 1.1\r\nHost: localhost\r\n\r\n")); 
	  boost::asio::write(sock, boost::asio::buffer("GET / HTTP 1.1\r\nHost: google.de\r\n\r\n")); 
    sock.async_read_some(boost::asio::buffer(buffer), read_handler); 
  } 
} 

void resolve_handler(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it) 
{ 
  if (!ec) 
  { 
    sock.async_connect(*it, connect_handler); 
  } 
} 

int main() 
{ 
  boost::asio::ip::tcp::resolver::query query("www.google.de", "80"); 
  resolver.async_resolve(query, resolve_handler); 
  io_service.run();
  std::cout << response;
} 