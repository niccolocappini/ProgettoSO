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

#define PASSWORD "ProgettoSO"

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
  rubrica = fopen("RubricaDB", "w+");
  if (rubrica == NULL)
    generazioneErrore("Rubrica non aperta correttamente \n");

  int serverSocket, clientSocket;
  struct sockaddr_in indirizzo;
  int lunghezzaIndirizzo = sizeof(indirizzo);

  // Creazione del socket
  serverSocket = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
  if (serverSocket < 0)
    generazioneErrore("Creazione socket fallita \n");

  // Configurazione di "indirizzo" per poter effettuare il binding correttamente
  indirizzo.sin_family = AF_INET;
  indirizzo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  indirizzo.sin_port = htons(PORTA);

  // Binding del socket alla porta specificata in "indirizzo"
  if (bind(serverSocket, (struct sockaddr *)&indirizzo, lunghezzaIndirizzo) < 0)
    generazioneErrore("Errore durante il binding \n");

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

    printf("Server in ascolto sulla porta %d \n", PORTA);

    // Accetta una connessione in entrata
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&indirizzo, (socklen_t *)&lunghezzaIndirizzo)) < 0)
      generazioneErrore("Errore durante l'accettazione di una richiesta \n");

    printf("Accettata connessione da un client \n");

    // Gestione richiesta tramite fork
    int pid = fork();

    if (pid == 0)
    {
      printf("Sono un figlio generato per gestire la richiesta del client \n");

      // Login utente
      int richiesta;
      if (recv(clientSocket, &richiesta, sizeof(richiesta), 0) < 1)
        generazioneErrore("Lettura della richiesta fallita \n");

      printf("Lettura effettuata: %d \n", richiesta);

      char *output;                                                                                                 // stringa che contterrà ciò che verrà inviato al client
      int dimensioneMassima = recordContenuti * 4 * MAX_LUNG_CAMPO + 4 * recordContenuti * 2 + recordContenuti * 2; // nel caso pessimo si devono stampare tutti i record
      int risultato;
      switch (richiesta)
      {

      case VISUALIZZA_OGNI_RECORD:
        printf("Gestione Richiesta 1: \n");
        output = (char *)malloc(dimensioneMassima);
        visualizzaRubrica(&output);

        break;

      case RICERCA_RECORD_CON_COGNOME:
        printf("Gestione Richiesta 2: \n");
        output = (char *)malloc(dimensioneMassima);
        ricercaRecordConCognome(clientSocket, &output);

        break;

      case RICERCA_RECORD_CON_NOME_COGNOME:
        printf("Gestione Richiesta 3: \n");
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
        risultato = rimuoviRecord(clientSocket, &output);
        controlloOutput(clientSocket, risultato, output);

        break;

      case MODIFICA_INDIRIZZO:
        printf("Gestione Richiesta 6: \n");
        richiestaPassword(clientSocket);
        risultato = modificaIndirizzo(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Modifica Indirizzo Fallita \n");

        break;

      case MODIFICA_TELEFONO:
        printf("Gestione Richiesta 7: \n");
        richiestaPassword(clientSocket);
        risultato = modificaTelefono(clientSocket, &output);
        controlloOutput(clientSocket, risultato, "Modifica Telefono Fallita \n");

        break;

      default:
        generazioneErrore("Richiesta non valida \n");
      }

      // Invio della risposta al client
      while (write(clientSocket, output, strlen(output) + 1) == -1)
      {
      }
      printf("Risposta inviata: \n%s \n", output); // da rimuovere (forse) prima della consegna
      printf("Richiesta eseguita: terminazione di questo figlio \n\n");
      close(clientSocket);

      exit(EXIT_SUCCESS);
    }
    else
    {
      printf("Sono il processo che continua ad accettare richieste \n");
      close(clientSocket);
    }

    printf("La connessione si è conclusa \n");
  }

  return 0;
}

void richiestaPassword(int clientSocket)
{
  char passwordRicevuta[MAX_LUNG_PASSWORD];
  if (recv(clientSocket, passwordRicevuta, sizeof(passwordRicevuta), 0) < 0)
    generazioneErrore("Lettura Password Fallita \n");
  printf("Password Ricevuta: %s \n", passwordRicevuta);

  char rispostaPassword[MAX_LUNG_MESSAGGIO];
  if (strcmp(passwordRicevuta, PASSWORD) != 0)
  {
    strcpy(rispostaPassword, "Password Errata \n");
    while (send(clientSocket, rispostaPassword, MAX_LUNG_MESSAGGIO, 0) == ESITO_NEGATIVO)
    {
    }
    generazioneErrore("Password errata: l'operazione non può essere eseguita \n");
  }
  else
  {
    strcpy(rispostaPassword, "Password Corretta \n");
    while (send(clientSocket, rispostaPassword, MAX_LUNG_MESSAGGIO, 0) == ESITO_NEGATIVO)
    {
    }
    printf("Password Accettata \n");
  }
}

