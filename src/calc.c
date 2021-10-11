/** @file
 * Kalkulator działający na wielomianach i stosujący odwrotną notację polską.
 *
 * @author Mateusz Sulimowicz <ms429603@students.mimuw.edu.pl>
 * @date 2021
 */

/** Makro potrzebne do korzystania z GNU C Library. */
#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "stack.h"

/** Długość nazwy polecenia "DEG_BY" */
#define DEG_BY_LENGTH 6

/** Długość nazwy polecenia "AT" */
#define AT_LENGTH 2

/** Długość nazwy polecenia "COMPOSE" */
#define COMPOSE_LENGTH 7

/**
 * Wstawia na wierzchołek stosu wielomian
 * tożsamościowo równy zeru.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 */
static void CommandZeroExec(Stack s) {
    Poly p = PolyZero();
    StackPush(s, &p);
}

/**
 * Sprawdza, czy wielomian na wierzchołku stosu
 * jest współczynnikiem – wypisuje na standardowe wyjście 0 lub 1.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandIsCoeffExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackTop(s, err);
        printf("%d\n", PolyIsCoeff(&p));
    } else {
        *err = true;
    }
}

/**
 * Sprawdza, czy wielomian na wierzchołku stosu
 * jest tożsamościowo równy zeru – wypisuje na standardowe wyjście 0 lub 1.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandIsZeroExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackTop(s, err);
        printf("%d\n", PolyIsZero(&p));
    } else {
        *err = true;
    }
}

/**
 * Wstawia na stos kopię wielomianu z wierzchołka.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandCloneExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackTop(s, err);
        Poly q = PolyClone(&p);
        StackPush(s, &q);
    } else {
        *err = true;
    }
}

/**
 * Dodaje dwa wielomiany z wierzchu stosu,
 * usuwa je i wstawia na wierzchołek stosu ich sumę.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandAddExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 2) {
        Poly p = StackPop(s, err);
        Poly q = StackPop(s, err);
        Poly r = PolyAdd(&p, &q);
        StackPush(s, &r);
        PolyDestroy(&q);
        PolyDestroy(&p);
    } else {
        *err = true;
    }
}

/**
 * Mnoży dwa wielomiany z wierzchu stosu,
 * usuwa je i wstawia na wierzchołek stosu ich iloczyn.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandMulExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 2) {
        Poly p = StackPop(s, err);
        Poly q = StackPop(s, err);
        Poly r = PolyMul(&p, &q);
        StackPush(s, &r);
        PolyDestroy(&q);
        PolyDestroy(&p);
    } else {
        *err = true;
    }
}

/**
 * Neguje wielomian na wierzchołku stosu.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandNegExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackPop(s, err);
        Poly r = PolyNeg(&p);
        StackPush(s, &r);
        PolyDestroy(&p);
    } else {
        *err = true;
    }
}

/**
 * Odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem,
 * usuwa je i wstawia na wierzchołek stosu różnicę.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandSubExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 2) {
        Poly p = StackPop(s, err);
        Poly q = StackPop(s, err);
        Poly r = PolySub(&p, &q);
        StackPush(s, &r);
        PolyDestroy(&q);
        PolyDestroy(&p);
    } else {
        *err = true;
    }
}

/**
 * Sprawdza, czy dwa wielomiany na wierzchu stosu
 * są równe – wypisuje na standardowe wyjście 0 lub 1.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandIsEqExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 2) {
        Poly p = StackPop(s, err);
        Poly q = StackTop(s, err);
        printf("%d\n", PolyIsEq(&p, &q));
        StackPush(s, &p);
    } else {
        *err = true;
    }
}

/**
 * Wypisuje na standardowe wyjście stopień wielomianu
 * (−1 dla wielomianu tożsamościowo równego zeru).
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandDegExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackTop(s, err);
        printf("%d\n", PolyDeg(&p));
    } else {
        *err = true;
    }
}

/**
 * Wypisuje na standardowe wyjście stopień wielomianu
 * ze względu na zadaną zmienną
 * (−1 dla wielomianu tożsamościowo równego zeru).
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos kalkulatora
 * @param[in] var_idx : indeks zmiennej
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandDegByExec(Stack s, size_t var_idx, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackTop(s, err);
        printf("%d\n", PolyDegBy(&p, var_idx));
    } else {
        *err = true;
    }
}

/**
 * Wylicza wartość wielomianu w zadanym punkcie,
 * usuwa wielomian z wierzchołka i wstawia na stos wynik operacji.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[in] x : punkt
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandAtExec(Stack s, poly_coeff_t x, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackPop(s, err);
        Poly r = PolyAt(&p, x);
        StackPush(s, &r);
        PolyDestroy(&p);
    } else {
        *err = true;
    }
}

/**
 * Wypisuje na standardowe wyjście wielomian z wierzchołka stosu,
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandPrintExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackTop(s, err);
        PolyPrint(&p);
    } else {
        *err = true;
    }
}

/**
 * Usuwa wielomian z wierzchołka stosu.
 * W przypadku wykrycia błędu, ustawia `*err = true`.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[out] err : wskaźnik na informację o błędzie
 */
