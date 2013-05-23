CXX = g++
	
SRC = generate.cpp
OBJ = generate.o

all: generate connect
	
libcryptopp.so:
	cd cryptopp; make libcryptopp.so
	cp cryptopp/libcryptopp.so ./
	
generate.o: generate.cpp
	$(CXX) -c generate.cpp
generate: generate.o libcryptopp.so
	$(CXX) -o generate generate.o libcryptopp.so -lssl -lcrypto

connect.o: database/connect.cpp
	cd database;$(CXX) -I /opt/local/include/mysql5-connector-cpp -I /opt/local/include/ -c connect.cpp
	
connect: connect.o
	cd database;$(CXX) -o connect connect.o -L /opt/local/lib/mysql5 -L /opt/local/lib/mysql5-connector-cpp/ -lmysqlcppconn

clean:
	rm cryptopp/libcryptopp.so database/*.o database/connect *.o *.so generate