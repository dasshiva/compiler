#ifndef __COUNTER_H__
#define __COUNTER_H__

typedef int Counter;

int GetCounter(Counter* ctr);
void ResetCounter(Counter* ctr, int def);

#endif
