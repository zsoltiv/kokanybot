#ifndef STREAM_H
#define STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

void stream_init(void);
int stream_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* STREAM_H */
