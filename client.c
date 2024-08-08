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
    if (clientSocket < 0){
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
    if(argc == 1){
        generazioneErrore("Nessun codice passato al client \n");
    }
    int richiesta = atoi(argv[1]);
    if(richiesta < 1 || richiesta >7){
        generazioneErrore("Codice richiesta non valido \n");
    }

    // Fase in cui il client inoltra, tramite socket, il codice della richiesta al server

    int scritto = send(clientSocket,&richiesta,sizeof(richiesta),0);
    if(scritto < 0){
        generazioneErrore("Scrittura su socket fallita \n");
    }

    // Fase in cui il client riceve l'eventuale richiesta di password
    if(richiesta >= 4 && richiesta <= 7){
        char richiesta[72];
        char password[100];
        recv(clientSocket, richiesta, sizeof(richiesta), 0);
        printf("%s",richiesta);
        printf("Inserisci Password: \n");
        scanf("%s",password);
        printf("Password Inserita: %s \n",password);
        send(clientSocket,password,sizeof(password),0);
    }

    // Fase Passaggio Dati per soddisfare la Richiesta

    switch (richiesta)
    {
        case VISUALIZZA_OGNI_RECORD:
            visualizzaRubrica();

            break;

        case RICERCA_RECORD_CON_COGNOME:
            ricercaRecordCognome();

            break;

        case RICERCA_RECORD_CON_NOME_COGNOME:
            ricercaRecordNomeCognome();

            break;

        case AGGIUGI_RECORD:
            aggiungiRecord();

            break;

        case RIMUOVI_RECORD:
            rimuoviRecord();
        
            break;

        case MODIFICA_TELEFONO:
            modificaTelefono();
        
            break;

        case MODIFICA_INDIRIZZO:
            modificaIndirizzo();
        
            break;

        default:
            generazioneErrore("Richiesta non valida\n");

    }

    // Fase attesa risultati da Server
    char * output;
    recv(clientSocket, output, sizeof(output), 0);

    // Fase di stampa dei risultati
    printf("%s",output);
    printf("\n");

    // Chiusura della connessione con il Server
    close(clientSocket);

    return 0;
}

/* I parametri li facciamo inserire dagli utenti tramite tastiera ??????? */
void visualizzaRubrica(){
    char * rubrica;
    printf("Stampa della Rubrica Attuale: \n");
}

void ricercaRecordCognome(){
    char * cognome;
}

void ricercaRecordNomeCognome(){
    char * nome, cognome;
}

void aggiungiRecord(){
    recordRub record;
}

void rimuoviRecord(){
    recordRub record;
}

void modificaTelefono(){
    char * vecchioTelefono, nuovoTelefono;
}

void modificaIndirizzo(){
    char * vecchioIndirizzo, nuovoIndirizzo;
}
