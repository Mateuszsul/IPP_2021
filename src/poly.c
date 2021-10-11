/** @file
  Implementacja operacji na wielomianach rzadkich wielu zmiennych.

  Każdy wielomian utworzony za pomocą dostępnych funkcji jest w możliwie najprostszej postaci, czyli:
    - Wielomian stały ma pustą tablicę jednomianów (.arr == NULL).
    - Tablica jednomianów wielomianu niestałego jest posortowana ściśle rosnąco względem wartości wykładnika oraz
    nie występują w niej jednomiany o zerowym współczynniku.

  @author Mateusz Sulimowicz <ms429603@students.mimuw.edu.pl>
  @date 2021
*/

#include <stdlib.h>
#include <stdbool.h>
#include "poly.h"
#include "stdio.h"

/**
 * Sprawdza, czy jednomiany wielomianu
 * są posortowane rosnąco po wartości wykładnika.
 * @param[in] p : wielomian @f$ p @f$,
 * @return Czy tablica jednomianów wielomianu @f$p@f$
 * jest posortowana ściśle rosnąco?
 */
static bool PolyIsSorted(const Poly *p) {
    assert(p != NULL);
    bool res = true;
    if (p->arr != NULL) {
        size_t i = 0;
        poly_coeff_t last_exp = -1;
        while (res && i < p->size) {
            res = (i == 0 || p->arr[i].exp > last_exp);
            last_exp = MonoGetExp(&p->arr[i]);
            ++i;
        }
    }
    return res;
}

static bool PolyIsSimple(const Poly *p) __attribute__((unused));

/**
 * Sprawdza, czy wielomian jest w możliwie najprostszej postaci.
 * Sprawdza, czy wielomian stały ma tablicę arr = NULL, a wielomian
 * niestały ma tablicę jednomianów posortowaną
 * ściśle rosnąco względem wartości wykładnika. Sprawdza również czy
 * w tablicy jednomianów nie występują niepotrzebne jednomiany zerowe.
 * @param[in] p : wielomian @f$ p @f$,
 * @return Czy @f$ p @f$ jest w najprostszej postaci?
 */
static bool PolyIsSimple(const Poly *p) {
    assert(p != NULL);
    bool res = PolyIsSorted(p);
    if (res && p->arr != NULL) {
        size_t i = 0;
        int exp_0 = 0;
        while (res && i < p->size) {
            exp_0 += (MonoGetExp(&p->arr[i]) == 0) && PolyIsCoeff(&p->arr[i].p);
            res = PolyIsSimple(&p->arr[i].p) &&
                  !PolyIsZero(&p->arr[i].p) &&
                  (exp_0 == 0 || (exp_0 == 1 && p->size > 1));
            ++i;
        }
    }
    return res;
}

/**
 * Porównuje jednomiany po wartości wykładnika.
 * @param[in] m : jednomian,
 * @param[in] n : jednomian,
 * @return -1 jeśli wykładnik @f$ m @f$ @f$<@f$ wykładnik @f$ n @f$,
 * 0 jeśli te wykładniki są równe,
 * 1 jeśli wykładnik  @f$ m @f$ @f$>@f$ wykładnik @f$ n @f$.
 */
static int MonoCmp(const void *m, const void *n) {
    Mono *m1 = (Mono *) m;
    Mono *n1 = (Mono *) n;
    if (MonoGetExp(m1) < MonoGetExp(n1)) {
        return -1;
    } else if (MonoGetExp(m1) == MonoGetExp(n1)) {
        return 0;
    } else {
        return 1;
    }
}

void PolyDestroy(Poly *p) {
    if (p != NULL && p->arr != NULL) {
        for (size_t i = 0; i < p->size; ++i) {
            MonoDestroy(&p->arr[i]);
        }
        free(p->arr);
    }
}

Poly PolyClone(const Poly *p) {
    assert(p != NULL);
    if (p->arr == NULL) {
        return (Poly) {.coeff = p->coeff, .arr = NULL};
    } else {
        Poly p1 = (Poly) {.size = p->size, .arr = NULL};
        p1.arr = malloc(p->size * sizeof(Mono));
        CHECK_PTR(p1.arr);
        for (size_t i = 0; i < p1.size; ++i) {
            p1.arr[i] = MonoClone(&p->arr[i]);
        }
        return p1;
    }
}

