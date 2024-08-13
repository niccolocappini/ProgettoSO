#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include "definizioniComuni.h"
#include "funzioniServer.h"

#define PASSWORD "PROGETTOSO"
#define ESITO_NEGATIVO -1

static FILE *rubrica = NULL;
static int recordContenuti = NUM_RECORD_RUBRICA;
/*

void handle_sigint(int sig);
void handle_death_signal_from_admin(int sig);
void handle_death_signal_from_user(int sig);

void handle_errno(int errorCode, char* errorMessage);
*/

int main()
{
  /* a+ -> file aperto in lettura/(scrittura in aggiunta) creandolo se necessario, o aggiungendovi a partire dalla fine
  e di conseguenza posizionandosi alla fine del file stesso */
  rubrica = fopen("RubricaDB", "w+");
  if (rubrica == NULL)
    generazioneErrore("Rubrica non aperta correttamente\n");

  int serverSocket, clientSocket;
  struct sockaddr_in indirizzo;
  int lunghezzaIndirizzo = sizeof(indirizzo);

  // Creazione del socket
  serverSocket = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  if (serverSocket < 0)
  {
    generazioneErrore("Creazione socket fallita \n");
  }

  // Configurazione di "indirizzo" per poter effettuare il binding correttamente
  indirizzo.sin_family = AF_INET;
  indirizzo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  indirizzo.sin_port = htons(PORTA);

  // Binding del socket alla porta specificata in "indirizzo"
  if (bind(serverSocket, (struct sockaddr *)&indirizzo, lunghezzaIndirizzo) < 0)
  {
    generazioneErrore("Errore durante il binding \n");
  }
  listen(serverSocket, 10);

  printf("Menù delle operazioni che possono essere richieste dal client: \n"
         "1) Visualizzazzione tutti i record della rubrica \n"
         "2) Ricerca record tramite cognome \n"
         "3) Ricerca record tramite coppia nome-cognome \n"
         "4) Aggiunta Record \n"
         "5) Eliminazione Record \n"
         "6) Modifica Indirizzo \n"
         "7) Modifica Numero di Telefono \n\n");

  while (1)
  {

    printf("Server in ascolto sulla porta %d\n", PORTA);

    // Accetta una connessione in entrata
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&indirizzo,
                               (socklen_t *)&lunghezzaIndirizzo)) < 0)
    {
      generazioneErrore("Errore durante l'accettazione di una richiesta \n");
    }

    printf("Accettata connessione da un client\n");

    // Gestione richiesta tramite fork
    int pid = fork();

    if (pid == 0)
    {
      printf("Sono un figlio generato per gestire la richiesta del client\n");

      // Login utente
      int richiesta;
      int risultato = recv(clientSocket, &richiesta, sizeof(richiesta), 0);
      if (risultato < 1)
      {
        generazioneErrore("Lettura fallita\n");
      }

      printf("Lettura effettuata: %d\n", richiesta);

      char *output;                                                                                                 // stringa che contterrà ciò che verrà inviato al client
      int dimensioneMassima = recordContenuti * 4 * MAX_LUNG_CAMPO + 4 * recordContenuti * 2 + recordContenuti * 2; // nel caso pessimo si devono stampare tutti i record
      switch (richiesta)
      {

      case VISUALIZZA_OGNI_RECORD:
        printf("Gestione Richiesta 1\n");
        output = (char *)malloc(dimensioneMassima);
        visualizzaRubrica(&output);

        break;

      case RICERCA_RECORD_CON_COGNOME:
        printf("Gestione Richiesta 2\n");
        output = (char *)malloc(dimensioneMassima);
        ricercaRecordConCognome(clientSocket, &output);

        break;

      case RICERCA_RECORD_CON_NOME_COGNOME:
        printf("Gestione Richiesta 3\n");
        output = (char *)malloc(dimensioneMassima);
        ricercaRecordConNomeCognome(clientSocket, &output);

        break;

      case AGGIUNGI_RECORD:
        printf("Gestione Richiesta 4: \n");
        richiestaPassword(clientSocket);
        risultato = aggiungiRecord(clientSocket, &output);
        controlloOutput(clientSocket, risultato, output);

        break;

      case RIMUOVI_RECORD:
        printf("Gestione Richiesta 5: \n");
        richiestaPassword(clientSocket);
        output = (char *)malloc(MAX_LUNG_MESSAGGIO);
        risultato = rimuoviRecord(clientSocket, &output);
        controlloOutput(clientSocket, risultato, output);

        break;

      case MODIFICA_INDIRIZZO:
        printf("Gestione Richiesta 7: \n");
        richiestaPassword(clientSocket);
        risultato = modificaIndirizzo(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Modifica Indirizzo Fallita \n");

        break;

      case MODIFICA_TELEFONO:
        printf("Gestione Richiesta 6: \n");
        richiestaPassword(clientSocket);
        risultato = modificaTelefono(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Modifica Telefono Fallita \n");

        break;

      default:
        generazioneErrore("Richiesta non valida\n");
      }

      // Invio della risposta al client
      write(clientSocket, output, strlen(output) + 1);
      printf("Risposta inviata: \n%s \n", output); // da togliere prima della consegna
      printf("Richiesta eseguita: terminazione di questo figlio\n\n");
      close(clientSocket);
      // free(output);
      exit(EXIT_SUCCESS);
    }
    else
    {
      printf("Sono il processo che continua ad accettare richieste\n");
      close(clientSocket);
    }

    // Chiudi la connessione con il client
    // logoutUtente(clientSocket);

    printf("La connessione si è conclusa\n");
  }

  return 0;
}

