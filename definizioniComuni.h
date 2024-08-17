#define NUM_RECORD_RUBRICA 50
#define MAX_LUNG_CAMPO 64

#define DEFAULT_PROTOCOL 0 // per creazione socket
#define PORTA 8400         // scelta tra quelle disponibili per la connessione client-server

#define VISUALIZZA_OGNI_RECORD 1
#define RICERCA_RECORD_CON_COGNOME 2
#define RICERCA_RECORD_CON_NOME_COGNOME 3
#define AGGIUNGI_RECORD 4
#define RIMUOVI_RECORD 5
#define MODIFICA_INDIRIZZO 6
#define MODIFICA_TELEFONO 7

#define MAX_LUNG_PASSWORD 100
#define MAX_LUNG_MESSAGGIO 100

#define ESITO_NEGATIVO -1

typedef struct
{
    char nome[MAX_LUNG_CAMPO];
    char cognome[MAX_LUNG_CAMPO];
    char indirizzo[MAX_LUNG_CAMPO];
    char telefono[MAX_LUNG_CAMPO];
} recordRub;

void generazioneErrore(char messaggio[])
{
    perror(messaggio);
    exit(EXIT_FAILURE);
}
