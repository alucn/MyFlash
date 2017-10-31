#include <mysql.h>
#include <glib.h>
#include <regex.h>
#include <dirent.h>

enum Exit_status_extend {
  /** No error occurred and execution should continue. */
  OK_CONTINUE= 0,
  /** No error occured and discard event for filter */
  OK_DISCARD,
  /** An error occurred and execution should stop. */
  ERROR_STOP,
  /** No error occurred but execution should stop. */
  OK_STOP
};


enum Status_Stop_Discard
{
  GOON=0,
  STOP=1,
  DISCARD=2
};


enum Binlog_event_type
{
  UNKNOWN_EVENT= 0,
  START_EVENT_V3= 1,
  QUERY_EVENT= 2,
  STOP_EVENT= 3,
  ROTATE_EVENT= 4,
  INTVAR_EVENT= 5,
  LOAD_EVENT= 6,
  SLAVE_EVENT= 7,
  CREATE_FILE_EVENT= 8,
  APPEND_BLOCK_EVENT= 9,
  EXEC_LOAD_EVENT= 10,
  DELETE_FILE_EVENT= 11,
  NEW_LOAD_EVENT= 12,
  RAND_EVENT= 13,
  USER_VAR_EVENT= 14,
  FORMAT_DESCRIPTION_EVENT= 15,
  XID_EVENT= 16,
  BEGIN_LOAD_QUERY_EVENT= 17,
  EXECUTE_LOAD_QUERY_EVENT= 18,

  TABLE_MAP_EVENT = 19,
  PRE_GA_WRITE_ROWS_EVENT = 20,
  PRE_GA_UPDATE_ROWS_EVENT = 21,
  PRE_GA_DELETE_ROWS_EVENT = 22,

  WRITE_ROWS_EVENT_V1 = 23,
  UPDATE_ROWS_EVENT_V1 = 24,
  DELETE_ROWS_EVENT_V1 = 25,

  INCIDENT_EVENT= 26,

  HEARTBEAT_LOG_EVENT= 27,

  IGNORABLE_LOG_EVENT= 28,
  ROWS_QUERY_LOG_EVENT= 29,

  WRITE_ROWS_EVENT = 30,
  UPDATE_ROWS_EVENT = 31,
  DELETE_ROWS_EVENT = 32,

  GTID_LOG_EVENT= 33,
  ANONYMOUS_GTID_LOG_EVENT= 34,

  PREVIOUS_GTIDS_LOG_EVENT= 35,
  TRANSACTION_CONTEXT_EVENT= 36,

  VIEW_CHANGE_EVENT= 37,

  XA_PREPARE_LOG_EVENT= 38,
  ENUM_END_EVENT
};

static gchar Binlog_event_type_name[][30]={
  "UNKNOWN_EVENT",
  "START_EVENT_V3",
  "QUERY_EVENT",
  "STOP_EVENT",
  "ROTATE_EVENT",
  "INTVAR_EVENT",
  "LOAD_EVENT",
  "SLAVE_EVENT",
  "CREATE_FILE_EVENT",
  "APPEND_BLOCK_EVENT",
  "EXEC_LOAD_EVENT",
  "DELETE_FILE_EVENT",
  "NEW_LOAD_EVENT",
  "RAND_EVENT",
  "USER_VAR_EVENT",
  "FORMAT_DESCRIPTION_EVENT",
  "XID_EVENT",
  "BEGIN_LOAD_QUERY_EVENT",
  "EXECUTE_LOAD_QUERY_EVENT",

  "TABLE_MAP_EVENT",
  "PRE_GA_WRITE_ROWS_EVENT",
  "PRE_GA_UPDATE_ROWS_EVENT",
  "PRE_GA_DELETE_ROWS_EVENT",

  "WRITE_ROWS_EVENT_V1",
  "UPDATE_ROWS_EVENT_V1",
  "DELETE_ROWS_EVENT_V1",

  "INCIDENT_EVENT",

  "HEARTBEAT_LOG_EVENT",

  "IGNORABLE_LOG_EVENT",
  "ROWS_QUERY_LOG_EVENT",

  "WRITE_ROWS_EVENT",
  "UPDATE_ROWS_EVENT",
  "DELETE_ROWS_EVENT",
  "GTID_LOG_EVENT",
  "ANONYMOUS_GTID_LOG_EVENT",

  "PREVIOUS_GTIDS_LOG_EVENT",
  "TRANSACTION_CONTEXT_EVENT",

  "VIEW_CHANGE_EVENT",

  "XA_PREPARE_LOG_EVENT",
  "ENUM_END_EVENT"

};