int loginUtente(int clientSocket) { return 0; }

void logoutUtente(int clientSocket) {}

void richiestaPassword(int clientSocket)
{
  char passwordRicevuta[MAX_LUNG_PASSWORD];
  recv(clientSocket, passwordRicevuta, sizeof(passwordRicevuta), 0);
  printf("Password Ricevuta: %s \n", passwordRicevuta);

  char rispostaPassword[MAX_LUNG_MESSAGGIO];
  if (strcmp(passwordRicevuta, PASSWORD) != 0)
  {
    strcpy(rispostaPassword, "Password Errata");
    send(clientSocket, rispostaPassword, MAX_LUNG_MESSAGGIO, 0);
    generazioneErrore("Password errata: l'operazione non può essere eseguita \n");
  }
  else
  {
    strcpy(rispostaPassword, "Password Corretta");
    send(clientSocket, rispostaPassword, MAX_LUNG_MESSAGGIO, 0);
    printf("Password Accettata \n");
  }
}

void controlloOutput(int clientSocket, int risultato, char *messaggio)
{
  if (risultato == ESITO_NEGATIVO)
  {
    write(clientSocket, messaggio, strlen(messaggio) + 1);
    generazioneErrore(messaggio);
  }
}

void riceviCampoDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggioDiErrore)
{
  int byteLetti;

  byteLetti = recv(clientSocket, datoDaRicevere, dimensioneDato, 0);
  if (byteLetti < 1)
  {
    write(clientSocket, messaggioDiErrore, strlen(messaggioDiErrore) + 1);
    generazioneErrore(messaggioDiErrore);
  }
}

void riceviRecordDaClient(int clientSocket, recordRub *recordDaRicevere, int dimensioneRecord, char *messaggioDiErrore)
{
  int byteLetti;

  byteLetti = recv(clientSocket, recordDaRicevere, dimensioneRecord, 0);
  if (byteLetti < 1)
  {
    write(clientSocket, messaggioDiErrore, strlen(messaggioDiErrore) + 1);
    generazioneErrore(messaggioDiErrore);
  }
}

int controlloRubricaVuota(char **output) // restituisce 0 se la rubrica è vuota e in tal caso scrive già in output la stringa corretta
{
  fseek(rubrica, 0, SEEK_END);
  long int posizioneFinale = ftell(rubrica);
  if (posizioneFinale == 0)
  {
    strcat(*output, "La rubrica al momento è vuota\n");
    return 0;
  }
  return 1;
}

