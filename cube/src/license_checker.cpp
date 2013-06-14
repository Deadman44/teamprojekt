/***********************************************************************************
Teamprojekt
Lizenzierungstechnike am Beispiel von Cube
     ___          ___          ___          ___     
    /  /\        /  /\        /  /\        /  /\    
   /  /::\      /  /:/       /  /::\      /  /::\   
  /  /:/\:\    /  /:/       /  /:/\:\    /  /:/\:\  
 /  /:/  \:\  /  /:/       /  /::\ \:\  /  /::\ \:\ 
/__/:/ \  \:\/__/:/     /\/__/:/\:\_\:|/__/:/\:\ \:\
\  \:\  \__\/\  \:\    /:/\  \:\ \:\/:/\  \:\ \:\_\/ 
 \  \:\       \  \:\  /:/  \  \:\_\::/  \  \:\ \:\  
  \  \:\       \  \:\/:/    \  \:\/:/    \  \:\_\/  
   \  \:\       \  \::/      \__\::/      \  \:\    
    \__\/        \__\/           ~~        \__\/    

Feilen Markus,Wilde Hermann,Hoor Johannes,Schneider Florian

Beschreibung.:
Baut die SSL Verbindung zum Lizenzserver auf und managed die Tickets
************************************************************************************/
#include "cube.h"


enum { max_length = 1024};

class license_checker
{
public:
    std::stringstream ss;	// Hier wird die vollstaendige Antwort des Servers gespeichert
    std::string response;	// Hier wird der HTTP Body der Antwort gespeichert
	std::string user;		
	std::string password;
	
	
	license_checker(boost::asio::io_service& io_service,
      boost::asio::ssl::context& context,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context)
  {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
        boost::bind(&license_checker::verify_certificate, this, _1, _2));

    boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
        boost::bind(&license_checker::handle_connect, this,
          boost::asio::placeholders::error));
  }
  // Zertifikat 
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
          boost::bind(&license_checker::handle_handshake, this,
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
	  size_t request_length = strlen(request_);
	  // Bei der Erstverbindung werden die Benutzerdaten mitgesendet
	  if(initialization) {
		  boost::asio::async_write(socket_,
			  boost::asio::buffer("GET /cCheck_License_Key_ADV.php?email=" + user + "&pass=" + password + "&license=" + license + " HTTP/1.1\r\nHost: localhost\r\n\r\n"),
			  boost::bind(&license_checker::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		  initialization = false;
		  std::cout << "URL: " << "GET /cCheck_License_Key_ADV.php?email=" + user + "&pass=" + password + "&license=" + license + " HTTP/1.1\r\nHost: localhost\r\n\r\n" << std::endl;
	  } else {
	  // Bei weiteren Verbindungen wird nur noch der Nutzername und das Ticket gesendet
		  boost::asio::async_write(socket_,
			  boost::asio::buffer("GET /cCheck_License_Key_permanent_check.php?email=" + user + "&ticket=" + ticket + " HTTP/1.1\r\nHost: localhost\r\n\r\n"),
			  boost::bind(&license_checker::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		  std::cout << "Lizenz mit Ticket:" << ticket << std::endl;
		  std::cout << "GET /cCheck_License_Key_permanent_check.php?email=" + user + "&ticket=" + ticket + " HTTP/1.1\r\nHost: localhost\r\n\r\n" << std::endl;
	  }
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
		socket_.async_read_some(boost::asio::buffer(reply_, bytes_transferred),boost::bind(&license_checker::handle_read, this,
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
		ss << std::string(reply_, bytes_transferred);
      socket_.async_read_some(boost::asio::buffer(reply_, max_length),
          boost::bind(&license_checker::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
	else if (0 != bytes_transferred)
	       {
	           std::cout <<"\n" << "Quit";
	       }
  }

  void setLicense(std::string l) {
	  license = urlencode(l);
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
	// Objekte der Boost::Asio-Bibliothek für asynchrone Kommunikation
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    // Definiere ein Query mit Host (DNS Namen erlaubt) und Port
	boost::asio::ip::tcp::resolver::query query("127.0.0.1","443");
	// Löse DNS Namen in IP-Adresse auf
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
	// SSL Kontext wird erzeugt
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
	ctx.load_verify_file("server.crt");
    license_checker c(io_service, ctx, iterator);
	c.user = u;
	c.password = p;
	c.setLicense(l);
	// Blockierender Aufruf, der bis zum Erhalt aller Daten vom Server wartet
    io_service.run();
	// HTTP Parser, der den HTTP Header vom HTTP Body trennt und den Body in c.repsonse speichert
	char del = '\n';
	std::string item;
	bool headerEnd = false;
	while(std::getline(c.ss, item, del)) {
		if(headerEnd && item != "\r") {
			c.response.append(item);
		}
		// HTTP Body beginnt mit naechster Zeile
		if(item.compare("Content-Type: text/html\r")==0)
			headerEnd = true;
	}
	// Falls die Authentifizierung erfolgreich war, sendet der Server "True" konkateniert mit dem Ticket zurück
	if(c.response.substr(0,4).compare("True")==0) {
		std::cout << "Lizenzpruefung erfolgreich" << std::endl;
		std::cout << "HTTP Body Response: " << c.response << std::endl;
		// Ticket aus der Antwort extrahieren
		std::string responseTicket = c.response.substr(4,92); //test 88+4 als länge des ticket
		ticket = c.urlencode(responseTicket);
		std::cout << "Ticket: " << ticket << std::endl;
		// Prüfung erfolgreich, es wird Statuscode 200 zurückgegeben
		return 200;
	} else if(c.response.substr(0,5).compare("False")==0) {
		conoutf("Lizenzpruefung fehlgeschlagen");
		// Wartezeit, damit Nutzer Die Nachricht zur Kenntnis nehmne kann
		return 403;
	} else {
		// Verbindungsfehler
		conoutf("Falsche HTTP Antwort");
		std::cout << c.response << std::endl;
		return 500;
	}
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

std::string license_datei() {
	// Lizenzdatei von Festplatte auslesen
	std::ifstream licence_key_datei;
	licence_key_datei.open("Lizenzschluessel.txt");
    if (!licence_key_datei)	// muss existieren
    {
        conoutf("Lizenzschluessel.txt kann nicht geoeffnet werden!");
		conoutf("Bitte legen Sie die Datei Lizenzschluessel.txt an und speichern Sie in dieser den Lizenzschlüssel");
		boost::posix_time::seconds waiting(10);
		boost::this_thread::sleep(waiting);
		quit();
    }
	char c;
	std::string licencenumber;
	while (licence_key_datei.get(c))
	{
		licencenumber += c;
	}
	licence_key_datei.close();
	return licencenumber;
}
