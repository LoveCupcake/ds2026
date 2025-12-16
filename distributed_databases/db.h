#ifndef DB_H
#define DB_H

#include "common.h"
#include "wal.h"
#include <pthread.h>

#define HASH_SIZE 1024

typedef struct Entry {
    char key[MAX_KEY];
    char value[MAX_VALUE];
    uint64_t version;
    struct Entry *next;
} Entry;

typedef struct {
    Entry *table[HASH_SIZE];
    pthread_mutex_t lock;
    WAL *wal;
} DB;

DB* db_open(const char *walfile);
void db_put(DB *db, const char *key, const char *value);
int db_get(DB *db, const char *key, char *out);
void db_close(DB *db);

extern DB *global_db;

#endif
