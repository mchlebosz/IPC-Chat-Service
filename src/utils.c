#include "utils.h"

unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int createSessonKey(int clientId, int sessionSeed)
{
    return (clientId << 7) + sessionSeed;
}
