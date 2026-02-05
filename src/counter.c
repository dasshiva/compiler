#include "counter.h"

int GetCounter(Counter* ctr) {
	int ret = *ctr;
	(*ctr)++;
	return ret;
}

void ResetCounter(Counter* ctr, int def) { *ctr = def; }

