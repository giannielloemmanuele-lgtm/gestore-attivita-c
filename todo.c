/*
 * gestore-attivita - un semplice gestore di attivita' (to-do list) da terminale.
 *
 * Dimostra in un solo file: struct, array dinamici (malloc/realloc/free),
 * puntatori, gestione della memoria e I/O su file di testo.
 *
 * Compilazione: gcc -Wall -Wextra -o todo todo.c
 * Esecuzione:   ./todo
 *
 * Licenza: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TESTO          256
#define FILE_DATI          "attivita.txt"
#define CAPACITA_INIZIALE  4

/* Una singola attivita' della lista. */
typedef struct {
    int  id;
    char testo[MAX_TESTO];
    int  completata;   /* 0 = da fare, 1 = fatta            */
    int  priorita;     /* 1 = bassa, 2 = media, 3 = alta     */
} Attivita;

/* Array dinamico di attivita'. */
typedef struct {
    Attivita *elementi;
    size_t    numero;       /* quante attivita' ci sono ora              */
    size_t    capacita;     /* quante ne contiene prima di riallocare    */
    int       prossimo_id;  /* prossimo id da assegnare                  */
} Lista;

/* ------------------------------------------------------------------ */
/* Gestione della lista (memoria dinamica)                            */
/* ------------------------------------------------------------------ */

static void lista_inizializza(Lista *l)
{
    l->elementi = malloc(CAPACITA_INIZIALE * sizeof(Attivita));
    if (l->elementi == NULL) {
        fprintf(stderr, "Errore: memoria insufficiente.\n");
        exit(EXIT_FAILURE);
    }
    l->numero      = 0;
    l->capacita    = CAPACITA_INIZIALE;
    l->prossimo_id = 1;
}

static void lista_libera(Lista *l)
{
    free(l->elementi);
    l->elementi = NULL;
    l->numero   = 0;
    l->capacita = 0;
}

/* Aggiunge un'attivita', raddoppiando la capacita' se l'array e' pieno. */
static void lista_aggiungi(Lista *l, const char *testo, int priorita)
{
    if (l->numero == l->capacita) {
        size_t nuova_cap = l->capacita * 2;
        Attivita *tmp = realloc(l->elementi, nuova_cap * sizeof(Attivita));
        if (tmp == NULL) {
            fprintf(stderr, "Errore: impossibile espandere la memoria.\n");
            return;
        }
        l->elementi = tmp;
        l->capacita = nuova_cap;
    }

    Attivita *a = &l->elementi[l->numero];
    a->id = l->prossimo_id++;
    strncpy(a->testo, testo, MAX_TESTO - 1);
    a->testo[MAX_TESTO - 1] = '\0';
    a->completata = 0;
    a->priorita   = priorita;
    l->numero++;
}

