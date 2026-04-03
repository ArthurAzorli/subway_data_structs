#include "session.h"
#include <stdio.h>
#include <stdlib.h>

struct Session *Session_init() {
    struct Session *session = malloc(sizeof(struct Session));
    if (session == NULL) {
        printf("ERROR: Failed to allocate memory for session\n");
        return NULL;
    }
    session->outputFilePath = NULL;
    session->inputFilePath = NULL;
    return session;
}

void Session_clear(const struct Session *session) {
    if (session == NULL) {
        printf("ERROR: Session is NULL\n");
        return;
    }
    if (session->inputFilePath != NULL) free(session->inputFilePath);
    if (session->outputFilePath != NULL) free(session->outputFilePath);
}

void Session_free(struct Session *session) {
    if (session == NULL) {
        printf("ERROR: Session is NULL\n");
        return;
    }
    Session_clear(session);
    free(session);
}