/**
 * Sprowadza wielomian do jak najprostszej postaci.
 * Wielomiany stałe przekształca do postaci współczynnikowej.
 * Sumuje niezerowe jednomiany o jednakowych wykładnikach.
 * Po uproszczeniu, tablica jednomianów jest posortowana
 * ściśle rosnąco względem wartości wykładnika.
 * @param[in] p : wielomian @f$p@f$
 * */
static void PolySimplify(Poly *p) {
    assert(p != NULL);
    if (!PolyIsCoeff(p)) {
        size_t new_size = 0;
        // Z tablicy jednomianów wielomianu p chcemy pozbyć się
        // jednomianów o współczynniku równym 0,
        // więc zliczamy niezerowe jednomiany.
        for (size_t i = 0; i < p->size; ++i) {
            if (!PolyIsZero(&p->arr[i].p)) {
                ++new_size;
            }
        }
        if (new_size == 0) { // p jest wielomianem zerowym.
            PolyDestroy(p);
            *p = PolyZero();
        } else if (new_size == 1 && MonoGetExp(&p->arr[0]) == 0 &&
                   PolyIsCoeff(&p->arr[0].p) && !PolyIsZero(&p->arr[0].p)) { // p jest wielomianem stałym.
            poly_coeff_t c = p->arr[0].p.coeff;
            PolyDestroy(p);
            *p = PolyFromCoeff(c);
        } else { // Po uproszczeniu p nie jest wielomianem stałym.
            // Przepisujemy zawartość tablicy jednomianów tak,
            // aby znajdowały się w niej tylko niezerowe jednomiany.
            p->size = new_size;
            size_t new_i = 0;
            size_t i = 0;
            while (new_i < new_size) {
                if (!PolyIsZero(&p->arr[i].p)) {
                    p->arr[new_i] = p->arr[i];
                    ++new_i;
                }
                ++i;
            }
        }
        assert(PolyIsSimple(p));
    }
}

static Poly PolyAddCoeff(const Poly *p, const Poly *c);

/**
 * Dodaje do współczynnika jednomianu wielomian stały/współczynnik.
 * @param[in] m : jednomian @f$m@f$
 * @param[in] c : wielomian stale równy @f$c@f$
 * @return jednomian o współczynniku będącym sumą @f$ c@f$ i współczynnika @f$ m@f$, o wykładniku
 * takim jaki wykładnik @f$ m@f$.
 */
static Mono MonoAddCoeff(const Mono *m, const Poly *c) {
    return (Mono) {.p = PolyAddCoeff(&m->p, c), .exp = m->exp};
}

/**
 * Dodaje do wielomianu wielomian stały/współczynnik.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] c : wielomian stały @f$c@f$
 * @return @f$p + c@f$
 */