void normalizzaRecord(recordRub *recordDaAggiungere)
{
  /*int c = recordDaAggiungere->nome[0];
  toupper(c);
  printf("%c\n",c);*/
}

long int ricercaRecord(recordRub *recordDaRicercare) // metodo generale di ricerca record da usare nelle ricerche settoriali
{
  long int posizioneRecord;

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 0;
  char campoLetto[MAX_LUNG_CAMPO];
  int uscita = 0;
  while (1)
  {
    recordTrovato = 0;
    for (int j = 0; j < 4; j++)
    {
      if (j == 0)
      {
        posizioneRecord = ftell(rubrica);
      }

      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) <= 0)
      {
        uscita = 1;
        break;
      }

      if (j == 0 && strcmp(campoLetto, recordDaRicercare->nome) == 0)
      {
        recordTrovato++;
      }

      if (j == 1 && strcmp(campoLetto, recordDaRicercare->cognome) == 0)
      {
        recordTrovato++;
      }

      if (j == 2 && strcmp(campoLetto, recordDaRicercare->indirizzo) == 0)
      {
        recordTrovato++;
      }

      if (j == 3 && strcmp(campoLetto, recordDaRicercare->telefono) == 0)
      {
        recordTrovato++;
      }
    }

    if (recordTrovato == 4)
    {
      printf("Tutti i campi sono uguali\n");
      printf("Il record trovato è il %ldesimo\n", posizioneRecord / (4 * MAX_LUNG_CAMPO) + 1);
      printf("Posizione ritornata: %ld\n", posizioneRecord); // debug
      return posizioneRecord;
    }

    if (uscita == 1)
    {
      break;
    }
  }
  return ESITO_NEGATIVO; // si ritorna -1 se il record non è presente, altrimenti la posizione del record
}

void visualizzaRubrica(char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    char supporto[MAX_LUNG_CAMPO];
    int i = 0;
    int contatore = 0;
    char fineStringa[MAX_LUNG_CAMPO];

    fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio del file
    while (1)
    {
      i = fread(supporto, MAX_LUNG_CAMPO, 1, rubrica);
      if (i <= 0)
      {
        break;
      }

      if(strcmp(supporto,"\0") != 0)
      {
        strcat(*output, supporto);
        if (contatore % 4 != 3)
        {
          strcat(*output, ", ");
        }
        else
        {
          strcat(*output, "\n");
        }
      }
      contatore++;
    }
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConCognome(int clientSocket, char **output)
{
  char cognomeDaRicercare[MAX_LUNG_CAMPO];

  printf("In attesa del cognome da ricercare... \n");
  riceviCampoDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido\n");

  if (controlloRubricaVuota(output) == 0)
  {
    return;
  }

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato;
  char recordCorrente[4 * MAX_LUNG_CAMPO + 100];
  char campoLetto[MAX_LUNG_CAMPO];
  int uscita = 0;
  while (1)
  {
    recordTrovato = 1;
    strcpy(recordCorrente, "");
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) <= 0)
      {
        uscita = 1;
        break;
      }

      strcat(recordCorrente, campoLetto);
      if (j != 3)
      {
        strcat(recordCorrente, ", ");
      }

      if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
      {
        recordTrovato = 0;
      }
    }

    if (recordTrovato == 0)
    {
      strcat(*output, recordCorrente);
      strcat(*output, "\n");
    }

    if (uscita == 1)
    {
      break;
    }
  }

  if (strlen(*output) == 0)
  {
    strcat(*output, "Nella rubrica non è presente nessun record con il cognome ");
    strcat(*output, cognomeDaRicercare);
    strcat(*output, "\n");
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConNomeCognome(int clientSocket, char **output)
{

  char nomeDaRicercare[MAX_LUNG_CAMPO];
  char cognomeDaRicercare[MAX_LUNG_CAMPO];

  printf("In attesa del nome da ricercare... \n");
  riceviCampoDaClient(clientSocket, nomeDaRicercare, sizeof(nomeDaRicercare), "Nome non ricevuto o non valido\n");

  printf("In attesa del cognome da ricercare... \n");
  riceviCampoDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido\n");

  if (controlloRubricaVuota(output) == 0)
  {
    return;
  }

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 0;
  char recordCorrente[4 * MAX_LUNG_CAMPO + 100];
  char campoLetto[MAX_LUNG_CAMPO];
  int uscita = 0;
  while (1)
  {
    recordTrovato = 0;
    strcpy(recordCorrente, "");
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) <= 0)
      {
        uscita = 1;
        break;
      }

      strcat(recordCorrente, campoLetto);
      if (j != 3)
      {
        strcat(recordCorrente, ", ");
      }

      if (j == 0 && strcmp(campoLetto, nomeDaRicercare) == 0)
      {
        recordTrovato++;
      }

      if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
      {
        recordTrovato++;
      }
    }

    if (recordTrovato == 2)
    {
      strcat(*output, recordCorrente);
      strcat(*output, "\n");
    }

    if (uscita == 1)
    {
      break;
    }
  }

  if (strlen(*output) == 0)
  {
    strcat(*output, "Nella rubrica non è presente nessun record con nome-cognome ");
    strcat(*output, nomeDaRicercare);
    strcat(*output, " ");
    strcat(*output, cognomeDaRicercare);
    strcat(*output, "\n");
  }
}

