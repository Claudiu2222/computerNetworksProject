/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
    Intoarce corect identificatorul din program al thread-ului.


   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define TRANSFERSIZE 4096
/* portul folosit */
#define PORT 9000

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
    int idThread; // id-ul thread-ului tinut in evidenta de acest program
    int cl;       // descriptorul intors de accept
} thData;
int min(int a, int b)
{
    return a < b ? a : b;
}
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void receiveType(int sd, char *typeReceived2)
{
    char typeReceived[TRANSFERSIZE];
    int r = recv(sd, typeReceived, min(TRANSFERSIZE,sizeof(typeReceived)), 0);
    printf("%s", typeReceived);
    
    if (strcmp(typeReceived, "latex2html") == 0)
    {
        char *response = "OK";
        if (send(sd, response, TRANSFERSIZE, 0) == -1)
        {
            perror("[client] Error in sending file");
            exit(1);
        }
    }
    else
    {
        char *response = "NAH";
        if (send(sd, response, TRANSFERSIZE, 0) == -1)
        {
            perror("[client] Error in sending file");
            exit(1);
        }
    }
    
}
void receiveSendFile(int sd)
{
    char *typeReceived = "";
   // receiveType(sd, typeReceived);
    // sleep(1);
    writeRecvInfo(sd, 1);
   // char *response="OK";
    //send(sd,response,TRANSFERSIZE,0);
}
void writeRecvInfo(int sd, int isBinary)
{
    FILE *fp;
    char *fileName = "dad2.pdf";
    int r;
    char info[TRANSFERSIZE] = {0};
    if ((fp = fopen(fileName, "wb")) == NULL)
    {
        printf("ERR");
        exit(1);
    }
    long int size;
    r = recv(sd, info, TRANSFERSIZE, 0);
    char *remaining;
    long answer;
    info[r]='\0';
    long int readBytes=0;
    answer = strtol(info, &remaining, 10);
    printf("%ld",answer);
     while (readBytes<answer)
    {
      
        r = recv(sd, info, TRANSFERSIZE, 0);
       
        readBytes+=strlen(info);
        if (r <= 0)
        {
           
            printf("Received %d and in info %d b\n", r, strlen(info));
            fflush(fp);
            fclose(fp);
            bzero(info, TRANSFERSIZE);
            return;
        }
        printf("Received %d and in info %d b\n", r, strlen(info));
        printf("%s\n", info);
        // fprintf(fp, "%s", info);
        if (isBinary == 1)
            fwrite(info, 1, sizeof(info), fp);
        else
            fprintf(fp, "%s", info);
        fflush(fp);
        bzero(info, TRANSFERSIZE);
            printf("%ld | %ld",readBytes,answer);
    }
}
int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int nr; // mesajul primit de trimis la client
    int sd; // descriptorul de socket
    int pid;
    pthread_t th[2]; // Identificatorii thread-urilor care se vor crea
    int i = 0;

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData *td; // parametru functia executata de thread
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);

    } // while
};
static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData *)arg);
    /* am terminat cu acest client, inchidem conexiunea */

    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg)
{
    int nr, i = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    /* if (read (tdL.cl, &nr,sizeof(int)) <= 0)
            {
              printf("[Thread %d]\n",tdL.idThread);
              perror ("Eroare la read() de la client.\n");

            } */

    char *fromType;
    char *toType;
    receiveSendFile(tdL.cl);

    printf("[Thread %d]Mesajul a fost receptionat...%d\n", tdL.idThread, nr);

    /*pregatim mesajul de raspuns */
    // nr++;
    // printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);

    /* returnam mesajul clientului */
    //  if (write (tdL.cl, &nr, sizeof(int)) <= 0)
    // 	{
    // 	 printf("[Thread %d] ",tdL.idThread);
    // 	 perror ("[Thread]Eroare la write() catre client.\n");
    // 	}
    // else
    // 	printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
}
