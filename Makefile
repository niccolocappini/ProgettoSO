all: server generatoreRubrica client

generatoreRubrica: generatoreRubrica.o
		gcc generatoreRubrica.o -o generatoreRubrica

server: server.o
		gcc server.o -o server

client: client.o funzioniClient.o
		gcc client.o -o client

generatoreRubrica.o : generatoreRubrica.c definizioniComuni.h
		gcc -c generatoreRubrica.c
		
server.o : server.c definizioniComuni.h
		gcc -c server.c

client.o : client.c funzioniClient.h definizioniComuni.h
		gcc -c client.c

funzioniClient.o: funzioniClient.c funzioniClient.h
		gcc -c funzioniClient.c

# rm -f : rimozione forzata senza considerare eventuali errori
clean:
		rm -f *.o
		rm -f server generatoreRubrica client
		rm -f RubricaDB
