#include <stdio.h>
#include "sqlite3.h"
//int main(void)
//{
//	sqlite3* db = 0;
//	sqlite3_stmt* stmt = 0;
//	char* err=0;
//	char* sql;
//	int retcode;
//	retcode = sqlite3_open("/home/zhouyao/workspace/intelij/Clion/sqlite_demo1/MyDB", &db); /* Open a database named MyDB */
////	fprintf(stderr, "OK");
//	if (retcode != SQLITE_OK){
//		sqlite3_close(db);
//		fprintf(stderr, "Could not open MyDB\n");
//	return retcode;
//	}
////	sql ="select sum(SID) from Students where SID >=400;";
////	sql = "select SID from Students where SID >=400 limit 2 offset 1;";
//    sql = "select * from Students;";
//  	retcode = sqlite3_prepare(db, sql, -1, &stmt, 0);
//
//	if (retcode != SQLITE_OK){
//		sqlite3_close(db);
//	fprintf(stderr, "Could not execute SELECT\n");
//	return retcode;
//	}
//	while (sqlite3_step(stmt) == SQLITE_ROW){
//		int i = sqlite3_column_int(stmt, 0);
//	printf("SID = %d\n", i);
//	}
//
//	sqlite3_finalize(stmt);
//	sqlite3_close(db);
//
//	return SQLITE_OK;
//}

static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    fprintf(stderr, "%s: ", (const char*)data);
    for(i=0; i<argc; i++){
        printf("%s = %s \t", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main(void)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char* data = "Callback function called";

    /* Open database */

    rc = sqlite3_open("/home/zhouyao/workspace/jetbrains/ClionProjects/sqlite_demo1/yaoreadcopy.db", &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    }else{
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sql = "SELECT id,fname,name from reading where id=2";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }else{
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);
    return 0;
}