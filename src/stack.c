/** @file
 * Implementacja operacji na stosie wielomianów.
 *
 * @author Mateusz Sulimowicz <ms429603@students.mimuw.edu.pl>
 * @date 2021
 */

#include "stack.h"

/** Mnożnik rozmiaru tablicy do realokacji.*/
#define MULTIPLIER 2

/** Początkowy rozmiar alokowanej tablicy */
#define INIT_SIZE 4

/** To jest struktura reprezentująca stos */
struct Stack {
    size_t size; ///< aktualny rozmiar stosu
    size_t top; ///< liczba wielomianów na stosie
    /** To jest tablica przechowująca aktualną zawartość stosu. */
    Poly *arr;
};

void StackInit(Stack *s) {
    assert(s != NULL);
    *s = malloc(sizeof(struct Stack));
    CHECK_PTR(s);
    (*s)->arr = malloc(INIT_SIZE * sizeof(Poly));
    CHECK_PTR((*s)->arr);
    (*s)->size = INIT_SIZE;
    (*s)->top = 0;
}
/**
 * Sprawdza, czy stos @p s jest pusty.
 * @param[in] s : wskaźnik na stos
 * @return Czy @p s jest pusty?
 */
static bool StackIsEmpty(Stack s) {
    assert(s != NULL);
    return (s->top == 0);
}

/**
 * Sprawdza, czy stos @p s jest pełen.
 * @param[in] s : wskaźnik na stos
 * @return Czy @p s jest pełen?
 */
static bool StackIsFull(Stack s) {
    assert(s != NULL);
    return (s->top == s->size);
}

/**
 * Powiększa rozmiar stosu @p s.
 * @param[in,out] s : wskaźnik na stos
 */
static void StackExpand(Stack s) {
    assert(s != NULL);
    size_t new_size = MULTIPLIER * s->size;
    s->arr = realloc(s->arr, new_size * sizeof(Poly));
    CHECK_PTR(s->arr);
    s->size = new_size;
}

void StackPush(Stack s, const Poly *p) {
    assert(s != NULL && p != NULL);
    if (StackIsFull(s)) {
        StackExpand(s);
    }
    s->arr[s->top] = *p;
    ++(s->top);
}

Poly StackTop(Stack s, bool *err) {
    assert(s != NULL && err != NULL);
    if (!StackIsEmpty(s)) {
        Poly p = s->arr[s->top - 1];
        return p;
    } else {
        *err = true;
        return PolyZero();
    }
}

Poly StackPop(Stack s, bool *err) {
    assert(s != NULL && err != NULL);
    Poly res;
    if (!StackIsEmpty(s)) {
        res = s->arr[s->top - 1];
        --(s->top);
    } else {
        *err = true;
        res = PolyZero();
    }
    return res;
}

void StackDestroy(Stack s) {
    assert(s != NULL);
    bool err = false;
    while (!StackIsEmpty(s)) {
        Poly p = StackPop(s, &err);
        PolyDestroy(&p);
    }
    free(s->arr);
    free(s);
}

size_t StackPolyCount(Stack s) {
    assert(s != NULL);
    return s->top;
}
