# Gestore Attività (CLI in C)

Un semplice gestore di attività (to-do list) che gira interamente da terminale,
scritto in **C** in un unico file sorgente.

Nato come progetto di portfolio per esercitare i concetti fondamentali del
linguaggio C: struct, array dinamici, puntatori, gestione della memoria e I/O su file.

## Caratteristiche

- Aggiunta, completamento e rimozione di attività
- Tre livelli di priorità (bassa, media, alta)
- I salvataggi persistono su file di testo (`attivita.txt`), leggibile a mano
- Array **dinamico** che raddoppia la capacità quando si riempie (`malloc` / `realloc` / `free`)
- Nessuna dipendenza esterna: serve solo un compilatore C

## Compilazione

```bash
gcc -Wall -Wextra -std=c11 -o todo todo.c
```

Compila senza alcun warning.

## Esecuzione

```bash
./todo
```

Apparirà un menu interattivo:

```
===== GESTORE ATTIVITA' =====
1) Mostra attivita'
2) Aggiungi attivita'
3) Segna come completata
4) Rimuovi attivita'
5) Salva ed esci
Scelta:
```

## Formato del file dati

Ogni riga di `attivita.txt` rappresenta un'attività:

```
id|completata|priorita|testo
1|1|3|Comprare il latte
```

dove `completata` vale `0`/`1` e `priorita` va da `1` (bassa) a `3` (alta).

## Concetti dimostrati

| Concetto                  | Dove nel codice                              |
|---------------------------|----------------------------------------------|
| `struct`                  | `Attivita`, `Lista`                          |
| Array dinamici            | `lista_aggiungi` (con `realloc`)             |
| Gestione della memoria    | `malloc` / `realloc` / `free`                |
| Puntatori                 | passaggio della lista per riferimento        |
| I/O su file               | `lista_salva` / `lista_carica`               |
| Input sicuro              | `fgets` + `strcspn` invece di `gets`         |

## Possibili sviluppi futuri

- Ordinamento delle attività per priorità
- Modifica del testo di un'attività esistente
- Interfaccia a riga di comando (`./todo aggiungi "..."`) usando `argc` / `argv`
- Date di scadenza

## Licenza

Distribuito con licenza MIT. Vedi il file `LICENSE`.
