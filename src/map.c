#include <stdlib.h>
#include <stdint.h>

#include "vector.h"

typedef uint32_t (*HashContents) (void* data);
typedef int (*Compare) (void* e1, void* e2);

typedef struct Map {
	Vector* map;
	HashContents* hasher;
	Compare* compare; 
	uint32_t size;
} Map;

typedef struct Entity {
	void* key;
	void* value;
} Entity;

enum EntryType {
	ETYPE_DATA,
	ETYPE_COLLISION
} EntryType;

typedef struct Entry {
	union {
		Vector* colliders;
		Entity* data;
	};
	int type;
} Entry;

Map* NewMap(HashContents* hasher, Compare* compare) {

}
