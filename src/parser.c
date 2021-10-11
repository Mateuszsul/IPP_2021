/** @file
 * Implementacja parsera wielomianów z tekstu.
 *
 * @author Mateusz Sulimowicz
 * @date 2021
 */

/** Makro potrzebne do korzystania z GNU C Library. */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include "parser.h"

/** Mnożnik rozmiaru tablicy do realokacji. */
#define MULTIPLIER 2

/** Początkowy rozmiar alokowanej tablicy */
#define INIT_SIZE 4

/**
 * Parsuje tekst na wykładnik jednomianu.
 * Po wykonaniu, `*endptr` wskazuje o jedną pozycję dalej niż sparsowany napis.
 * Jeśli wystąpił błąd w trakcie parsowania, ustawia `*err = true`.
 * @param[in] str : napis
 * @param[out] endptr : wskaźnik na pierwszy niesparsowany znak z @p str
 * @param[out] err : wskaźnik na informację o błędzie
 * @return sparsowany wykładnik z @p str
 */
static poly_exp_t MonoExpParse(const char *str, char **endptr, bool *err) {
    assert(str != NULL && err != NULL && endptr != NULL);
    errno = 0;
    long l = strtol(str, endptr, DECIMAL_BASE);
    // Wykładnik może być tylko nieujemną liczbą z zakresu int i
    // nie może być w tekście poprzedzony znakiem '+'.
    *err |= !(isdigit(*str) || (l == 0 && *str == '-')) || str == *endptr || errno != 0 || l < 0 || l > INT_MAX;
    return (poly_exp_t) l;
}

static Poly PolyParseHelper(const char *str, char **endptr, bool *err);

/**
 * Parsuje tekst na jednomian.
 * Po wykonaniu, `*endptr` wskazuje o jedną pozycję dalej niż sparsowany napis.
 * Jeśli wystąpił błąd w trakcie parsowania, ustawia `*err = true`.
 * @param[in] str : napis
 * @param[out] endptr : wskaźnik na pierwszy niesparsowany znak z @p str
 * @param[out] err : wskaźnik na informację o błędzie
 * @return sparsowany jednomian z @p str
 */
static Mono MonoParse(const char *str, char **endptr, bool *err) {
    assert(str != NULL && err != NULL);
    Mono res;
    // Jednomian powinien zaczynać się znakiem '('.
    *err |= str[0] != '(';
    const char *temp = str + 1;
    Poly p = PolyParseHelper(temp, endptr, err);
    // Współczynnik i wykładnik jednomianu,
    // powinien rozdzielać przecinek.
    *err |= **endptr != ',';
    if (!*err) {
        // Po przecinku parsowany jest wykładnik.
        temp = *endptr + 1;
        poly_exp_t exp = MonoExpParse(temp, endptr, err);
        // Jednomian powinien kończyć się znakiem ')'.
        *err |= **endptr != ')';
        ++*endptr;
        res = (Mono) {.p = p, .exp = exp};
    }
    if (*err) {
        PolyDestroy(&p);
        res = (Mono) {PolyZero(), 0};
    }
    return res;
}

/**
 * Parsuje tekst na wielomian współczynnikowy.
 * Po wykonaniu, `*endptr` wskazuje o jedną pozycję dalej niż sparsowany napis.
 * Jeśli wystąpił błąd w trakcie parsowania, ustawia `*err = true`.
 * @param[in] str : napis
 * @param[out] endptr : wskaźnik na pierwszy niesparsowany znak z @p str
 * @param[out] err : wskaźnik na informację o błędzie
 * @return sparsowany współczynnik z @p str
 */
Poly PolyCoeffParse(const char *str, char **endptr, bool *err) {
    assert(str != NULL && err != NULL && endptr != NULL);
    errno = 0;
    poly_coeff_t c = strtol(str, endptr, DECIMAL_BASE);
    *err |= *str == '+' || errno != 0;
    return PolyFromCoeff(c);
}

/**
 * Powiększa rozmiar tablicy jednomianów jeśli `count == size`.
 * @param[in,out] monos : wskaźnik na tablicę jednomianów
 * @param[in] count : liczba zajętych pozycji
 * @param[in,out] size : wskaźnik na rozmiar tablicy
 */
static void MonoArrExpand(Mono **monos, size_t count, size_t *size) {
    if (*size == 0) {
        *size = INIT_SIZE;
        *monos = malloc(*size * sizeof(Mono));
        CHECK_PTR(*monos);
    } else if (count == *size) {
        *size *= MULTIPLIER;
        *monos = realloc(*monos, *size * sizeof(Mono));
        CHECK_PTR(*monos);
    }
}

/**
 * Usuwa z pamięci tablicę jednomianów.
 * @param[in] monos : tablica jednomianów
 * @param[in] count : liczba jednomianów w tablicy
 */
static void MonoArrDestroy(Mono *monos, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        MonoDestroy(&monos[i]);
    }
}

/**
 * Parsuje tekst na wielomian.
 * Po wykonaniu, `*endptr` wskazuje na o jedną pozycję dalej niż sparsowany napis.
 * Jeśli wystąpił błąd w trakcie parsowania, ustawia `*err = true`.
 * @param[in] str : napis
 * @param[out] endptr : wskaźnik na pierwszy niesparsowany znak z @p str
 * @param[out] err : wskaźnik na informację o błędzie
 * @return sparsowany wielomian z @p str
 */
static Poly PolyParseHelper(const char *str, char **endptr, bool *err) {
    assert(err != NULL && endptr != NULL && str != NULL);
    Poly r = PolyZero();
    if (isdigit(str[0]) || str[0] == '-') { // Wielomian stały.
        r = PolyCoeffParse(str, endptr, err);
    } else { // Jednomian lub suma jednomianów.
        *err |= str[0] != '(';
        Mono m;
        size_t size = 0;
        size_t count = 0;
        Mono *monos = NULL;
        const char *temp = str;
        while (!*err && *temp != '\n' && *temp != ',') {
            // Jeśli wielomian jest współczynnikiem jednomianu,
            // to kończy się przed znakiem ',',
            // po którym powinien wystąpić wykładnik.
            char c = *temp;
            switch (c) {
                case '(': // Początek jednomianu.
                    m = MonoParse(temp, endptr, err);
                    if (!*err) {
                        MonoArrExpand(&monos, count, &size);
                        monos[count] = m;
                        ++count;
                    }
                    // Następnie parsowane jest to,
                    // czego nie sparsowała funkcja MonoParse().
                    temp = *endptr;
                    *err |= *temp != '\n' && *temp != ',' && *temp != '+';
                    break;
                case '+':
                    // Dokładnie jeden znak '+' rozdziela jednomiany,
                    // których sumą jest wynikowy wielomian.
                    *err |= *(temp + 1) != '(';
                    ++temp;
                    break;
                default:
                    *err = true;
                    ++temp;
            }
        }
        if (!*err) {
            r = PolyAddMonos(count, monos);
        } else {
            MonoArrDestroy(monos, count);
        }
        free(monos);
    }
    return r;
}

Poly PolyParse(char *str, size_t line, ssize_t len, bool *err) {
    assert(str != NULL && str[len - 1] == '\n' && err != NULL);
    char *endparsed = str;
    Poly p = PolyParseHelper(str, &endparsed, err);
    *err |= endparsed != str + len - 1;
    if (*err) {
        fprintf(stderr, "ERROR %zu WRONG POLY\n", line);
        PolyDestroy(&p);
        return PolyZero(); // Atrapa.
    } else {
        return p;
    }
}