static void CommandPopExec(Stack s, bool *err) {
    if (StackPolyCount(s) >= 1) {
        Poly p = StackPop(s, err);
        PolyDestroy(&p);
    } else {
        *err = true;
    }
}

/**
* Wykonuje składanie wielomianów
* W przypadku wykrycia błędu, ustawia `*err = true`.
* @param[in,out] s : wskaźnik na stos kalkulatora
* @param[in] k : liczba wielomianów ze stosu, z którymi ma być złożony wielomian z wierzchołka
* @param[out] err : wskaźnik na informację o błędzie
*/
static void CommandComposeExec(Stack s, size_t k, bool *err) {
    if (StackPolyCount(s) > k) {
        Poly p = StackPop(s, err);
        Poly *q = malloc(k * sizeof(Poly));
        CHECK_PTR(q);
        for (size_t i = 0; i < k; ++i) {
            q[k - 1 - i] = StackPop(s, err);
        }
        Poly r = PolyCompose(&p, k, q);
        StackPush(s, &r);
        PolyDestroy(&p);
        for (size_t i = 0; i < k; ++i) {
            PolyDestroy(&q[i]);
        }
        free(q);
    } else {
        *err = true;
    }
}

/**
 * Parsuje wiersz na polecenie kalkulatora i je wykonuje.
 * W przypadku niepowodzenia, odpowiednia informacja
 * jest wypisywana na standardowe wyjście błędu.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * @param[in] str : napis
 * @param[in] line : numer wiersza
 * @param[in] len : długość wiersza
 */
