#g++ -c connect.cpp -I /opt/local/include -I include
#g++ -o connect connect.o lib/libmysqlcppconn-static.a lib/libmysqlcppconn.7.1.1.3.dylib -I /opt/local/include/mysql5 -lz -lm /opt/local/lib/mysql5/mysql/libmysqlclient.a 
g++ -c connect.cpp -I /opt/local/include/mysql5-connector-cpp -I /opt/local/include/
g++ -o connect connect.o -L /opt/local/lib/mysql5 -L /opt/local/lib/mysql5-connector-cpp/ -lmysqlcppconn
./connect
