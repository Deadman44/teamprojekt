#include "cube.h"


enum { max_length = 1024};

class client
{
public:
    std::stringstream ss;	// Hier wird die vollstaendige Antwort des Servers gespeichert
    std::string response;	// Hier wird der HTTP Body der Antwort gespeichert
	std::string user;
	std::string password;

  client(boost::asio::io_service& io_service,
      boost::asio::ssl::context& context,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context)
  {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
        boost::bind(&client::verify_certificate, this, _1, _2));

    boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
        boost::bind(&client::handle_connect, this,
          boost::asio::placeholders::error));
  }

  bool verify_certificate(bool preverified,
      boost::asio::ssl::verify_context& ctx)
  {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }

  void handle_connect(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_handshake(boost::asio::ssl::stream_base::client,
          boost::bind(&client::handle_handshake, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Connect failed: " << error.message() << "\n";
    }
  }

  void handle_handshake(const boost::system::error_code& error)
  {
    if (!error)
    {
      //std::cout << "Enter message: ";
		//std::cin.getline(request_, max_length);
      //size_t request_length = strlen(request_);
	  size_t request_length = strlen(request_);
      boost::asio::async_write(socket_,
          boost::asio::buffer("GET /cCheck_License_Key.php?email=" + user + "&pass=" + password + "&license=" + license + " HTTP/1.1\r\nHost: localhost\r\n\r\n"),
          boost::bind(&client::handle_write, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
	  //std::cout << "GET /cCheck_License_Key.php?email=" + user + "&pass=" + password + "&license=" + license + " HTTP/1.1\r\nHost: localhost\r\n\r\n" << std::endl;
    }
    else
    {
      std::cout << "Handshake failed: " << error.message() << "\n";
    }
  }

  void handle_write(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
     /* boost::asio::async_read(socket_,
          boost::asio::buffer(reply_, bytes_transferred),
          boost::bind(&client::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
		
	     boost::asio::async_read(socket_,
		  boost::asio::buffer(reply_, bytes_transferred),
	            boost::bind(&client::handle_read, this,
	              boost::asio::placeholders::error,
				  boost::asio::placeholders::bytes_transferred));
		*/
		socket_.async_read_some(boost::asio::buffer(reply_, bytes_transferred),boost::bind(&client::handle_read, this,
               boost::asio::placeholders::error,
               boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      std::cout << "Write failed: " << error.message() << "\n";
    }
  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
		//std::cout << "Reply" << bytes_transferred << ": ";
		//std::cout.write(reply_, bytes_transferred);
		//std::cout << "\n";
		ss << std::string(reply_, bytes_transferred);
      socket_.async_read_some(boost::asio::buffer(reply_, max_length),
          boost::bind(&client::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
	else if (0 != bytes_transferred)
	       {
	           std::cout <<"\n" << "Quit";
	       }
		   /*
    else
    {
      std::cout << "Read failed: " << error.message() << "\n";
    }
		   */
	//handle_write(error, bytes_transferred);
  }

  void setLicense(std::string l) {
	  license = urlencode(l);
	  //std::cout << license << std::endl;
  }

std::string urlencode(const std::string &c)
{
    
    std::string escaped="";
    int max = c.length();
    for(int i=0; i<max; i++)
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//abc...xyz
             (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
             (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
        )
        {
            escaped.append( &c[i], 1);
        }
        else
        {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
    return escaped;
}

std::string char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    std::string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

private:
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  char request_[max_length];
  char reply_[max_length];
  std::string license;

  
};

int check_license(std::string u, std::string p, std::string l)
{
  try
  {

    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query("localhost","443");
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
	ctx.load_verify_file("server.crt");

    client c(io_service, ctx, iterator);
	c.user = u;
	c.password = p;
	c.setLicense(l);

    io_service.run();
	char del = '\n';
	std::string item;
	//std::cout << "#####" << std::endl;
	//std::cout << c.ss.str() << std::endl;
	bool headerEnd = false;
	while(std::getline(c.ss, item, del)) {
		//std::cout << item << std::endl;
		//std::cout << "###" << std::endl;
		if(headerEnd && item != "\r") {
			c.response.append(item);
			//c.response.append("\n");
			//std::cout << "gefunden" << std::endl;
		}
		// HTTP Body beginnt mit naechster Zeile
		if(item.compare("Content-Type: text/html\r")==0)
			headerEnd = true;
	}
    std::cout << c.response << std::endl;
	const char wahr[5]= {'T', 'r', 'u', 'e', '\0'}; //weil cstrings /0 am ende haben, hier aber weg... weil php skript true zurück gibt
	if(c.response.compare(wahr) == 0) {
		std::cout << "Lizenzpruefung erfolgreich" << std::endl;
		return 200;
	} else {
		std::cout << "Lizenzpruefung fehlgeschlagen" << std::endl;
		//std::system("MSG * Lizenzpruefung fehlgeschlagen");
		//std::system("pause");
		Sleep(3000);
		return 404;
	}
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
