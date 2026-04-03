#ifndef TRABALHO01_SESSION_H
#define TRABALHO01_SESSION_H

struct Session {
    char *inputFilePath;
    char *outputFilePath;
};

struct Session *Session_init();

void Session_clear(const struct Session *session);

void Session_free(struct Session *session);

#endif //TRABALHO01_SESSION_H
