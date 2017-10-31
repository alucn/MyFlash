

#include "dumpFromRemote.h"

guint8 sendBinlogDumpCommand(MYSQL* mysql, gchar* logName,gsize startPosition){

    gchar* sock=NULL;
    int status;
    status=mysql_query(mysql,"SET @master_binlog_checksum='NONE'");
    if (status)
    {
      g_error("Colud not execute statement(s):%s",mysql_error(mysql));
      mysql_close(mysql);
      exit(0);
    }

    guint16 command=COM_BINLOG_DUMP;
    gchar* command_buffer=NULL;
    gsize command_size=0;
    gboolean stop_never=0;
    constructBinlogDumpCommand(command,logName,startPosition,stop_never,&command_buffer,&command_size);
    if (simple_command(mysql, command, command_buffer, command_size, 1))
    {
      g_warning("Got fatal error sending the log dump command.");
      g_warning("Failed on dump :%s",mysql_error(mysql));
      free(command_buffer);
      return ERROR_STOP;
      //DBUG_RETURN(ERROR_STOP);
    }
    free(command_buffer);

    return OK_CONTINUE;

  }
