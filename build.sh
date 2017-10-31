export LD_LIBRARY_PATH=/usr/local/mysql/lib/
gcc -g -w  `pkg-config --cflags --libs glib-2.0` -I/usr/local/mysql/include -L/usr/local/mysql/lib/ -lmysqlclient source/binlogParseGlib.c source/mysqlHelper/common.c  source/mysqlHelper/dumpFromRemote.c  -o binary/flashback
