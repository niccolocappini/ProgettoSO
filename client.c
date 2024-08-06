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

    int clientFd, dimServer, risultato;

    struct sockaddr_in indirizzoINETServer;
    struct sockaddr *indirizzoINETServerPtr;

    // Creazione Socket del Client
    clientFd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (clientFd < 0)
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
        risultato = connect(clientFd, indirizzoINETServerPtr, dimServer);
        if (risultato == -1)
        {
            printf("Connessione con Server Fallita \n");
            sleep(1);
        }
    } while (risultato == -1);

    printf("Connessione con Server andata a buon fine \n");

    // Fase in cui il client determina l'operazione da richiedere la server

    int richiesta = 0;
    printf("Menù delle operazione che possono essere richieste dal client: \n");
    printf("1) Visualizzazzione tutti i record della rubrica \n");
    printf("2) Ricerca record tramite cognome \n");
    printf("3) Ricerac record tramite coppia nome-cognome \n");
    printf("4) Aggiunta Record \n");
    printf("5) Eliminazione Record \n");
    printf("6) Modifica Numero di Telefono \n");
    printf("7) Modifica Indirizzo \n");

    scanf("Inserire numero operazione richiesta: %d", &richiesta);
    if (richiesta < 1 || richiesta > 7)
    {
        generazioneErrore("Codice richiesta non valido \n");
    }

    // Fase in cui il client inoltra, tramite socket, il codice della richiesta al server

    int scritto = send(clientFd, &richiesta, sizeof(richiesta), 0);
    if (scritto < 0)
    {
        generazioneErrore("Scrittura su socket fallita \n");
    }

    // Chiusura della connessione con il Server
    close(clientFd);

    return 0;
}
