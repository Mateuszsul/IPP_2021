/** @file
 * Interfejs stosu.
 *
 * @author Mateusz Sulimowicz <ms429603@students.mimuw.edu.pl>
 * @date 2021
 */

#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>
#include "poly.h"

/** To jest definicja typu wskaźnika na stos */
typedef struct Stack* Stack;

/**
 * Inicjuje stos.
 * @param[out] s : wskaźnik na zainicjowany stos
 */
void StackInit(Stack *s);

/**
 * Zdejmuje wielomian ze szczytu
 * stosu i przejmuje go na własność.
 * Jeśli stos @p s jest pusty, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos
 * @param[out] err : wskaźnik na informację o błędzie
 * @return wielomian ze szczytu @p s.
 */
Poly StackPop(Stack s, bool *err);

/**
 * Wstawia wielomian @p p na stos @p s.
 * @param[in,out] s : wskaźnik na stos
 * @param[in] p : wielomian
 */
void StackPush(Stack s, const Poly *p);

/**
 * Zwraca wielomianu z wierzchołka stosu, ale go nie zdejmuje.
 * Jeśli stos jest pusty, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos
 * @param[out] err : wskaźnik na informację o błędzie
 * @return wielomianu z wierzchołka @p s
 */
Poly StackTop(Stack s, bool *err);

/**
 * Usuwa stos @p s z pamięci.
 * @param[in] s : wskaźnik na stos
 */
void StackDestroy(Stack s);

/**
 * Zwraca aktualną liczbę wielomianów na stosie @p s.
 * @param s : wskaźnik na stos
 * @return liczba wielomianów na stosie @p s
 */
size_t StackPolyCount(Stack s);

#endif //__STACK_H__