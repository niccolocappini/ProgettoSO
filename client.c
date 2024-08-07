#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
        printf(richiesta);
        printf("Inserisci Password: \n");
        scanf("%s",password);
        printf("Password Inserita: %s \n",password);
        send(clientSocket,password,sizeof(password),0);
    }

    // Fase Passaggio Dati per soddisfare la Richiesta

    switch (richiesta)
    {

        case VISUALIZZA_OGNI_RECORD:
            printf("Richiesta 1\n");

            break;

        case RICERCA_RECORD_CON_COGNOME:
            printf("Richiesta 2\n");

            break;

        case RICERCA_RECORD_CON_NOME_COGNOME:
            printf("Richiesta 3\n");

            break;

        case AGGIUGI_RECORD:
            printf("Richiesta 4\n");

        break;

        case RIMUOVI_RECORD:
            printf("Richiesta 5\n");
        
        break;

        case MODIFICA_TELEFONO:
            printf("Richiesta 6\n");
        
        break;

        case MODIFICA_INDIRIZZO:
            printf("Richiesta 7\n");
        
        break;

        default:
            generazioneErrore("Richiesta non valida\n");
        break;
    }

    // Fase attesa risultati da Server

    // Chiusura della connessione con il Server
    close(clientSocket);

    return 0;
}
