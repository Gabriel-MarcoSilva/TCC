#ifndef VIBRATION_H
#define VIBRATION_H

#include <stdbool.h>

void vibration_start(const char *padrao, bool m1, bool m2);
void vibration_stop(void);

#endif