typedef enum enum_dml_types {
  INSERT =0,
  UPDATE =1,
  DELETE =2,
  UNKNOWN_TYPE
} enum_dml_types;
/*
typedef enum enum_field_types {
  MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
  MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
  MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
  MYSQL_TYPE_NULL, MYSQL_TYPE_TIMESTAMP,
  MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
  MYSQL_TYPE_DATE, MYSQL_TYPE_TIME,
  MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
  MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
  MYSQL_TYPE_BIT,
  MYSQL_TYPE_TIMESTAMP2,
  MYSQL_TYPE_DATETIME2,
  MYSQL_TYPE_TIME2,
  MYSQL_TYPE_JSON=245,
  MYSQL_TYPE_NEWDECIMAL=246,
  MYSQL_TYPE_ENUM=247,
  MYSQL_TYPE_SET=248,
  MYSQL_TYPE_TINY_BLOB=249,
  MYSQL_TYPE_MEDIUM_BLOB=250,
  MYSQL_TYPE_LONG_BLOB=251,
  MYSQL_TYPE_BLOB=252,
  MYSQL_TYPE_VAR_STRING=253,
  MYSQL_TYPE_STRING=254,
  MYSQL_TYPE_GEOMETRY=255
} enum_field_types;
*/

typedef enum enum_flag
  {
  /* Last event of a statement */
  STMT_END_F = (1U << 0),

  /* Value of the OPTION_NO_FOREIGN_KEY_CHECKS flag in thd->options */
  NO_FOREIGN_KEY_CHECKS_F = (1U << 1),

  /* Value of the OPTION_RELAXED_UNIQUE_CHECKS flag in thd->options */
  RELAXED_UNIQUE_CHECKS_F = (1U << 2),

  /**
  Indicates that rows in this event are complete, that is contain
  values for all columns of the table.
  */
  COMPLETE_ROWS_F = (1U << 3)
  } enum_flag;

  typedef struct _GtidSetInfo{
    gchar* uuid;
    guint64 startSeqNo;
    guint64 stopSeqNo;
  } GtidSetInfo;

  typedef struct _EventHeader{
  	guint32 binlogTimestamp;
  	guint8  eventType;
  	guint32 serverId;
  	guint32 eventLength;
  	guint32 nextEventPos;
  	guint16  flag;
  	gchar *rawEventHeader;
  } EventHeader;

  typedef struct _FormatDescriptionEvent{
  	EventHeader *eventHeader;
  	gchar *rawFormatDescriptionEventDataDetail;
  } FormatDescriptionEvent;

  typedef struct _TableMapEvent{
  	EventHeader           *eventHeader;
  	gchar                 *rawTableMapEventDataDetail;
    gsize databaseNameLength;
    gchar* databaseName;
    gsize tableNameLength;
  	gchar* tableName;
  	guint64 tableId;
  	gsize columnNumber;
  	GByteArray            *columnTypeArray;
    guint16               *columnMetadataArray;
    gsize                  metadataBlockSize;
  } TableMapEvent;

  typedef struct _RowEvent{
  	EventHeader           *eventHeader;
  	gchar                 *rawRowEventDataDetail;
  } RowEvent;

  typedef struct _QueryEvent{
    EventHeader           *eventHeader;
    gchar                 *rawQueryEventDataDetail;
    gsize                 databaseNameLength;
    gchar                 *databaseName;
    gchar                 sqlTextLength;
    gchar                 *sqlText;
  } QueryEvent;


  typedef struct _GtidEvent{
    EventHeader           *eventHeader;
    gchar                 *rawGtidEventDataDetail;
    gchar                 *uuid;
    guint64               seqNo;
  } GtidEvent;



  typedef struct _XidEvent{
    EventHeader           *eventHeader;
    gchar                 *rawXidEventDataDetail;
    guint64               xid;
  } XidEvent;


  typedef struct _RotateEvent{
    EventHeader          *eventHeader;
    gchar                *rawRotateEventDataDetail;
    gchar                *binlogFileName;
  } RotateEvent;

  typedef struct _EventWrapper{
    guint8    eventType;
    gpointer  eventPointer;
  } EventWrapper;


  typedef struct _LeastExecutionUnitEvents{
  	TableMapEvent *tableMapEvent;
    GList         *rowEventList;
    guint8        originalRowEventType;
  } LeastExecutionUnitEvents;


#define packet_error (~(unsigned long) 0)

MYSQL* getConnection(gchar* host, guint16 port, gchar* user, gchar* pass, gchar* db, gchar* sock);
int constructBinlogDumpCommand(guint16 command, gchar* logname, guint64 start_position,
  gboolean stop_never,gchar** command_buffer, gsize *command_size);

gchar* getHeaderFromRawEvent(gchar* rawEvent,gsize headerLength);
gchar* getDataFromRawEvent(gchar* rawEvent,gsize dataLength);

gboolean isConsideredEventType(guint8 eventType);

gboolean checkPotentialConflictOutputFile(gchar* baseName);

gboolean isDirExists(gchar* baseName);
gchar* constructFileNameWithPostfixIndex(gchar* baseName, gsize postfixIndex);

gboolean rotateFile(gchar* baseName, gsize postfixIndex);

gboolean rotateOutputBinlogFileNames(gchar* baseName, gsize postfixIndex);