void controlloOutput(int clientSocket, int risultato, char *messaggio)
{
  if (risultato == ESITO_NEGATIVO)
  {
    while (write(clientSocket, messaggio, strlen(messaggio) + 1) == ESITO_NEGATIVO)
    {
    }
    generazioneErrore(messaggio);
  }
}

void riceviCampoDaClient(int clientSocket, char *datoDaRicevere, int dimensioneDato, char *messaggioDiErrore)
{
  if (recv(clientSocket, datoDaRicevere, dimensioneDato, 0) < 1)
  {
    while (write(clientSocket, messaggioDiErrore, strlen(messaggioDiErrore) + 1) == ESITO_NEGATIVO)
    {
    }
    generazioneErrore(messaggioDiErrore);
  }
}

void riceviRecordDaClient(int clientSocket, recordRub *recordDaRicevere, int dimensioneRecord, char *messaggioDiErrore)
{
  if (recv(clientSocket, recordDaRicevere, dimensioneRecord, 0) < 1)
  {
    while (write(clientSocket, messaggioDiErrore, strlen(messaggioDiErrore) + 1) == ESITO_NEGATIVO)
    {
    }
    generazioneErrore(messaggioDiErrore);
  }
}

int controlloRubricaVuota(char **output) // restituisce 0 se la rubrica è vuota e in tal caso scrive già in output la stringa corretta
{
  fseek(rubrica, 0, SEEK_END);
  long int posizioneFinale = ftell(rubrica);
  if (posizioneFinale == 0)
  {
    strcat(*output, "La rubrica al momento è vuota \n");
    return ESITO_NEGATIVO;
  }
  return 0;
}

