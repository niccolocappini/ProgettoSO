#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#include "funzioniClient.h"

int main(int argc, char *argv[])
{   
    signal(SIGINT, handle_sigint);

    printf("Menù delle operazione che possono essere richieste dal client: \n"
           "1) Visualizzazione tutti i record della rubrica \n"
           "2) Ricerca record tramite cognome \n"
           "3) Ricerca record tramite coppia nome-cognome \n"
           "4) Aggiunta Record \n"
           "5) Eliminazione Record \n"
           "6) Modifica Indirizzo \n"
           "7) Modifica Numero di Telefono \n\n");

    int clientSocket, dimServer, risultato;

    struct sockaddr_in indirizzoINETServer;
    struct sockaddr *indirizzoINETServerPtr;

    // Creazione Socket del Client
    clientSocket = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (clientSocket < 0)
    {
        generazioneErrore("Creazione Socket del Client Fallita \n");
    }

    indirizzoINETServerPtr = (struct sockaddr *)&indirizzoINETServer;
    dimServer = sizeof(indirizzoINETServer);

    indirizzoINETServer.sin_family = AF_INET;
    indirizzoINETServer.sin_port = htons(PORTA);
    indirizzoINETServer.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Connessione con il Server
    printf("Tentata Connesione con Server \n");

    do
    {
        risultato = connect(clientSocket, indirizzoINETServerPtr, dimServer);
        if (risultato == -1)
        {
            printf("Connessione con Server Fallita \n");
            sleep(1);
        }
    } while (risultato == -1);

    printf("Connessione con Server andata a buon fine \n\n");

    // Fase in cui il client determina l'operazione da richiedere al server
    int richiesta = 0;
    printf("Inserisci il codice dell'operazione da effettuare: \n");
    scanf("%d", &richiesta);
    if (richiesta < 1 || richiesta > 7)
    {
        generazioneErrore("Codice richiesta non valido \n");
    }

    // Fase in cui il client inoltra, tramite socket, il codice della richiesta al server

    int scritto = send(clientSocket, &richiesta, sizeof(richiesta), 0);
    if (scritto < 0)
    {
        generazioneErrore("Scrittura su socket fallita \n");
    }

    // Fase in cui il client eventualmente inoltra la password
    if (richiesta >= 4 && richiesta <= 7)
    {
        char password[MAX_LUNG_PASSWORD];
        printf("Inserisci Password: \n");
        scanf("%s", password);
        send(clientSocket, password, sizeof(password), 0);
        char rispostaPassword[MAX_LUNG_MESSAGGIO];
        recv(clientSocket, rispostaPassword, sizeof(rispostaPassword), 0);
        if (strcmp(rispostaPassword, "Password Errata \n") == 0)
        {
            generazioneErrore(rispostaPassword);
        }
        printf("%s", rispostaPassword);
    }

    // Fase Passaggio Dati per soddisfare la Richiesta
    switch (richiesta)
    {
    case VISUALIZZA_OGNI_RECORD:
        visualizzaRubrica();

        break;

    case RICERCA_RECORD_CON_COGNOME:
        ricercaRecordCognome(clientSocket);

        break;

    case RICERCA_RECORD_CON_NOME_COGNOME:
        ricercaRecordNomeCognome(clientSocket);

        break;

    case AGGIUNGI_RECORD:
        aggiungiRecord(clientSocket);

        break;

    case RIMUOVI_RECORD:
        rimuoviRecord(clientSocket);

        break;

    case MODIFICA_INDIRIZZO:
        modificaIndirizzo(clientSocket);

        break;

    case MODIFICA_TELEFONO:
        modificaTelefono(clientSocket);

        break;

    default:
        generazioneErrore("Richiesta non valida \n");
        break;
    }

    // Fase di stampa dei Risultati
    stampaOutputDalServer(clientSocket);

    // Fase di stampa dello stato del server
    stampaOutputDalServer(clientSocket);

    // Chiusura della connessione con il Server
    close(clientSocket);

    return 0;
}

void stampaOutputDalServer(int clientSocket)
{
    char str[1];
    int n = 0;

    do
    {
        n = read(clientSocket, str, 1);
        printf("%s", str);
    } while (n > 0 && strcmp(str, "\0") != 0);
    printf("\n");
}

