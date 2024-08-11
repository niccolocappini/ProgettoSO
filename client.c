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
        /*char passwordLungStr[3];
        int passwordLung = 0;
        char password[11];
        recv(clientSocket, passwordLungStr, sizeof(passwordLungStr), 0);
        passwordLung = atoi(passwordLungStr);
        printf("Inserisci Password: \n");
        scanf("%s", password);
        printf("Password Inserita: %s \n", password);
        send(clientSocket, password, sizeof(password), 0);*/

        char password[MAX_LUNG_PASSWORD];
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
    char supporto;
    printf("Inserire un cognome per la ricerca: ");
    fflush(stdin);
    scanf("%c",&supporto);
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
    scanf("%c",&supporto);
    scanf("%[^'\n']s", nome);
    printf("Inserire Cognome da Ricercare: ");
    scanf("%c",&supporto);
    scanf("%[^'\n']s", cognome);
    send(clientSocket, nome, MAX_LUNG_CAMPO, 0);
    send(clientSocket, cognome, MAX_LUNG_CAMPO, 0);
    printf("Nome e Cognome inviati al server \n");
    printf("\nStampa dei record in cui nome e cognome sono %s e %s: \n", nome,cognome);
    fflush(stdin);
}

void aggiungiRecord(int clientSocket)
{
    recordRub record;
    char recordStr[4*MAX_LUNG_CAMPO ];
    char supporto;

    fflush(stdin);
    printf("Inserire Nome da Inserire: ");
    scanf("%c",&supporto);
    scanf("%[^'\n']s", record.nome);
    strcpy(recordStr,record.nome);

    printf("Inserire Cognome da Inserire: ");
    scanf("%c",&supporto);
    scanf("%[^'\n']s", record.cognome);
    strcat(recordStr,record.cognome);

    printf("Inserire Indirizzo da Inserire: ");
    scanf("%c",&supporto);
    scanf("%[^'\n']s", record.indirizzo);
    strcat(recordStr,record.indirizzo);

    printf("Inserire Telefono da Inserire: ");
    scanf("%c",&supporto);
    scanf("%[^'\n']s", record.telefono);
    strcat(recordStr,record.telefono);

    printf("%s\n",recordStr);
    write(clientSocket,recordStr,strlen(recordStr)+1);
    

    printf("Dati del record inviati al server per l'inserimento\n");
    fflush(stdin);
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