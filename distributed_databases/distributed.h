#ifndef DIST_H
#define DIST_H

#include "common.h"

typedef struct {
    char ip[16];
    int port;
} Peer;

extern Peer peers[MAX_PEERS];
extern int peer_count;

void dist_put(const char *key, const char *value);

#endif
