#include "db.h"
#include <stdlib.h>
#include <string.h>

DB *global_db = NULL;
static uint64_t global_version = 1;

static unsigned hash(const char *key) {
    unsigned h = 0;
    while (*key) h = h * 33 + *key++;
    return h % HASH_SIZE;
}

static void apply_replay(const char *key, const char *value, uint64_t version) {
    unsigned idx = hash(key);
    Entry *e = global_db->table[idx];

    // Search for existing key
    while (e) {
        if (strcmp(e->key, key) == 0) {
            // Update if newer version
            if (version > e->version) {
                strncpy(e->value, value, MAX_VALUE - 1);
                e->value[MAX_VALUE - 1] = '\0';
                e->version = version;
            }
            return;
        }
        e = e->next;
    }

    // Key not found, create new entry
    e = malloc(sizeof(Entry));
    if (!e) return;
    
    strncpy(e->key, key, MAX_KEY - 1);
    e->key[MAX_KEY - 1] = '\0';
    strncpy(e->value, value, MAX_VALUE - 1);
    e->value[MAX_VALUE - 1] = '\0';
    e->version = version;
    e->next = global_db->table[idx];
    global_db->table[idx] = e;
    
    // Update global version counter
    if (version >= global_version) {
        global_version = version + 1;
    }
}

DB* db_open(const char *walfile) {
    DB *db = calloc(1, sizeof(DB));
    if (!db) return NULL;
    
    pthread_mutex_init(&db->lock, NULL);
    db->wal = wal_open(walfile);
    
    if (!db->wal) {
        free(db);
        return NULL;
    }

    global_db = db;
    wal_replay(walfile, apply_replay);

    return db;
}

void db_put(DB *db, const char *key, const char *value) {
    pthread_mutex_lock(&db->lock);
    
    uint64_t version = global_version++;
    
    // Write to WAL first (durability)
    wal_append(db->wal, key, value, version);
    wal_flush(db->wal);

    // Then update memory
    apply_replay(key, value, version);
    
    pthread_mutex_unlock(&db->lock);
}

int db_get(DB *db, const char *key, char *out) {
    pthread_mutex_lock(&db->lock);
    
    unsigned idx = hash(key);
    Entry *e = db->table[idx];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            strncpy(out, e->value, MAX_VALUE - 1);
            out[MAX_VALUE - 1] = '\0';
            pthread_mutex_unlock(&db->lock);
            return 1;
        }
        e = e->next;
    }

    pthread_mutex_unlock(&db->lock);
    return 0;
}

void db_close(DB *db) {
    if (!db) return;
    
    // Free all entries
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry *e = db->table[i];
        while (e) {
            Entry *next = e->next;
            free(e);
            e = next;
        }
    }
    
    wal_close(db->wal);
    pthread_mutex_destroy(&db->lock);
    free(db);
}