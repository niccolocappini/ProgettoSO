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

    printf("Connessione con Server andata a buon fine \n");

    // Fase in cui il client determina l'operazione da richiedere la server
    if (argc == 1)
    {
        generazioneErrore("Nessun codice passato al client \n");
    }
    int richiesta = atoi(argv[1]);
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

    case AGGIUGI_RECORD:
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
    printf("Stampa della Rubrica attuale: \n");
}

void ricercaRecordCognome(int clientSocket)
{
    char cognomeDaRicercare[MAX_LUNG_CAMPO];
    printf("Inserire un cognome per la ricerca: ");
    scanf("%s", cognomeDaRicercare);
    send(clientSocket, cognomeDaRicercare, MAX_LUNG_CAMPO, 0);
    printf("Cognome inviato al server \n");
}

void ricercaRecordNomeCognome(int clientSocket)
{
    char nome[MAX_LUNG_CAMPO];
    char cognome[MAX_LUNG_CAMPO];

    printf("Inserire Nome da Ricercare: \n");
    scanf("%s",nome);
    printf("Inserire Cognome da Ricercare: \n");
    scanf("%s",cognome);
    send(clientSocket, nome, MAX_LUNG_CAMPO, 0);
    send(clientSocket, cognome, MAX_LUNG_CAMPO, 0);

}

void aggiungiRecord(int clientSocket)
{
    recordRub record;
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