static Poly PolyAddCoeff(const Poly *p, const Poly *c) {
    assert(p != NULL && c != NULL);
    assert(PolyIsSimple(p) && PolyIsCoeff(c));
    Poly r;
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff + c->coeff);
    } else if (MonoGetExp(&p->arr[0]) == 0) {
        r = PolyClone(p);
        Mono temp = r.arr[0];
        r.arr[0] = MonoAddCoeff(&temp, c);
        MonoDestroy(&temp);
    } else {
        // Jeśli p nie zawiera jednomianu o wykładniku 0, wstawiamy na początek tablicy jednomianów
        // jednomian postaci c * x^0, a pozostałe jednomiany zostają przesunięte o 1 indeks w górę.
        r.size = p->size + 1;
        r.arr = malloc(r.size * sizeof(Mono));
        CHECK_PTR(r.arr);
        Poly new = PolyClone(c);
        (r.arr)[0] = (Mono) {.p = new, .exp = 0};
        for (size_t i = 1; i < r.size; ++i) {
            (r.arr)[i] = MonoClone(&p->arr[i - 1]);
        }
    }
    PolySimplify(&r);
    return r;
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    assert(PolyIsSimple(p) && PolyIsSimple(q));
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff + q->coeff);
    } else if (PolyIsCoeff(p)) {
        return PolyAddCoeff(q, p);
    } else if (PolyIsCoeff(q)) {
        return PolyAddCoeff(p, q);
    } else {
        // Suma dwóch wielomianów p, q,
        // składa się z co najwyżej p->size + q->size jednomianów.
        Poly r = (Poly) {.size = p->size + q->size, .arr = NULL};
        r.arr = malloc(r.size * sizeof(Mono));
        CHECK_PTR(r.arr);
        size_t p_i = 0;
        size_t q_i = 0;
        size_t r_i = 0;
        Mono temp;
        while (r_i < r.size && (p_i < p->size || q_i < q->size)) {
            // Wpisujemy do tablicy jednomianów wielomianu r jednomiany tak, aby
            // wynikowy wielomian miał wykładniki jednomianów posortowane rosnąco.
            if (p_i < p->size && q_i < q->size) {
                poly_exp_t p_exp = MonoGetExp(&p->arr[p_i]);
                poly_exp_t q_exp = MonoGetExp(&q->arr[q_i]);
                if (p_exp < q_exp) {
                    r.arr[r_i] = MonoClone(&p->arr[p_i]);
                    ++p_i;
                } else if (p_exp > q_exp) {
                    r.arr[r_i] = MonoClone(&q->arr[q_i]);
                    ++q_i;
                } else {
                    temp = MonoAdd(&p->arr[p_i], &q->arr[q_i]);
                    r.arr[r_i] = temp;
                    ++p_i;
                    ++q_i;
                }
            } else if (p_i < p->size) {
                r.arr[r_i] = MonoClone(&p->arr[p_i]);
                ++p_i;
            } else if (q_i < q->size) {
                r.arr[r_i] = MonoClone(&q->arr[q_i]);
                ++q_i;
            }
            ++r_i;
        }
        r.size = r_i;
        PolySimplify(&r);
        return r;
    }
}

Poly PolyAddMonos(size_t count, const Mono monos[]) { //TODO: przetestować!
    if (count == 0 || monos == NULL) {
        return PolyZero();
    } else {
        Mono *arr = malloc(count * sizeof(Mono));
        CHECK_PTR(arr);
        for (size_t i = 0; i < count; ++i) {
            arr[i] = monos[i];
        }
        return PolyOwnMonos(count, arr);
    }
}

Poly PolyOwnMonos(size_t count, Mono *monos) { //TODO: przetestować!
    if (count == 0 || monos == NULL) {
        return PolyZero();
    } else {
        Poly p = (Poly) {.size = count, .arr  = monos};
        if (!PolyIsSorted(&p)) {
            qsort(p.arr, p.size, sizeof(Mono), MonoCmp);
        }
        size_t last_exp_i = 0;
        size_t new_size = 1;
        // Dodajemy do siebie jednomiany o jednakowych wykładnikach.
        for (size_t i = 1; i < p.size; ++i) {
            if (MonoGetExp(&p.arr[i]) == MonoGetExp(&p.arr[last_exp_i])) {
                Mono m = p.arr[last_exp_i];
                Mono n = p.arr[i];
                p.arr[last_exp_i] = MonoAdd(&m, &n);
                MonoDestroy(&m);
                MonoDestroy(&n);
            } else {
                ++new_size;
                last_exp_i = i;
            }
        }
        p.size = new_size;
        size_t new_i = 0;
        size_t old_i = 1;
        // Wpisujemy do tablicy arr sumy jednomianów o jednakowych wykładnikach tak,
        // aby w wynikowym wielomianie tablica arr była posortowana ściśle rosnąco
        // po wartości wykładnika.
        while (new_i + 1 < new_size) {
            if (MonoGetExp(&p.arr[old_i]) > MonoGetExp(&p.arr[new_i])) {
                ++new_i;
                p.arr[new_i] = p.arr[old_i];
            }
            ++old_i;
        }
        PolySimplify(&p);
        assert(PolyIsSimple(&p));
        return p;
    }
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) { //TODO: przetestować!
    if (count == 0 || monos == NULL) {
        return PolyZero();
    } else {
        Mono *arr = malloc(count * sizeof(Mono));
        CHECK_PTR(arr);
        for (size_t i = 0; i < count; ++i) {
            arr[i] = MonoClone(&monos[i]);
        }
        return PolyOwnMonos(count, arr);
    }
}

