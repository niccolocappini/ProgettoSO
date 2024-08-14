#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "definizioniComuni.h"

void generatoreNome(char nome[]);
void generatoreCognome(char cognome[]);
void generatoreTelefono(char telefono[]);
void generatoreIndirizzo(char indirizzo[]);

int main(int argc, char *argv[])
{
    srand(time(NULL));

    recordRub *rubrica = (recordRub *)calloc(NUM_RECORD_RUBRICA, sizeof(recordRub));
    if (rubrica == NULL)
    {
        perror("Errore durante l'allocazione di memoria \n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_RECORD_RUBRICA; i++)
    {
        generatoreNome(rubrica[i].nome);
        generatoreCognome(rubrica[i].cognome);
        generatoreIndirizzo(rubrica[i].indirizzo);
        generatoreTelefono(rubrica[i].telefono);
    }

    FILE *RubricaDB = fopen("RubricaDB", "w+");
    if (RubricaDB == NULL)
    {
        generazioneErrore("Il file RubricaDB non è stato aperto \n");
    }

    for (int i = 0; i < NUM_RECORD_RUBRICA; i++)
    {
        fwrite(&rubrica[i], sizeof(recordRub), 1, RubricaDB);
    }

    fclose(RubricaDB);

    printf("Rubrica Telefonica: \n");

    for (int i = 0; i < NUM_RECORD_RUBRICA; i++)
    {
        printf("Elemento %d della rubrica: \n", i + 1);
        printf("Nome: %s \n", rubrica[i].nome);
        printf("Cognome: %s \n", rubrica[i].cognome);
        printf("Indirizzo: %s \n", rubrica[i].indirizzo);
        printf("Telefono: %s \n", rubrica[i].telefono);
        printf("------------------------------------------------------------- \n");
    }

    return 0;
}

void generatoreNome(char nome[])
{

    const char *nomi[100] = {
        "Alessandro", "Alessandra", "Andrea", "Alice", "Antonio", "Anna", "Carlo", "Arianna", "Claudio", "Barbara",
        "Davide", "Beatrice", "Enrico", "Camilla", "Fabio", "Chiara", "Federico", "Claudia", "Francesco", "Cristina",
        "Gabriele", "Daniela", "Gianluca", "Elena", "Giovanni", "Elisa", "Giuseppe", "Elisabetta", "Jacopo",
        "Federica", "Leonardo", "Francesca", "Lorenzo", "Gabriella", "Luca", "Giada", "Marco", "Giulia", "Mario",
        "Ilaria", "Matteo", "Laura", "Michele", "Lavinia", "Niccolo", "Linda", "Paolo", "Lisa", "Pietro", "Lucia",
        "Riccardo", "Maddalena", "Roberto", "Maria", "Salvatore", "Martina", "Stefano", "Michela", "Tommaso", "Miriam",
        "Umberto", "Monica", "Valentino", "Nadia", "Vincenzo", "Nicole", "Alessio", "Paola", "Emanuele", "Roberta",
        "Daniele", "Sara", "Angelo", "Simona", "Raffaele", "Sofia", "Silvio", "Silvia", "Giulio", "Teresa", "Cristian",
        "Valentina", "Filippo", "Vanessa", "Elio", "Veronica", "Samuele", "Vittoria", "Sergio", "Irene", "Vittorio",
        "Elettra", "Aldo", "Alessia", "Ettore", "Ginevra", "Piero", "Emma", "Maurizio", "Bianca"};

    strcpy(nome, nomi[rand() % 99]);
}

void generatoreCognome(char cognome[])
{

    const char *cognomi[30] = {
        "Rossi", "Ferrari", "Russo", "Bianchi", "Romano", "Colombo", "Ricci", "Marino", "Greco", "Bruno", "Gallo",
        "Conti", "De Luca", "Mancini", "Costa", "Giordano", "Rizzo", "Lombardi", "Moretti", "Barbieri", "Fontana",
        "Santoro", "Mariani", "Rinaldi", "Caruso", "Ferrara", "Galli", "Martini", "Leone", "Longo"};

    strcpy(cognome, cognomi[rand() % 29]);
}

void generatoreTelefono(char telefono[])
{

    int i = 0;
    telefono[i++] = '3';

    for (i = 1; i < 10; i++)
    {
        telefono[i] = rand() % 9 + '0';
    }

    telefono[i] = '\0';
}

void generatoreIndirizzo(char indirizzo[])
{

    const char *vie[20] = {
        "Via Roma", "Via Garibaldi", "Corso Vittorio Emanuele", "Via Dante Alighieri", "Via Giuseppe Mazzini",
        "Via Giacomo Matteotti", "Via Giovanni Pascoli", "Via Leonardo Da Vinci", "Via Alessandro Volta",
        "Via Galileo Galilei", "Via Cesare Battisti", "Via Francesco Petrarca", "Via Ugo Foscolo",
        "Via Antonio Gramsci", "Via Luigi Pirandello", "Via Guglielmo Marconi", "Via Enrico Fermi",
        "Via Vittorio Veneto", "Via Aldo Moro", "Via Sandro Pertini"};

    const char *province[20] = {
        "Roma", "Milano", "Napoli", "Torino", "Palermo", "Genova", "Bologna", "Firenze", "Bari", "Catania",
        "Venezia", "Verona", "Messina", "Padova", "Trieste", "Taranto", "Brescia", "Parma", "Modena", "Reggio Calabria"};

    strcat(indirizzo, vie[rand() % 19]);
    strcat(indirizzo, " ");

    char civico[4];
    sprintf(civico, "%d", rand() % 200);

    strcat(indirizzo, civico);
    strcat(indirizzo, " ");
    strcat(indirizzo, province[rand() % 19]);
}
