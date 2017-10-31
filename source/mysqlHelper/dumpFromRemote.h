
#include "common.h"
#define simple_command(mysql, command, arg, length, skip_check) cli_advanced_command(mysql, command, 0, 0, arg, length, skip_check, NULL)


guint8 sendBinlogDumpCommand(MYSQL* mysql, gchar* logName,gsize startPosition);
