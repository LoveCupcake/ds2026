#ifndef WAL_H
#define WAL_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE *file;
} WAL;

WAL* wal_open(const char *filename);
void wal_append(WAL *wal, const char *key, const char *value, uint64_t version);
void wal_flush(WAL *wal);
void wal_replay(const char *filename,
                void (*apply)(const char*, const char*, uint64_t));
void wal_close(WAL *wal);

#endif