/* Casi di errore: Aggiunta non riuscita*/
int aggiungiRecord(int clientSocket, char **output)
{
  recordRub recordDaAggiungere;
  int byteScritti;

  printf("In attesa del record da inserire... \n");
  riceviRecordDaClient(clientSocket, &recordDaAggiungere, sizeof(recordDaAggiungere), "Record non ricevuto o non valido\n");

  if (ricercaRecord(&recordDaAggiungere) != -1)
  {
    *output = "Record già presente in Rubrica\n";
    return ESITO_NEGATIVO;
  }

  normalizzaRecord(&recordDaAggiungere);

  fseek(rubrica, 0, SEEK_END);
  byteScritti = fwrite(&recordDaAggiungere, sizeof(recordDaAggiungere), 1, rubrica);
  if (byteScritti <= 0) 
  {
    *output = "Aggiunta Record Fallita\n";
    return ESITO_NEGATIVO;
  }
  recordContenuti++;
  *output = "Aggiunta Record andata a buon fine\n";
  return 1;
}

/* Casi di errore: Eliminazione non riuscita
  Gestire il ricompattamento del file dopo l'eliminazione del record*/
int rimuoviRecord(int clientSocket, char **output)
{
  recordRub recordDaRimuovere;

  printf("In attesa del record da rimuovere... \n");
  riceviRecordDaClient(clientSocket, &recordDaRimuovere, sizeof(recordDaRimuovere),"Record non ricevuto o non valido\n");
  
  printf("Record da rimuovere: %s, %s, %s, %s \n", recordDaRimuovere.nome, recordDaRimuovere.cognome, recordDaRimuovere.indirizzo, recordDaRimuovere.telefono);

  if (controlloRubricaVuota(output) == 0)
  {
    return ESITO_NEGATIVO;
  }

  long int posizioneRecordDaRimuovere = ricercaRecord(&recordDaRimuovere);
  if (posizioneRecordDaRimuovere < 0)
  {
    *output = "Rimozione Record Fallita";
    return ESITO_NEGATIVO;
  }

  printf("Record trovato: inizio rimozione...\n");

  fseek(rubrica, posizioneRecordDaRimuovere, SEEK_SET);
  
  char fineStringa[4 * MAX_LUNG_CAMPO] = "\0";
  int byteScritti = fwrite(fineStringa, 4 * MAX_LUNG_CAMPO, 1, rubrica);

  *output = "Rimozione Record Compiuta\n";
  recordContenuti--;
  return 0;
}

