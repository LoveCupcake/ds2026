#include "wal.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

WAL* wal_open(const char *filename) {
    WAL *wal = malloc(sizeof(WAL));
    wal->file = fopen(filename, "a+");
    if (!wal->file) {
        free(wal);
        return NULL;
    }
    return wal;
}

void wal_append(WAL *wal, const char *key, const char *value, uint64_t version) {
    fprintf(wal->file, "PUT %s %s %llu\n", key, value, (unsigned long long)version);
}

void wal_flush(WAL *wal) {
    fflush(wal->file);
    fsync(fileno(wal->file));
}

void wal_replay(const char *filename,
                void (*apply)(const char*, const char*, uint64_t)) {
    FILE *f = fopen(filename, "r");
    if (!f) return;

    char op[16], key[128], value[512];
    unsigned long long version;

    while (fscanf(f, "%s %127s %511s %llu", op, key, value, &version) == 4) {
        if (strcmp(op, "PUT") == 0) {
            apply(key, value, (uint64_t)version);
        }
    }
    fclose(f);
}

void wal_close(WAL *wal) {
    if (wal) {
        if (wal->file) {
            fclose(wal->file);
        }
        free(wal);
    }
}