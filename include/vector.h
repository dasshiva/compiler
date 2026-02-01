#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdint.h>

typedef struct Vector {
	void** data;
	uint32_t capacity;
	uint32_t size;
} Vector;

Vector*  NewVector();
uint32_t VectorLength(Vector* vec);
void     Append(Vector* vec, void* data);
void*    Get(Vector* vec, uint32_t index);
void     DeleteVector(Vector* vec);

#define INVALID_INDEX ((void*)-1)

#endif