/* Casi di errore: vecchioIndirizzo non trovato, modifica non riuscita*/
int modificaIndirizzo(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    recordRub recordDaModificare;

    printf("In attesa del record da modificare... \n");
    int byteLetti = recv(clientSocket, &recordDaModificare, sizeof(recordDaModificare), 0);
    if (byteLetti < 1)
    {
      generazioneErrore("Record non ricevuto o non valido\n");
    }
    printf("Record da modificare: %s, %s, %s, %s \n", recordDaModificare.nome, recordDaModificare.cognome, recordDaModificare.indirizzo, recordDaModificare.telefono);

    long int posizioneRecordDaModificare = ricercaRecord(&recordDaModificare);
    if (posizioneRecordDaModificare < 0)
    {
      return ESITO_NEGATIVO;
    }

    printf("Il record da modificare è il %ldesimo \n", posizioneRecordDaModificare / (4 * MAX_LUNG_CAMPO) + 1);

    char indirizzoNuovo[MAX_LUNG_CAMPO];
    riceviCampoDaClient(clientSocket, indirizzoNuovo, sizeof(indirizzoNuovo), "Errore nella ricezione del nuovo indirizzo\n");

    printf("Nuovo Indirizzo: %s \n", indirizzoNuovo);

    if (modificaCampo(posizioneRecordDaModificare, 3, indirizzoNuovo) == 0)
      generazioneErrore("Indirizzo non modificato");

    *output = "Modifica Indirizzo andata a buon fine\n";
    return 1;
  }
  return ESITO_NEGATIVO;
}

/* Casi di errore: vecchioTelefono non trovato, modifica non riuscita*/
int modificaTelefono(int clientSocket, char **output)
{
  if (controlloRubricaVuota(output) != 0)
  {
    recordRub recordDaModificare;

    printf("In attesa del record da modificare... \n");
    int byteLetti = recv(clientSocket, &recordDaModificare, sizeof(recordDaModificare), 0);
    if (byteLetti < 1)
    {
      generazioneErrore("Record non ricevuto o non valido\n");
    }
    printf("Record da modificare: %s, %s, %s, %s \n", recordDaModificare.nome, recordDaModificare.cognome, recordDaModificare.indirizzo, recordDaModificare.telefono);

    long int posizioneRecordDaModificare = ricercaRecord(&recordDaModificare);
    if (posizioneRecordDaModificare < 0)
    {
      return ESITO_NEGATIVO;
    }

    printf("Il record da modificare è il %ldesimo \n", posizioneRecordDaModificare / (4 * MAX_LUNG_CAMPO) + 1);

    char telefonoNuovo[MAX_LUNG_CAMPO];
    riceviCampoDaClient(clientSocket, telefonoNuovo, sizeof(telefonoNuovo), "Errore nella ricezione del nuovo numero di telefono\n");

    printf("Nuovo Telefono: %s \n", telefonoNuovo);

    if (modificaCampo(posizioneRecordDaModificare, 4, telefonoNuovo) == 0)
      generazioneErrore("Telefono non modificato");

    *output = "Modifica Telefono andata a buon fine\n";
    return 1;
  }
  return ESITO_NEGATIVO;
}

int modificaCampo(int posizioneRecordDaModificare, int campoScelto, char *nuovoValore)
{
  fseek(rubrica, posizioneRecordDaModificare + (campoScelto - 1) * MAX_LUNG_CAMPO, SEEK_SET);
  return fwrite(nuovoValore, MAX_LUNG_CAMPO, 1, rubrica);
}

/*
long int ricercaPosizioneRecordConSingoloCampo(char *valoreDaRicercare, int campoScelto)
{

  if (campoScelto < 1 || campoScelto > 4)
    generazioneErrore("Utilizzo errato della ricerca dei record con un campo");

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio
  int recordTrovato = 1;
  char campoLetto[MAX_LUNG_CAMPO];

  for (int i = 0; i < recordContenuti; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) == 0)
      {
        generazioneErrore("Errore nella lettura\n");
      }

      if (j == campoScelto && strcmp(campoLetto, valoreDaRicercare) == 0)
      {
        recordTrovato = 0;
      }
    }

    if (recordTrovato == 0)
    {
      return ftell(rubrica) - campoScelto * MAX_LUNG_CAMPO; // la sottrazione è perché il record è stato letto, quindi il puntatore è stato spostato alla sua fine
    }
  }
  return -1;
}
*/