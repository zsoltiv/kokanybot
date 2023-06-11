#ifndef MQ135_H
#define MQ135_H

#ifdef __cplusplus
extern "C" {
#endif

struct mq135;
bool mq135_get_presence(struct mq135 *sensor);
int mq135_thread(void *arg);
void mq135_lock(struct mq135 *sensor);
void mq135_unlock(struct mq135 *sensor);
bool mq135_get_presence(struct mq135 *sensor);

#ifdef __cplusplus
}
#endif

#endif /* MQ135_H */
