#include <stdlib.h>
#include "vector.h"

Vector* NewVector() {
	return VectorOfLength(0);
}

Vector* VectorOfLength(uint32_t size) {
	Vector* vec = malloc(sizeof(Vector));
	if (!vec)
		return NULL;

	vec->size = size;
	vec->capacity = vec->size + 100;
	vec->data = malloc(sizeof(void*) * vec->capacity);

	if (!vec->data) {
		free(vec);
		return NULL;
	}

	return vec;
}

void DeleteVector(Vector* vec) {
	if (!vec)
		return;

	free(vec->data);
	free(vec);
}

uint32_t VectorLength(Vector* vec) {
	return vec->size;
}

void Append(Vector* vec, const void* data) {
	if (vec->size >= vec->capacity) {
		vec->capacity += 100;
		vec->data = realloc(vec->data, vec->capacity * sizeof(void*));
		if (!vec->data)
			return;
	}

	vec->data[vec->size] = (void*) data;
	vec->size++;
}

void* Get(Vector* vec, uint32_t index) {
	if (index >= vec->size)
		return INVALID_INDEX;

	return vec->data[index];
}

void* Pop(Vector* vec) {
	if (!vec->size)
		return INVALID_INDEX;

	void* data = vec->data[vec->size - 1];
	vec->size--;
	return data;
}