static Poly PolyMulByCoeff(const Poly *p, const Poly *c);

/**
 * Mnoży jednomian przez stałą.
 * @param[in] m : jednomian @f$m@f$
 * @param[in] c : stała @f$c@f$
 * @return @f$m \cdot c@f$
 */
static Mono MonoMulByCoeff(const Mono *m, const Poly *c) {
    return (Mono) {.p = PolyMulByCoeff(&m->p, c), .exp = MonoGetExp(m)};
}

/**
 * Mnoży wielomian przez stałą.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] c : wielomian stały @f$c@f$
 * @return @f$p \cdot c@f$
 */
static Poly PolyMulByCoeff(const Poly *p, const Poly *c) {
    assert(p != NULL && c != NULL);
    assert(PolyIsSimple(p));
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff * c->coeff);
    } else {
        Poly r = (Poly) {.size = p->size, .arr = NULL};
        r.arr = malloc(r.size * sizeof(Mono));
        CHECK_PTR(r.arr);
        for (size_t i = 0; i < r.size; ++i) {
            r.arr[i] = MonoMulByCoeff(&p->arr[i], c);
        }
        PolySimplify(&r);
        return r;
    }
}

/**
 * Mnoży dwa jednomiany.
 * @param[in] m : jednomian @f$m@f$
 * @param[in] n : jednomian @f$n@f$
 * @return @f$m \cdot n@f$
 */
static Mono MonoMul(const Mono *m, const Mono *n) {
    return (Mono) {.p = PolyMul(&m->p, &n->p),
            .exp = MonoGetExp(m) + MonoGetExp(n)};
}

Poly PolyMul(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff * q->coeff);
    } else if (PolyIsCoeff(q)) {
        return PolyMulByCoeff(p, q);
    } else if (PolyIsCoeff(p)) {
        return PolyMulByCoeff(q, p);
    } else {
        // W wyniku mnożenia dwóch wielomianów,
        // otrzymujemy wielomian o ilości jednomianów nie większej
        // niż p->size * q->size.
        Mono *monos = malloc(p->size * q->size * sizeof(Mono));
        CHECK_PTR(monos);
        for (size_t i = 0; i < p->size; ++i) {
            for (size_t j = 0; j < q->size; ++j) {
                // Przypisujemy jednomiany do monos jak
                // wartości do "dwuwymiarowej" tablicy.
                monos[i * (q->size) + j] = MonoMul(&p->arr[i], &q->arr[j]);
            }
        }
        Poly r = PolyAddMonos(p->size * q->size, monos);
        PolySimplify(&r);
        free(monos);
        return r;
    }
}

/**
 * Zwraca jednomian o przeciwnym współczynniku.
 * @param[in] m : jednomian @f$m@f$
 * @return @f$-m@f$
 */
static Mono MonoNeg(const Mono *m) {
    return (Mono) {.p = PolyNeg(&m->p), .exp = MonoGetExp(m)};
}

Poly PolyNeg(const Poly *p) {
    assert(p != NULL);
    if (PolyIsCoeff(p)) {
        return (Poly) {.coeff = (-1) * (p->coeff), .arr = NULL};
    } else {
        Poly r = (Poly) {.size = p->size, .arr = NULL};
        r.arr = malloc(p->size * sizeof(Mono));
        CHECK_PTR(r.arr);
        for (size_t i = 0; i < p->size; ++i) {
            r.arr[i] = MonoNeg(&p->arr[i]);
        }
        return r;
    }
}

