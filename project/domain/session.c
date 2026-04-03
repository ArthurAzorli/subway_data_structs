#include "session.h"

#include "../core/utils/errors.h"
#include <stdlib.h>

struct Session *Session_init() {
    struct Session *session = malloc(sizeof(struct Session));
    if (session == NULL) {
        throwError("Failed to allocate memory for session");
        return NULL;
    }
    session->outputFilePath = NULL;
    session->inputFilePath = NULL;
    return session;
}

void Session_clear(const struct Session *session) {
    if (session == NULL) {
        throwError("Session is NULL");
        return;
    }
    if (session->inputFilePath != NULL) free(session->inputFilePath);
    if (session->outputFilePath != NULL) free(session->outputFilePath);
}

void Session_free(struct Session *session) {
    if (session == NULL) {
        throwError("Session is NULL");
        return;
    }
    Session_clear(session);
    free(session);
}
