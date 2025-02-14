#ifndef _VECTOR_H_
#define _VECTOR_H_

#ifndef VECTOR_STEP
#define VECTOR_STEP 16
#endif

#ifdef DEBUG
#define VECTOR_CHECK_ELEMENTS_NULL(x)                                                        \
    if (vec->elements == NULL) {                                                             \
        printf("%sError in %s on line %d\n%s", "\033[1;31m", __FILE__, __LINE__, "\033[0m"); \
        return x;                                                                            \
    }
#else
#define VECTOR_CHECK_ELEMENTS_NULL(x)
#endif

typedef int (*comparefn)(const void*, const void*);

typedef struct {
    unsigned int element_size;
    unsigned int actual_length;
    unsigned int logical_length;
    void* elements;
} Vector;

// Basic
void vector_create(Vector* vec, unsigned int element_size);
void vector_destroy(Vector* vec, void (*element_destroy)(void*));
void vector_push(Vector* vec, void* element);
void* vector_pop(Vector* vec, void* element);

// Advanced
void* vector_replace(const Vector* vec, unsigned int index, void* element, void (*element_destroy)(void*));
void vector_sort(Vector* vec, comparefn cmpfn);

// Utils
void vector_print(const Vector* vec, void (*element_print)(const void*));
void* vector_at(const Vector* vec, unsigned int index);
void* vector_start(const Vector* vec);
void* vector_end(const Vector* vec);
unsigned int vector_capacity(const Vector* vec);
unsigned int vector_size(const Vector* vec);

#endif