int normalizzaRecord(recordRub *recordDaAggiungere)
{ 
  char carattere;
  for (int i = 0; i < MAX_LUNG_CAMPO; i++)
  {
    carattere = (*recordDaAggiungere).nome[i];
    if(carattere == '\0')
    {
      break;
    }
    if(isalpha(carattere) == 0 && isspace(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }

    if(i == 0 || isspace((*recordDaAggiungere).nome[i-1]))
    {
      (*recordDaAggiungere).nome[i] = toupper(carattere);
    }
    else
    {
      (*recordDaAggiungere).nome[i] = tolower(carattere);
    }
  }

  for (int i = 0; i < MAX_LUNG_CAMPO; i++)
  {
    carattere = (*recordDaAggiungere).cognome[i];
    if(carattere == '\0')
    {
      break;
    }
    if(isalpha(carattere) == 0 && isspace(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }

    if(i == 0 || isspace((*recordDaAggiungere).cognome[i-1]))
    {
      (*recordDaAggiungere).cognome[i] = toupper(carattere);
    }
    else
    {
      (*recordDaAggiungere).cognome[i] = tolower(carattere);
    }
  }

  for (int i = 0; i < MAX_LUNG_CAMPO - 1; i++)
  {
    carattere = (*recordDaAggiungere).indirizzo[i];
    if(carattere == '\0')
    {
      break;
    }
    if(isalpha(carattere) == 0 && isspace(carattere) == 0 && isdigit(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }

    if(i == 0 || isspace((*recordDaAggiungere).indirizzo[i-1]))
    {
      (*recordDaAggiungere).indirizzo[i] = toupper(carattere);
    }
    else
    {
      (*recordDaAggiungere).indirizzo[i] = tolower(carattere);
    }
  }

  for (int i = 0; i < MAX_LUNG_CAMPO; i++)
  {
    carattere = (*recordDaAggiungere).telefono[i];
    if(carattere == '\0')
    {
      break;
    }
    if(isdigit(carattere) == 0)
    {
      return ESITO_NEGATIVO;
    }
  }

  return 0;
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
        posizioneRecord = ftell(rubrica);

      if (fread(campoLetto, MAX_LUNG_CAMPO, 1, rubrica) <= 0)
      {
        uscita = 1;
        break;
      }

      if (j == 0 && strcmp(campoLetto, recordDaRicercare->nome) == 0)
        recordTrovato++;

      if (j == 1 && strcmp(campoLetto, recordDaRicercare->cognome) == 0)
        recordTrovato++;

      if (j == 2 && strcmp(campoLetto, recordDaRicercare->indirizzo) == 0)
        recordTrovato++;

      if (j == 3 && strcmp(campoLetto, recordDaRicercare->telefono) == 0)
        recordTrovato++;
    }

    if (recordTrovato == 4)
    {
      printf("Il record trovato è il %ldesimo \n", posizioneRecord / (4 * MAX_LUNG_CAMPO) + 1);
      return posizioneRecord;
    }

    if (uscita == 1)
      break;
  }
  return ESITO_NEGATIVO; // si ritorna -1 se il record non è presente, altrimenti la posizione del record
}

void visualizzaRubrica(char **output)
{

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return;

  char supporto[MAX_LUNG_CAMPO];
  int i = 0;
  int contatore = 0;
  char fineStringa[MAX_LUNG_CAMPO];

  fseek(rubrica, 0, SEEK_SET); // il puntatore del file viene spostato all'inizio del file
  while (1)
  {
    i = fread(supporto, MAX_LUNG_CAMPO, 1, rubrica);
    if (i <= 0)
      break;

    if (strcmp(supporto, "\0") != 0)
    {
      strcat(*output, supporto);
      if (contatore % 4 != 3)
        strcat(*output, ", ");

      else
      {
        strcat(*output, "\n");
      }
    }
    contatore++;
  }
}

/* Casi di errore: Record non Trovato*/
void ricercaRecordConCognome(int clientSocket, char **output)
{

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return;

  char cognomeDaRicercare[MAX_LUNG_CAMPO];

  printf("In attesa del cognome da ricercare... \n");
  riceviCampoDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido \n");

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
        strcat(recordCorrente, ", ");

      if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
        recordTrovato = 0;
    }

    if (recordTrovato == 0)
    {
      strcat(*output, recordCorrente);
      strcat(*output, "\n");
    }

    if (uscita == 1)
      break;
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

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return;

  char nomeDaRicercare[MAX_LUNG_CAMPO];
  char cognomeDaRicercare[MAX_LUNG_CAMPO];

  printf("In attesa del nome da ricercare... \n");
  riceviCampoDaClient(clientSocket, nomeDaRicercare, sizeof(nomeDaRicercare), "Nome non ricevuto o non valido \n");

  printf("In attesa del cognome da ricercare... \n");
  riceviCampoDaClient(clientSocket, cognomeDaRicercare, sizeof(cognomeDaRicercare), "Cognome non ricevuto o non valido \n");

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
        strcat(recordCorrente, ", ");

      if (j == 0 && strcmp(campoLetto, nomeDaRicercare) == 0)
        recordTrovato++;

      if (j == 1 && strcmp(campoLetto, cognomeDaRicercare) == 0)
        recordTrovato++;
    }

    if (recordTrovato == 2)
    {
      strcat(*output, recordCorrente);
      strcat(*output, "\n");
    }

    if (uscita == 1)
      break;
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

  printf("In attesa del record da inserire... \n");
  riceviRecordDaClient(clientSocket, &recordDaAggiungere, sizeof(recordDaAggiungere), "Record non ricevuto o non valido \n");

  if (ricercaRecord(&recordDaAggiungere) != -1)
  {
    *output = "Record già presente in Rubrica\n";
    return ESITO_NEGATIVO;
  }

  if(normalizzaRecord(&recordDaAggiungere) == ESITO_NEGATIVO)
  {
    *output = "Record Formattato Scorrettamente\n";
    return ESITO_NEGATIVO;
  }

  fseek(rubrica, 0, SEEK_END);

  if (fwrite(&recordDaAggiungere, sizeof(recordDaAggiungere), 1, rubrica) < 1)
  {
    *output = "Aggiunta Record Fallita \n";
    return ESITO_NEGATIVO;
  }

  *output = "Aggiunta Record andata a buon fine \n";
  recordContenuti++;
  return 0;
}

/* Casi di errore: Eliminazione non riuscita
  Gestire il ricompattamento del file dopo l'eliminazione del record*/
