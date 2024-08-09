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
    if (richiesta >= 4 && richiesta <= 7)
    {
        char richiesta[72];
        char password[100];
        recv(clientSocket, richiesta, sizeof(richiesta), 0);
        printf("%s", richiesta);
        printf("Inserisci Password: \n");
        scanf("%s", password);
        printf("Password Inserita: %s \n", password);
        send(clientSocket, password, sizeof(password), 0);
    }

    // Fase Passaggio Dati per soddisfare la Richiesta
    long dimensioneOutput = 0;
    switch (richiesta)
    {
        case VISUALIZZA_OGNI_RECORD:
            dimensioneOutput = visualizzaRubrica(clientSocket);

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

    // Fase attesa risultati da Server
    char output[dimensioneOutput];
    recv(clientSocket, output, dimensioneOutput, 0);

    // Fase di stampa dei risultati
    printf("%s", output);
    printf("\n");

    // Chiusura della connessione con il Server
    close(clientSocket);

    return 0;
}

/* I parametri li facciamo inserire dagli utenti tramite tastiera ??????? */
int visualizzaRubrica(int clientSocket){
    char dimStr[5];
    recv(clientSocket,dimStr,sizeof(dimStr),0);
    printf("%s\n",dimStr);
    printf("Stampa della Rubrica Attuale: \n");
    return atoi(dimStr);
}

void ricercaRecordCognome(int clientSocket)
{
    char *richiestaCognome;
    recv(clientSocket, richiestaCognome, sizeof(richiestaCognome), 0);
    printf("%s", richiestaCognome);
    char * cognome;
    scanf("Inserire un cognome per la ricerca: %s", cognome);
    send(clientSocket, cognome, sizeof(cognome), 0);
    char *risultato;
    recv(clientSocket, risultato, sizeof(risultato), 0);
    printf(risultato);
}

void ricercaRecordNomeCognome(int clientSocket)
{
    char * nome, cognome;
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