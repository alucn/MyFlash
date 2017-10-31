


#include <glib.h>
#include <my_command.h>
#include <my_dbug.h>
#include <stdio.h>
#include "common.h"

const int BINLOG_POS_INFO_SIZE= 8;
const int BINLOG_DATA_SIZE_INFO_SIZE= 4;
const int BINLOG_POS_OLD_INFO_SIZE= 4;
const int BINLOG_FLAGS_INFO_SIZE= 2;
const int BINLOG_SERVER_ID_INFO_SIZE= 4;
const int BINLOG_NAME_SIZE_INFO_SIZE= 4;
const int BINLOG_DUMP_NON_BLOCK= 1<<0;



int get_dump_flags( gboolean stop_never)
{
  return stop_never ? 0 : BINLOG_DUMP_NON_BLOCK;
}

MYSQL* getConnection(gchar* host, guint16 port, gchar* user, gchar* pass,gchar* db, gchar* sock){
  MYSQL* mysql;
  mysql=mysql_init(NULL);
  if( NULL == mysql ){
    g_warning("Insufficient memory to allocate mysql handler");
    exit(1);
  }
  if(!mysql_real_connect(mysql, host, user, pass, db, port, sock, 0)){
    g_warning("Failed on connect :%s",mysql_error(mysql));
    exit(1);
  }


  return mysql;

}

int constructBinlogDumpCommand(guint16 command, gchar* logname, guint64 start_position,gboolean stop_never,gchar** command_buffer, gsize *command_size){

  gsize tlen = strlen(logname);
  const gsize BINLOG_NAME_INFO_SIZE=tlen;
  //uchar *command_buffer= NULL;
  //size_t command_size= 0;
  guint16 server_id= 1;
  gchar* command_buffer_new;

  if (command == COM_BINLOG_DUMP)
  {

    gsize allocation_size= BINLOG_POS_OLD_INFO_SIZE +
      BINLOG_NAME_INFO_SIZE + BINLOG_FLAGS_INFO_SIZE +
      BINLOG_SERVER_ID_INFO_SIZE + 1;
    if (!(command_buffer_new= (uchar *) malloc(allocation_size)))
    {
      g_error("Got fatal error allocating memory.");
      return ERROR_STOP;
    }
    gchar* ptr_buffer= command_buffer_new;

    /*
      COM_BINLOG_DUMP accepts only 4 bytes for the position, so
      we are forced to cast to uint32.
    */
    int4store(ptr_buffer, (uint32) start_position);
    ptr_buffer+= BINLOG_POS_OLD_INFO_SIZE;
    int2store(ptr_buffer, get_dump_flags(stop_never));
    ptr_buffer+= BINLOG_FLAGS_INFO_SIZE;
    int4store(ptr_buffer, server_id);
    ptr_buffer+= BINLOG_SERVER_ID_INFO_SIZE;
    memcpy(ptr_buffer, logname, BINLOG_NAME_INFO_SIZE);
    ptr_buffer+= BINLOG_NAME_INFO_SIZE;

    *command_size= ptr_buffer - command_buffer_new;
    *command_buffer=command_buffer_new;
    //DBUG_ASSERT(command_size == (allocation_size - 1));
    //assert(command_size == (allocation_size - 1));
  }

  return 0;
}

gchar* getHeaderFromRawEvent(gchar* rawEvent, gsize headerLength){
  gchar* headerBuffer;
  headerBuffer=(gchar*)malloc(headerLength);
  memcpy(headerBuffer,rawEvent,headerLength);
  return headerBuffer;
}

gchar* getDataFromRawEvent(gchar* rawEvent,gsize dataLength){
  gchar* dataBuffer;
  dataBuffer=(gchar*)malloc(dataLength);
  memcpy(dataBuffer,rawEvent,dataLength);
  return dataBuffer;
}

gboolean isConsideredEventType(guint8 eventType){
  gboolean isConsidered=FALSE;
  switch(eventType){
    case FORMAT_DESCRIPTION_EVENT:{
      isConsidered=TRUE;
      break;
    }
    case TABLE_MAP_EVENT:{
      isConsidered=TRUE;
      break;
    }
    case WRITE_ROWS_EVENT:
    case UPDATE_ROWS_EVENT:
    case DELETE_ROWS_EVENT:{
      isConsidered=TRUE;
      break;
    }
    case QUERY_EVENT:{
      isConsidered=TRUE;
      break;
    }
    case XID_EVENT:{
      isConsidered=TRUE;
      break;
    }
    case GTID_LOG_EVENT:{
      isConsidered=TRUE;
      break;
    }
  }

  return isConsidered;
}

gboolean checkPotentialConflictOutputFile(gchar* baseName){
    int reti;
    regex_t regex;
    reti = regcomp(&regex, baseName, 0);
    if(reti){
      g_error("failed to compile regex %s", baseName);
    }
    DIR           *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
      while ((dir = readdir(d)) != NULL)
      {
        if (!regexec(&regex, dir->d_name, 0, NULL, 0)){
          g_error("the output %s.* may overwrite the existing file %s, please choose a newFileName specified by --outBinlogFileNameBase or remove the existing file",baseName,dir->d_name);
          return FALSE;
        }
      }

      closedir(d);
    }

    return TRUE;
}

gboolean isDirExists(gchar* baseName){
  gchar *dirCopy, *dname;
  DIR* dir;
  struct stat statbuf;

  dirCopy=strdup(baseName);
  dname=dirname(dirCopy);

  if (stat(dname, &statbuf) != -1) {
      return TRUE;
  }else{
      printf("dir: %s does not exists\n",dname);
      return FALSE;

 }

}

gchar* constructFileNameWithPostfixIndex(gchar* baseName, gsize postfixIndex){
  gchar* completeFileName;
  //completeFileName=g_new0(gchar,strlen(baseName)+postfixDisplayLength+1+1);
  if(0 == postfixIndex){
    completeFileName=g_strdup_printf("%s",baseName);
  }
  else{
    completeFileName=g_strdup_printf("%s.%06lu",baseName,postfixIndex);
  }
  return completeFileName;
}

gboolean rotateFile(gchar* baseName, gsize postfixIndex){
    gchar* completeFileName=constructFileNameWithPostfixIndex(baseName,postfixIndex);
    if( -1 == access( completeFileName, F_OK )  ) {
      return FALSE;
    }
    rotateFile(baseName,postfixIndex+1);
    gchar* newFileName=constructFileNameWithPostfixIndex(baseName,postfixIndex+1);
    gchar* oldFileName=constructFileNameWithPostfixIndex(baseName,postfixIndex);
    if(0 != rename(oldFileName,newFileName) ){
      g_error("unable to rename %s to %s",oldFileName, newFileName);
    }
    return TRUE;
}

gboolean rotateOutputBinlogFileNames(gchar* baseName, gsize postfixIndex){
    gchar* flashbackBaseName;
    flashbackBaseName=g_strdup_printf("%s.%s",baseName,"flashback");
    rotateFile(flashbackBaseName,postfixIndex);
    return TRUE;
}