void visualizzaRubrica()
{
    printf("\nStampa della Rubrica attuale: \n");
}

void ricercaRecordCognome(int clientSocket)
{
    char cognomeDaRicercare[MAX_LUNG_CAMPO];
    char supporto;
    printf("Inserire un cognome per la ricerca: ");
    fflush(stdin);
    scanf("%c", &supporto);
    scanf("%[^'\n']s", cognomeDaRicercare); // in questo modo si prendono da input le stringhe con gli spazi e senza l'accapo
    send(clientSocket, cognomeDaRicercare, MAX_LUNG_CAMPO, 0);
    printf("Cognome inviato al server \n");
    printf("\nStampa dei record in cui il cognome è %s: \n", cognomeDaRicercare);
    fflush(stdin);
}

void ricercaRecordNomeCognome(int clientSocket)
{
    char nome[MAX_LUNG_CAMPO];
    char cognome[MAX_LUNG_CAMPO];
    char supporto;

    fflush(stdin);
    printf("Inserire Nome da Ricercare: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", nome);
    printf("Inserire Cognome da Ricercare: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", cognome);
    send(clientSocket, nome, MAX_LUNG_CAMPO, 0);
    send(clientSocket, cognome, MAX_LUNG_CAMPO, 0);
    printf("Nome e Cognome inviati al server \n");
    printf("\nStampa dei record in cui nome e cognome sono %s e %s: \n", nome, cognome);
    fflush(stdin);
}

void inserimentoDatiRecord(recordRub *record)
{
    char recordStr[4 * MAX_LUNG_CAMPO];
    char supporto;

    fflush(stdin);
    printf("Inserire Nome: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", (char *)record->nome);
    strcpy(recordStr, (char *)record->nome);
    strcat(recordStr, " ");

    printf("Inserire Cognome: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", (char *)record->cognome);
    strcat(recordStr, (char *)record->cognome);
    strcat(recordStr, " ");

    printf("Inserire Indirizzo: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", (char *)record->indirizzo);
    strcat(recordStr, (char *)record->indirizzo);
    strcat(recordStr, " ");

    printf("Inserire Telefono: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", (char *)record->telefono);
    strcat(recordStr, (char *)record->telefono);

    printf("Record inviato: %s\n", recordStr);
    fflush(stdin);
}

void invioRecord(int clientSocket)
{
    recordRub recordDaInviare;
    inserimentoDatiRecord(&recordDaInviare);
    if (send(clientSocket, &recordDaInviare, sizeof(recordDaInviare), 0) < 0)
        generazioneErrore("Invio del record fallito \n");
}

void aggiungiRecord(int clientSocket)
{
    invioRecord(clientSocket);
    printf("Dati del record inviati al server per l'inserimento \n");
}

void rimuoviRecord(int clientSocket)
{
    invioRecord(clientSocket);
    printf("Dati del record inviati al server per la rimozione \n");
}

void modificaIndirizzo(int clientSocket)
{
    invioRecord(clientSocket);
    printf("Dati del record inviati al server per la modifica \n");

    char nuovoIndirizzo[MAX_LUNG_CAMPO];
    char supporto;

    fflush(stdin);
    printf("Inserire Nuovo Indirizzo: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", nuovoIndirizzo);
    send(clientSocket, nuovoIndirizzo, sizeof(nuovoIndirizzo), 0);
    fflush(stdin);
}

void modificaTelefono(int clientSocket)
{
    invioRecord(clientSocket);
    printf("Dati del record inviati al server per la modifica \n");

    char nuovoTelefono[MAX_LUNG_CAMPO];
    char supporto;

    fflush(stdin);
    printf("Inserire Nuovo Telefono: ");
    scanf("%c", &supporto);
    scanf("%[^'\n']s", nuovoTelefono);
    send(clientSocket, nuovoTelefono, sizeof(nuovoTelefono), 0);
    fflush(stdin);
}

void handle_sigint(int sig){
    printf("\nSegnale di interruzione rilevato: arresto dell'esecuzione \n");
    exit(EXIT_FAILURE);
}