static void CommandExec(Stack s, char *str, size_t line, ssize_t len) {
    assert(str != NULL && isalpha(*str) && str[len - 1] == '\n');
    errno = 0;
    char *name = str;
    char *val = NULL;
    char *endptr = str;
    bool err = false; // Jeśli na stosie jest za mało wielomianów, by wykonać polecenie, to `err = true`;
    str[len - 1] = '\0';
    if (memcmp(name, "ZERO", len) == 0) {
        CommandZeroExec(s);
    } else if (memcmp(name, "IS_COEFF", len) == 0) {
        CommandIsCoeffExec(s, &err);
    } else if (memcmp(name, "IS_ZERO", len) == 0) {
        CommandIsZeroExec(s, &err);
    } else if (memcmp(name, "CLONE", len) == 0) {
        CommandCloneExec(s, &err);
    } else if (memcmp(name, "ADD", len) == 0) {
        CommandAddExec(s, &err);
    } else if (memcmp(name, "MUL", len) == 0) {
        CommandMulExec(s, &err);
    } else if (memcmp(name, "NEG", len) == 0) {
        CommandNegExec(s, &err);
    } else if (memcmp(name, "SUB", len) == 0) {
        CommandSubExec(s, &err);
    } else if (memcmp(name, "IS_EQ", len) == 0) {
        CommandIsEqExec(s, &err);
    } else if (memcmp(name, "DEG", len) == 0) {
        CommandDegExec(s, &err);
    } else if (memcmp(name, "PRINT", len) == 0) {
        CommandPrintExec(s, &err);
    } else if (memcmp(name, "POP", len) == 0) {
        CommandPopExec(s, &err);
    } else if (memcmp(name, "DEG_BY", DEG_BY_LENGTH) == 0) {
        size_t var_idx;
        val = str + DEG_BY_LENGTH + 1;
        var_idx = strtoul(val, &endptr, DECIMAL_BASE);
        if (isspace(str[DEG_BY_LENGTH]) || DEG_BY_LENGTH + 1 == len) {
            if (str[DEG_BY_LENGTH] == ' ' && (isdigit(*val) || (*val == '-' && var_idx == 0)) && errno == 0 &&
                endptr == str + len - 1) {
                CommandDegByExec(s, var_idx, &err);
            } else {
                fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
            }
        } else {
            fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        }
    } else if (memcmp(name, "AT", AT_LENGTH) == 0) {
        poly_coeff_t x;
        val = str + AT_LENGTH + 1;
        x = strtol(val, &endptr, DECIMAL_BASE);
        if (isspace(str[AT_LENGTH]) || AT_LENGTH + 1 == len) {
            if (str[AT_LENGTH] == ' ' && (isdigit(*val) || *val == '-') && errno == 0 && endptr == str + len - 1) {
                CommandAtExec(s, x, &err);
            } else {
                fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
            }
        } else {
            fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        }
    } else if (memcmp(name, "COMPOSE", COMPOSE_LENGTH) == 0) {
        val = str + COMPOSE_LENGTH + 1;
        size_t k = strtoul(val, &endptr, DECIMAL_BASE);
        if (isspace(str[COMPOSE_LENGTH]) || COMPOSE_LENGTH + 1 == len) {
            if (str[COMPOSE_LENGTH] == ' ' && (isdigit(*val) || (*val == '-' && k == 0)) && errno == 0 &&
                endptr == str + len - 1) {
                CommandComposeExec(s, k, &err);
            } else {
                fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
            }
        } else {
            fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        }
    } else {
        fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
    }
    if (err) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
    }
}

/**
 * Sprowadza wiersz do postaci normalnej,
 * czyli jeśli nie kończy się końcem wiersza,
 * to wstawia na jego koniec ten znak.
 * @param[in] str : wskaźnik na wiersz tekstu
 * @param[in] len : wskaźnik na długość tekstu
 */
void NormalizeLine(char **str, ssize_t *len) {
    assert(*len > 0);
    if ((*str)[*len - 1] != '\n') {
        *str = realloc(*str, (*len + 2) * sizeof(char)); // +2 bo `\n` i jeszcze `\0`.
        CHECK_PTR(str);
        (*str)[*len] = '\n';
        (*str)[*len + 1] = '\0';
        ++(*len); // Długość nie uwzględnia znaku `\0` na końcu.
    }
}

/**
 * Parsuje polecenia ze standardowego wejścia i
 * wykonuje je na kalkulatorze.
 * @param[in,out] s : wskaźnik na stos kalkulatora
 * */
static void CalcRun(Stack s) {
    char *str = NULL;
    ssize_t len = 0;
    size_t n = 0;
    size_t line = 0;
    errno = 0;
    while ((len = getline(&str, &n, stdin)) > 0) {
        ++line;
        if ((len == 1 && str[0] == '\n') || str[0] == '#') {
            // Linia zaczynająca się znakiem '#' lub pusta, jest ignorowana.
            continue;
        } else {
            NormalizeLine(&str, &len);
            char c = *str;
            if (isalpha(c)) {
                CommandExec(s, str, line, len);
            } else {
                bool err = false;
                Poly p = PolyParse(str, line, len, &err);
                if (!err) {
                    StackPush(s, &p);
                }
            }
        }
        errno = 0;
    }
    if (len < 0 && errno == ENOMEM) {
        // Wystąpił błąd alokacji pamięci.
        exit(1);
    }
    free(str);
}

/**
 * Realizacja kalkulatora.
 * @return kod zakończenia programu
 * */
int main(void) {
    Stack s = NULL;
    StackInit(&s);

    CalcRun(s);

    StackDestroy(s);
    return 0;
}