Poly PolySub(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    Poly temp = PolyNeg(q);
    Poly r = PolyAdd(p, &temp);
    PolyDestroy(&temp);
    return r;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    assert(p != NULL);
    assert(PolyIsSimple(p));
    if (var_idx == 0 || PolyIsCoeff(p)) {
        // Wielomian stały jest stały bez względu na indeks zmiennej.
        if (PolyIsZero(p)) {
            return -1;
        } else if (!PolyIsCoeff(p)) {
            // W posortowanej tablicy jednomianów,
            // jednomian o najwyższym wykładniku
            // znajduje się na jej końcu.
            return p->arr[p->size - 1].exp;
        } else {
            return 0;
        }
    } else if (var_idx > 0) {
        poly_exp_t exp_max = -1;
        for (size_t i = 0; i < p->size; ++i) {
            poly_exp_t exp_temp = PolyDegBy(&p->arr[i].p, var_idx - 1);
            if (exp_max < exp_temp) {
                exp_max = exp_temp;
            }
        }
        return exp_max;
    } else {
        return 0;
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    assert(p != NULL);
    assert(PolyIsSimple(p));
    if (PolyIsZero(p)) {
        return -1;
    } else if (PolyIsCoeff(p)) {
        return 0;
    } else {
        poly_exp_t exp_max = -1;
        for (size_t i = 0; i < p->size; ++i) {
            poly_exp_t exp_temp;
            if (PolyIsCoeff(&p->arr[i].p)) {
                exp_temp = MonoGetExp(&p->arr[i]);
            } else {
                // Ogólny stopień jednomianu jest sumą
                // jego wykładnika i stopnia współczynnika.
                exp_temp = MonoGetExp(&p->arr[i]) + PolyDeg(&p->arr[i].p);
            }
            if (exp_max < exp_temp) {
                exp_max = exp_temp;
            }
        }
        return exp_max;
    }
}

/**
 * Sprawdza równość dwóch jednomianów.
 * @param[in] m : jednomian @f$m@f$
 * @param[in] n : jednomian @f$n@f$
 * @return @f$m = n@f$
 */
static inline bool MonoIsEq(const Mono *m, const Mono *n) {
    return (m->exp == n->exp && PolyIsEq(&m->p, &n->p));
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    assert(PolyIsSimple(p) && PolyIsSimple(q));
    bool r;
    /// Wielomiany @f$ p @f$ i @f$ q @f$ są w najprostszej postaci,
    /// więc mają jednoznaczną reprezentacje.
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        r = (p->coeff == q->coeff);
    } else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        r = (p->size == q->size);
        size_t i = 0;
        while (r && i < p->size) {
            r = MonoIsEq(&p->arr[i], &q->arr[i]);
            ++i;
        }
    } else {
        r = false;
    }
    return r;
}

/**
 * Wykonuje potęgowanie.
 * Odpowiednik algorytmu mnożenia rosyjskich chłopów, dla potęgowania.
 * @param[in] c: stały współczynnik,
 * @param[in] e: wykładnik,
 * @returns @f$c^e@f$
 */
static poly_coeff_t Power(poly_coeff_t c, poly_exp_t e) {
    poly_coeff_t res = 1;
    while (e > 0) {
        if (e & 1) {
            res *= c;
        }
        c *= c;
        e >>= 1;
    }
    return res;
}

/**
 * Zwraca wielomian będący sumą wielu wielomianów.
 * @param[in] p_arr : tablica wielomianów @f$p\_arr@f$
 * @param[in] n : liczba dodawanych wielomianów @f$n@f$
 * @return @f$ p\_arr[0] + p\_arr[1]  + ... + p\_arr[n-1]@f$
 */
static Poly PolyAddMult(const Poly *p_arr, size_t n) {
    if (n > 0) {
        Poly r = PolyClone(&p_arr[0]);
        for (size_t i = 1; i < n; ++i) {
            Poly temp = r;
            r = PolyAdd(&temp, &p_arr[i]);
            PolyDestroy(&temp);
        }
        return r;
    } else {
        return PolyZero();
    }
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    assert(p != NULL);
    assert(PolyIsSimple(p));
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff);
    } else {
        Poly *temp = malloc(p->size * sizeof(Poly));
        CHECK_PTR(temp);
        poly_exp_t prev_e = 0;
        for (size_t i = 0; i < p->size; ++i) {
            Mono m = p->arr[i];
            Poly c = PolyFromCoeff(Power(x, MonoGetExp(&m) - prev_e));
            temp[i] = PolyMulByCoeff(&m.p, &c);
        }
        Poly r = PolyAddMult(temp, p->size);
        for (size_t i = 0; i < p->size; ++i) {
            PolyDestroy(&temp[i]);
        }
        free(temp);
        PolySimplify(&r);
        return r;
    }
}

