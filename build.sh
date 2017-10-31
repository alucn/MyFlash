ln -s binary/libmysqlclient.so.20.3.7 /usr/local/mysql/lib/libmysqlclient.so
export LD_LIBRARY_PATH=/usr/local/mysql/lib/
 gcc -g -w  `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags mysqlclient` -L/usr/local/mysql/lib/ -lmysqlclient source/binlogParseGlib.c source/mysqlHelper/common.c  source/mysqlHelper/dumpFromRemote.c  -o binary/flashback