/* Restituisce l'indice dell'attivita' con quell'id, oppure -1. */
static int lista_indice_da_id(const Lista *l, int id)
{
    for (size_t i = 0; i < l->numero; i++) {
        if (l->elementi[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

/* Rimuove per id facendo scorrere gli elementi. 1 = riuscito, 0 = non trovato. */
static int lista_rimuovi(Lista *l, int id)
{
    int idx = lista_indice_da_id(l, id);
    if (idx < 0) {
        return 0;
    }
    for (size_t i = (size_t)idx; i + 1 < l->numero; i++) {
        l->elementi[i] = l->elementi[i + 1];
    }
    l->numero--;
    return 1;
}

static int lista_completa(Lista *l, int id)
{
    int idx = lista_indice_da_id(l, id);
    if (idx < 0) {
        return 0;
    }
    l->elementi[idx].completata = 1;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Stampa                                                             */
/* ------------------------------------------------------------------ */

static const char *etichetta_priorita(int p)
{
    switch (p) {
        case 3:  return "ALTA";
        case 2:  return "media";
        default: return "bassa";
    }
}

static void lista_stampa(const Lista *l)
{
    if (l->numero == 0) {
        printf("\n(nessuna attivita')\n");
        return;
    }
    printf("\n  ID  | Stato | Priorita' | Attivita'\n");
    printf("------+-------+-----------+--------------------------\n");
    for (size_t i = 0; i < l->numero; i++) {
        const Attivita *a = &l->elementi[i];
        printf(" %3d  |  [%c]  | %-9s | %s\n",
               a->id,
               a->completata ? 'x' : ' ',
               etichetta_priorita(a->priorita),
               a->testo);
    }
}

/* ------------------------------------------------------------------ */
/* I/O su file                                                        */
/* ------------------------------------------------------------------ */

static void lista_salva(const Lista *l)
{
    FILE *f = fopen(FILE_DATI, "w");
    if (f == NULL) {
        fprintf(stderr, "Errore: impossibile salvare su %s.\n", FILE_DATI);
        return;
    }
    for (size_t i = 0; i < l->numero; i++) {
        const Attivita *a = &l->elementi[i];
        fprintf(f, "%d|%d|%d|%s\n", a->id, a->completata, a->priorita, a->testo);
    }
    fclose(f);
}

static void lista_carica(Lista *l)
{
    FILE *f = fopen(FILE_DATI, "r");
    if (f == NULL) {
        return; /* nessun file: si parte da lista vuota */
    }

    char riga[MAX_TESTO + 64];
    while (fgets(riga, sizeof(riga), f) != NULL) {
        int  id, completata, priorita;
        char testo[MAX_TESTO];

        /* legge i tre interi e poi il resto della riga come testo */
        if (sscanf(riga, "%d|%d|%d|%255[^\n]",
                   &id, &completata, &priorita, testo) == 4) {
            lista_aggiungi(l, testo, priorita);
            Attivita *a = &l->elementi[l->numero - 1];
            a->id         = id;
            a->completata = completata;
            if (id >= l->prossimo_id) {
                l->prossimo_id = id + 1;
            }
        }
    }
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* Lettura input dell'utente                                          */
/* ------------------------------------------------------------------ */

/* Legge una riga togliendo il newline finale. 1 = letto, 0 = EOF. */
static int leggi_riga(char *buffer, size_t dim)
{
    if (fgets(buffer, (int)dim, stdin) == NULL) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

static int leggi_intero(const char *prompt, int predefinito)
{
    char buffer[64];
    printf("%s", prompt);
    if (!leggi_riga(buffer, sizeof(buffer)) || buffer[0] == '\0') {
        return predefinito;
    }
    return atoi(buffer);
}

/* ------------------------------------------------------------------ */
/* Menu principale                                                    */
/* ------------------------------------------------------------------ */

static void stampa_menu(void)
{
    printf("\n===== GESTORE ATTIVITA' =====\n");
    printf("1) Mostra attivita'\n");
    printf("2) Aggiungi attivita'\n");
    printf("3) Segna come completata\n");
    printf("4) Rimuovi attivita'\n");
    printf("5) Salva ed esci\n");
    printf("Scelta: ");
}

int main(void)
{
    Lista lista;
    lista_inizializza(&lista);
    lista_carica(&lista);

    printf("Benvenuta! Caricate %zu attivita'.\n", lista.numero);

    int in_esecuzione = 1;
    while (in_esecuzione) {
        stampa_menu();

        char scelta[16];
        if (!leggi_riga(scelta, sizeof(scelta))) {
            break; /* EOF, es. Ctrl+D */
        }

        switch (scelta[0]) {
            case '1':
                lista_stampa(&lista);
                break;

            case '2': {
                char testo[MAX_TESTO];
                printf("Testo dell'attivita': ");
                if (leggi_riga(testo, sizeof(testo)) && testo[0] != '\0') {
                    int p = leggi_intero(
                        "Priorita' (1=bassa, 2=media, 3=alta) [1]: ", 1);
                    if (p < 1 || p > 3) {
                        p = 1;
                    }
                    lista_aggiungi(&lista, testo, p);
                    printf("Attivita' aggiunta.\n");
                } else {
                    printf("Testo vuoto: nessuna attivita' aggiunta.\n");
                }
                break;
            }

            case '3': {
                int id = leggi_intero("ID da completare: ", -1);
                printf(lista_completa(&lista, id)
                       ? "Attivita' completata.\n"
                       : "ID non trovato.\n");
                break;
            }

            case '4': {
                int id = leggi_intero("ID da rimuovere: ", -1);
                printf(lista_rimuovi(&lista, id)
                       ? "Attivita' rimossa.\n"
                       : "ID non trovato.\n");
                break;
            }

            case '5':
                lista_salva(&lista);
                printf("Attivita' salvate in %s. A presto!\n", FILE_DATI);
                in_esecuzione = 0;
                break;

            default:
                printf("Scelta non valida.\n");
                break;
        }
    }

    lista_libera(&lista);
    return 0;
}