int rimuoviRecord(int clientSocket, char **output)
{

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return ESITO_NEGATIVO;

  recordRub recordDaRimuovere;

  printf("In attesa del record da rimuovere... \n");
  riceviRecordDaClient(clientSocket, &recordDaRimuovere, sizeof(recordDaRimuovere), "Record non ricevuto o non valido \n");

  printf("Record da rimuovere: %s, %s, %s, %s \n", recordDaRimuovere.nome, recordDaRimuovere.cognome, recordDaRimuovere.indirizzo, recordDaRimuovere.telefono);

  long int posizioneRecordDaRimuovere = ricercaRecord(&recordDaRimuovere);
  if (posizioneRecordDaRimuovere < 0)
  {
    *output = "Record non trovato \n";
    return ESITO_NEGATIVO;
  }

  printf("Record trovato: inizio rimozione... \n");

  fseek(rubrica, posizioneRecordDaRimuovere, SEEK_SET);

  char fineStringa[4 * MAX_LUNG_CAMPO] = "\0";
  if (fwrite(fineStringa, 4 * MAX_LUNG_CAMPO, 1, rubrica) < 1)
  {
    *output = "Rimozione Record Fallita \n";
    return ESITO_NEGATIVO;
  }

  *output = "Rimozione Record andata a buon fine \n";
  recordContenuti--;
  return 0;
}

/* Casi di errore: vecchioIndirizzo non trovato, modifica non riuscita*/
int modificaIndirizzo(int clientSocket, char **output)
{

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return ESITO_NEGATIVO;

  recordRub recordDaModificare;

  printf("In attesa del record da modificare... \n");

  if (recv(clientSocket, &recordDaModificare, sizeof(recordDaModificare), 0) < 1)
    generazioneErrore("Record non ricevuto o non valido \n");

  printf("Record da modificare: %s, %s, %s, %s \n", recordDaModificare.nome, recordDaModificare.cognome, recordDaModificare.indirizzo, recordDaModificare.telefono);

  long int posizioneRecordDaModificare = ricercaRecord(&recordDaModificare);
  if (posizioneRecordDaModificare < 0)
  {
    *output = "Record non trovato \n";
    return ESITO_NEGATIVO;
  }

  printf("Il record da modificare è il %ldesimo \n", posizioneRecordDaModificare / (4 * MAX_LUNG_CAMPO) + 1);

  char indirizzoNuovo[MAX_LUNG_CAMPO];
  riceviCampoDaClient(clientSocket, indirizzoNuovo, sizeof(indirizzoNuovo), "Errore nella ricezione del nuovo indirizzo \n");

  printf("Nuovo Indirizzo: %s \n", indirizzoNuovo);

  if (modificaCampo(posizioneRecordDaModificare, 3, indirizzoNuovo) == 0)
  {
    *output = "Indirizzo non modificato \n";
    return ESITO_NEGATIVO;
  }

  *output = "Modifica Indirizzo andata a buon fine \n";
  return 0;
}

/* Casi di errore: vecchioTelefono non trovato, modifica non riuscita*/
int modificaTelefono(int clientSocket, char **output)
{

  if (controlloRubricaVuota(output) == ESITO_NEGATIVO)
    return ESITO_NEGATIVO;

  recordRub recordDaModificare;

  printf("In attesa del record da modificare... \n");

  if (recv(clientSocket, &recordDaModificare, sizeof(recordDaModificare), 0) < 1)
    generazioneErrore("Record non ricevuto o non valido \n");

  printf("Record da modificare: %s, %s, %s, %s \n", recordDaModificare.nome, recordDaModificare.cognome, recordDaModificare.indirizzo, recordDaModificare.telefono);

  long int posizioneRecordDaModificare = ricercaRecord(&recordDaModificare);
  if (posizioneRecordDaModificare < 0)
  {
    *output = "Record non trovato \n";
    return ESITO_NEGATIVO;
  }

  printf("Il record da modificare è il %ldesimo \n", posizioneRecordDaModificare / (4 * MAX_LUNG_CAMPO) + 1);

  char telefonoNuovo[MAX_LUNG_CAMPO];
  riceviCampoDaClient(clientSocket, telefonoNuovo, sizeof(telefonoNuovo), "Errore nella ricezione del nuovo numero di telefono \n");

  printf("Nuovo Telefono: %s \n", telefonoNuovo);

  if (modificaCampo(posizioneRecordDaModificare, 4, telefonoNuovo) < 1)
  {
    *output = "Telefono non modificato \n";
    return ESITO_NEGATIVO;
  }

  *output = "Modifica Telefono andata a buon fine \n";
  return 0;
}

int modificaCampo(int posizioneRecordDaModificare, int campoScelto, char *nuovoValore)
{
  fseek(rubrica, posizioneRecordDaModificare + (campoScelto - 1) * MAX_LUNG_CAMPO, SEEK_SET);
  return fwrite(nuovoValore, MAX_LUNG_CAMPO, 1, rubrica);
}
