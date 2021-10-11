/** @file
 * Interfejs parsera wielomianów z wejścia.
 *
 * @author Mateusz Sulimowicz <ms429603@students.mimuw.edu.pl>
 * @date 2021
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include "poly.h"

/** Podstawa systemu dziesiętnego. */
#define DECIMAL_BASE 10

/**
 * Parsuje tekst na wielomian.
 * Jeśli wystąpił błąd w trakcie parsowania, ustawia `*err = true`.
 * @param[in] str : napis
 * @param[in] line : numer parsowanego wiersza
 * @param[in] len : długość parsowanego wiersza
 * @param[out] err : wskaźnik na informację o błędzie
 * @return sparsowany wielomian z @p str
 */
Poly PolyParse(char *str, size_t line, ssize_t len, bool *err);

#endif //__PARSER_H__