static void PolyPrintHelper(const Poly *p);

/**
 * Wypisuje na standardowe wyjście
 * jednomian w najprostszej postaci.
 * @param[in] m : jednomian @f$ m @f$
 */
static void MonoPrint(const Mono *m) {
    assert(m != NULL);
    printf("(");
    PolyPrintHelper(&m->p);
    printf(",%d", MonoGetExp(m));
    printf(")");
}

/**
 * Umożliwia poprawne wypisywanie
 * znaku końca wiersza itp., po wielomianie.
 * @param[in] p : wielomian @f$ p @f$
 */
static void PolyPrintHelper(const Poly *p) {
    assert(p != NULL);
    if (PolyIsCoeff(p)) {
        printf("%ld", p->coeff);
    } else {
        for (size_t i = 0; i < p->size - 1; ++i) {
            MonoPrint(&p->arr[i]);
            printf("+");
        }
        MonoPrint(&p->arr[p->size - 1]);
    }
}

void PolyPrint(const Poly *p) {
    assert(p != NULL);
    PolyPrintHelper(p);
    printf("\n");
}

/**
 * Wykonuje szybkie potęgowanie na wielomianach.
 * @param[in] p: wielomian @f$ p @f$,
 * @param[in] e: wykładnik,
 * @returns @f$p^e@f$
 */
static Poly PolyPower(const Poly *p, poly_exp_t e) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(Power(p->coeff, e));
    } else {
        Poly res = PolyFromCoeff(1);
        Poly base = PolyClone(p);
        Poly temp;
        while (e > 0) {
            if (e & 1) {
                temp = res;
                res = PolyMul(&res, &base);
                PolyDestroy(&temp);
            }
            temp = base;
            base = PolyMul(&base, &base);
            PolyDestroy(&temp);
            e >>= 1;
        }
        PolyDestroy(&base);
        return res;
    }
}

/**
 * Wykonuje operację składania jednomianu z wielomianami.
 * @param[in] m : jednomian @f$ m @f$
 * @param[in] k : rozmiar tablicy @f$ q @f$
 * @param[in] q : tablica wielomianów @f$ q @f$
 * @param[in] inner : wstawiany za zmienną wielomian w potędze równej wykładnikowi @f$ m @f$
 * @return wielomian powstały przez złożenie @f$ m @f$ z tablicą @f$ q @f$
 */
static Poly MonoCompose(const Mono *m, size_t k, const Poly q[], const Poly *inner) {
    assert(m != NULL && k != 0 && q != NULL);
    Poly coeff = PolyCompose(&m->p, k - 1, q + 1);
    Poly r = PolyMul(&coeff, inner);
    PolyDestroy(&coeff);
    return r;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    assert(p != NULL);
    Poly r = PolyZero();
    if (!PolyIsCoeff(p)) {
        if (k == 0 || q == NULL) {
            Poly at_zero = PolyAt(p, 0);
            r = PolyCompose(&at_zero, 0, NULL);
            PolyDestroy(&at_zero);
        } else {
            Poly prev_inner = PolyFromCoeff(1);
            Poly new_inner = PolyFromCoeff(1);
            poly_exp_t prev_exp = 0;
            Poly temp;
            Poly composed;
            for (size_t i = 0; i < p->size; ++i) {
                temp = PolyPower(q, MonoGetExp(&p->arr[i]) - prev_exp);
                new_inner = PolyMul(&temp, &prev_inner);
                PolyDestroy(&prev_inner);
                PolyDestroy(&temp);
                prev_exp = MonoGetExp(&p->arr[i]);
                prev_inner = new_inner;

                composed = MonoCompose(&p->arr[i], k, q, &new_inner);
                temp = r;
                r = PolyAdd(&r, &composed);
                PolyDestroy(&temp);
                PolyDestroy(&composed);
            }
            PolyDestroy(&new_inner);
        }
    } else {
        r = PolyClone(p);
    }
    return r;
}
