#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "funzioniClient.h"

int main(int argc, char *argv[])
{
    printf("Menù delle operazione che possono essere richieste dal client: \n"
         "1) Visualizzazzione tutti i record della rubrica \n"
         "2) Ricerca record tramite cognome \n"
         "3) Ricerca record tramite coppia nome-cognome \n"
         "4) Aggiunta Record \n"
         "5) Eliminazione Record \n"
         "6) Modifica Numero di Telefono \n"
         "7) Modifica Indirizzo \n\n");

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

    // Fase in cui il client determina l'operazione da richiedere la server
    int richiesta = 0;
    printf("Inserisci il codice dell'operazione da effettuare: \n");
    scanf("%d",&richiesta);
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

    // Fase in cui il client riceve l'eventuale richiesta di password
    if  (richiesta >= 4 && richiesta <= 7)
    
    {
        char richiestaStr[72];
        char password[100];
        recv(clientSocket, richiestaStr, sizeof(richiestaStr), 0);
        printf("%s",  richiestaStr);
        printf("Inserisci Password: \n");
        scanf("%s", password);
        printf("Password Inserita: %s \n", password);
        send(clientSocket, password, sizeof(password), 0);
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

    case MODIFICA_TELEFONO:
        modificaTelefono(clientSocket);

    default:
        generazioneErrore("Richiesta non valida\n");
        break;
    }

    // Fase di stampa dei Riusultati
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
    printf("Inserire un cognome per la ricerca: ");
    scanf("%s", cognomeDaRicercare);
    send(clientSocket, cognomeDaRicercare, MAX_LUNG_CAMPO, 0);
    printf("Cognome inviato al server \n");
    printf("\nStampa dei record in cui il cognome è %s: \n", cognomeDaRicercare);
}

void ricercaRecordNomeCognome(int clientSocket)
{
    char nome[MAX_LUNG_CAMPO];
    char cognome[MAX_LUNG_CAMPO];

    printf("Inserire Nome da Ricercare: ");
    scanf("%s",nome);
    printf("Inserire Cognome da Ricercare: ");
    scanf("%s",cognome);
    send(clientSocket, nome, MAX_LUNG_CAMPO, 0);
    send(clientSocket, cognome, MAX_LUNG_CAMPO, 0);
    printf("Nome e Cognome inviati al server \n");
    printf("\nStampa dei record in cui nome e cognome sono %s %s: \n", nome,cognome);
}

void aggiungiRecord(int clientSocket)
{
    recordRub record;
    
    printf("Inserire Nome da Inserire: ");
    scanf("%s",record.nome);
    send(clientSocket, record.nome, MAX_LUNG_CAMPO, 0);

    printf("Inserire Cognome da Inserire: ");
    scanf("%s",record.cognome);
    send(clientSocket, record.cognome, MAX_LUNG_CAMPO, 0);

    printf("Inserire Indirizzo da Inserire: ");
    scanf("%s",record.indirizzo);
    send(clientSocket, record.indirizzo, MAX_LUNG_CAMPO, 0);

    printf("Inserire Telefono da Inserire: ");
    scanf("%s",record.telefono);
    send(clientSocket, record.telefono, MAX_LUNG_CAMPO, 0);

    printf("Dati del record inviati al server per l'inserimento\n");
}

void rimuoviRecord(int clientSocket)
{
    recordRub record;
}

void modificaTelefono(int clientSocket)
{
    char *vecchioTelefono, nuovoTelefono;
}

void modificaIndirizzo(int clientSocket)
{
    char *vecchioIndirizzo, nuovoIndirizzo;
}