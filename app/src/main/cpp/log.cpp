#include "log.h"

static int pfd[2] = {0, 0};

static void *thread_func(void*)
{
    ssize_t rdsz = 0;
    char buf[128] = {0};
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0; // add null-terminator
        __android_log_write(ANDROID_LOG_DEBUG, "STDOUT&ERR", buf);
    }
    return nullptr;
}

void initStdIORedirection()
{
    // make stdout line-buffered and stderr unbuffered
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    // create the pipe and redirect stdout and stderr
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    // spawn the logging thread
    pthread_t thr;
    if(pthread_create(&thr, 0, thread_func, 0) != -1) pthread_detach(thr);
}
