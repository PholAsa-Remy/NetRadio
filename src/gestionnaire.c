#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/select.h>
#include <fcntl.h>
#include <strings.h>

#define MAX_DIFFUSEUR 50
#define ID 8
#define PORT 4
#define IP 15
#define TAILLE_MSG 57       // 9 (REGI + 5 espaces) + ID + 2*IP + 2*PORT + 2(\r\n)
#define TAILLE_MAX_BUFFER 160

pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

//adresse ip et descripteur du client
struct client {
    int descripteur;
    u_int32_t ip;
};
// test : REGI 48484848 123.123.123.123 1234 123.123.123.124 1523\r\n
typedef struct diffuseur {
    char  * id;     //Taille 8
    char * ip1;     //Taille 15, adresse multidiffusion
    char * port1;   //Taille 4, port multidiffusion
    char * ip2 ;    //Taille 15, adresse écoute
    char * port2;   //Taille 4, port écoute
    int index;
}diffuseur;


// stockage des infos des diffuseurs
diffuseur * list_diffuseur;

int diffuseurPresent (){
    pthread_mutex_lock(&verrou);
    int count = 0;
    for (int i = 0; i < MAX_DIFFUSEUR; i++)
        if (strcmp(list_diffuseur[i].id, "") != 0)
            count ++;
    pthread_mutex_unlock(&verrou);
    return count;
}

void afficheListeDiffuseur (){
    char * tmpId = malloc (sizeof(char) * (ID + 1));
    char * tmpIp = malloc (sizeof(char) * (IP + 1));
    char * tmpPort = malloc (sizeof(char) * (PORT + 1));
    memset (tmpId, '\0', ID + 1);
    memset (tmpIp, '\0', IP + 1);
    memset (tmpPort, '\0', PORT + 1);
    int nbDiffuseur = diffuseurPresent ();
    printf("Liste de diffuseur : taille %d\n", nbDiffuseur);
    for (int i = 0; i < MAX_DIFFUSEUR; i++){
        if (strcmp(list_diffuseur[i].id, "") != 0){
            memset (tmpId, '\0', ID + 1);
            memset (tmpIp, '\0', IP + 1);
            memset (tmpPort, '\0', PORT + 1);
            strncat (tmpId, list_diffuseur[i].id, ID);
            strncat (tmpIp, list_diffuseur[i].ip1, IP);
            strncat (tmpPort, list_diffuseur[i].port1, PORT);

            printf("ID : %s\n",tmpId);

            printf("ADRESSE MULTIDIFFUSION : %s\n",tmpIp);

            memset (tmpIp, '\0', IP + 1);
            strncat (tmpIp, list_diffuseur[i].ip2, IP);
            printf("ADRESSE D'ECOUTE : %s\n",tmpIp);

            printf("PORT DE MULTIDIFFUSION : %s\n",tmpPort);

            memset (tmpPort, '\0', PORT + 1);
            strncat (tmpPort, list_diffuseur[i].port2, PORT);
            printf("PORT D'ECOUTE : %s\n\n",tmpPort);
        }
    }
    free (tmpId);
    free (tmpIp);
    free (tmpPort);
}

//ajoute le diffuseur et retour son positionnement dans la liste
int ajoutDiffuseur (diffuseur d){
    char * buff = malloc (sizeof(char) * ID);
    memset(buff,'\0', ID);
    for (int i = 0; i < MAX_DIFFUSEUR; i++){
        memcpy(buff, list_diffuseur[i].id, ID);
        if(strcmp(list_diffuseur[i].id, "") == 0){
            pthread_mutex_lock(&verrou);
            free(list_diffuseur[i].id);
            free(list_diffuseur[i].ip1);
            free(list_diffuseur[i].ip2);
            free(list_diffuseur[i].port1);
            free(list_diffuseur[i].port2);
            list_diffuseur[i] = d;
            free (buff);
            pthread_mutex_unlock(&verrou);
            return i;
        }
    }
    free (buff);
    printf("Bug lors de l'ajout du diffuseur");
    return -1;
}

void suppDiffuseur (int index){
    pthread_mutex_lock(&verrou);
    free(list_diffuseur[index].id);
    free(list_diffuseur[index].ip1);
    free(list_diffuseur[index].ip2);
    free(list_diffuseur[index].port1);
    free(list_diffuseur[index].port2);

    diffuseur vide = {.id = malloc(ID), .ip1 = malloc(IP), .ip2 = malloc(IP), .port1 = malloc(PORT), .port2 = malloc(PORT)};
    
    memset(vide.id,'\0',ID);
    memset(vide.ip1,'\0',IP);
    memset(vide.ip2,'\0',IP);
    memset(vide.port1,'\0',PORT);
    memset(vide.port2,'\0',PORT);
    
    list_diffuseur[index] = vide;
    pthread_mutex_unlock(&verrou);
}

//vérifie le nombre et ajoute un 0 devant si x < 10
char * verifNombre (int x){
    char * nombre = malloc (sizeof(char) * 2);
    memset (nombre, '\0', 2);
    if ( x < 10 ){
        sprintf(nombre,"0%d",x);
        return nombre;
    }
    sprintf(nombre,"%d",x);
    return nombre;
}

void extension_mess (char * buff){
    for (int i = 0; i < MAX_DIFFUSEUR; i++){
        if (strcmp(list_diffuseur[i].id, "") != 0){
            int p=atoi(list_diffuseur[i].port2);
            int descripteur = socket (PF_INET,SOCK_STREAM,0);

            struct sockaddr_in address_sock;
            address_sock.sin_family=AF_INET;
            address_sock.sin_port=htons(p);
            address_sock.sin_addr.s_addr=htonl(INADDR_ANY);

            connect(descripteur, (struct sockaddr *) &address_sock, sizeof(struct sockaddr_in));
            send(descripteur,buff,156,0);
            close(descripteur);
        }
    }
}

void ca_va(int descripteur,char * buff){

    char * tmpId  = malloc(sizeof(char) * ID);
    char * tmpIp1 = malloc(sizeof(char) * IP);
    char * tmpIp2 = malloc(sizeof(char) * IP);
    char * tmpPort1 = malloc(sizeof(char) * PORT);
    char * tmpPort2 = malloc(sizeof(char) * PORT);

    memset(tmpId,'\0',ID);
    memset(tmpIp1,'\0',IP);
    memset(tmpIp2,'\0',IP);
    memset(tmpPort1,'\0',PORT);
    memset(tmpPort2,'\0',PORT);

    strncat (tmpId, buff + 5, ID);
    strncat (tmpIp1, buff + 5 + ID + 1, IP);
    strncat (tmpPort1, buff + 5 + ID + 2 + IP, PORT);
    strncat (tmpIp2, buff + 5 + ID + 3 + IP + PORT, IP);
    strncat (tmpPort2, buff + 5 + ID + 4 + 2*IP + PORT, PORT);

    diffuseur d = {.id = malloc(ID), .ip1 = malloc(IP), .ip2 = malloc(IP), .port1 = malloc(PORT), .port2 = malloc(PORT)};
    strcpy(d.id,tmpId);
    strcpy(d.ip1,tmpIp1);
    strcpy(d.ip2,tmpIp2);
    strcpy(d.port1,tmpPort1);
    strcpy(d.port2,tmpPort2);

    int index = ajoutDiffuseur(d);
    send(descripteur, "REOK\r\n", 6, 0);
    afficheListeDiffuseur ();

    free (tmpId);
    free (tmpIp1);
    free (tmpIp2);
    free (tmpPort1);
    free (tmpPort2);
    
    char * buff_cava = malloc (sizeof(char) * 7);
    memset (buff_cava, '\0', 7);

    fcntl(descripteur, F_SETFL, O_NONBLOCK);

    while(1){
        memset(buff_cava, '\0', 7);
        send(descripteur, "RUOK\r\n", 6, 0);
        sleep(10);
        recv(descripteur,buff_cava, 6,0);
        printf("Buffer reçu : %s\n", buff_cava);
        
        if (strncmp(buff_cava, "IMOK", 4) == 0){
            printf("Bonne réponse, je continue.\n");
            continue;
        }else{
            printf("Mauvaise réponse, au revoir Diffuseur.\n");
            break;
        }
    }
    suppDiffuseur (index);
    free(buff_cava);
}

void envoieListe (int descripteur, int nbDiffuseur, char * envoieNumDiff, char * buffDiffuseur){
    nbDiffuseur = diffuseurPresent();
    printf("Nombre de diffuseur présent : %d\n",nbDiffuseur);
    sprintf(envoieNumDiff,"LINB %s\r\n",verifNombre(nbDiffuseur));
    send(descripteur,envoieNumDiff, 9, 0);
    printf("Message d'envoie Numdiffuseur : %s\n",envoieNumDiff);

    for(int i = 0; i < MAX_DIFFUSEUR; i++){
        if(strcmp(list_diffuseur[i].id, "") != 0){
            memset(buffDiffuseur,'\0',TAILLE_MSG + 5);
            memcpy(buffDiffuseur, "ITEM ", 5);
            memcpy(buffDiffuseur + 5, list_diffuseur[i].id, ID);
            memcpy(buffDiffuseur + 5 + ID, " ", 1);
            memcpy(buffDiffuseur + 5 + ID + 1 , list_diffuseur[i].ip1, IP);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP, " ", 1);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP + 1, list_diffuseur[i].port1, PORT);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP + 1 + PORT, " ", 1);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP + 1 + PORT + 1, list_diffuseur[i].ip2, IP);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP + 1 + PORT + 1 + IP, " ", 1);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP + 1 + PORT + 1 + IP + 1, list_diffuseur[i].port2, PORT);
            memcpy(buffDiffuseur + 5 + ID + 1 + IP + 1 + PORT + 1 + IP + 1 + PORT, "\r\n", 2);

            printf("Liste message envoyé : %s\n",buffDiffuseur);

            int r = send(descripteur,buffDiffuseur, TAILLE_MSG+2, 0);
            if (r < 0){
                printf("Erreur sur l'envoie des listes de diffuseurs");
            }
        }
    }
}

void*communication(void *arg){

    //recupération du client (IP + descripteur)
    struct client * utilisateur = (struct client *)arg;
    int descripteur = utilisateur -> descripteur;

    //buff pour la reception des commandes de taille 160 pour être large
    char * buff = malloc (sizeof(char) * TAILLE_MAX_BUFFER);
    memset(buff,'\0', TAILLE_MAX_BUFFER);

    int nbDiffuseur = 0;

    //buff pour le diffuseur + 5 au cas où
    char * buffDiffuseur = malloc (sizeof(char) * TAILLE_MSG + 5); 
    memset(buffDiffuseur,'\0',TAILLE_MSG + 5);

    //LIMB + espace + nb entre 00 à 99 + 1 au cas où = 10
    char * envoieNumDiff = malloc (sizeof(char) * 10);
    memset(envoieNumDiff,'\0', 10);

    //reception du message dans le buff
    int r = recv(descripteur,buff,(TAILLE_MAX_BUFFER)*sizeof(char),0);

    if (r <= 0){
        printf("Message vide, Fin de la connexion ...\n");
    }

    //condition REGI
    if((strncmp(buff, "REGI ", 5) == 0) && r == TAILLE_MSG){

        printf("Condition REGI\n");
        if(diffuseurPresent() < 50){
            ca_va(descripteur,buff);
        }else{
            send(descripteur, "RENO\r\n", 6, 0);
            printf("Fin REGI\n");
        }

    //condition LIST
    }else if ((strncmp(buff, "LIST\r\n", 6) == 0) && r == 6){

        printf("Condition LIST\n");
        envoieListe(descripteur, nbDiffuseur, envoieNumDiff, buffDiffuseur);
        printf("Fin d'envoie de LIST\n");

    //condition MESS, le message doit être de taille 156
    }else if ((strncmp(buff, "MESS", 4) == 0) && r == 156){

        printf("Condition MESS\n");
        send(descripteur, "ACKM\r\n", 6, 0);
        extension_mess(buff);
        printf("Fin d'envoie de MESS\n");

    }else{
        printf("Votre message : %s\n",buff);
        printf("Mauvais format du message, fermeture de la connexion\n");
    }
    free (buff);
    free (buffDiffuseur);
    free (envoieNumDiff);  
    close(descripteur);
    return NULL;
}



int main(int argc, char**argv) {
    if(argc!=2){
        printf("Erreur il faut fournir un numero de port\n");
        return 0;
    }
    //remplissage d'une liste vide de diffuseur
    list_diffuseur = malloc (sizeof(diffuseur) * MAX_DIFFUSEUR);
    for(int i = 0; i < MAX_DIFFUSEUR; i++){
        diffuseur vide = {.id = malloc(ID), .ip1 = malloc(IP), .ip2 = malloc(IP), .port1 = malloc(PORT), .port2 = malloc(PORT)};;
        memset(vide.id,'\0',ID);
        memset(vide.ip1,'\0',IP);
        memset(vide.ip2,'\0',IP);
        memset(vide.port1,'\0',PORT);
        memset(vide.port2,'\0',PORT);

        list_diffuseur[i] = vide;
    }
    
    int p=atoi(argv[1]);
    int sock=socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in address_sock;
    address_sock.sin_family=AF_INET;
    address_sock.sin_port=htons(p);
    address_sock.sin_addr.s_addr=htonl(INADDR_ANY);
    int r=bind(sock,(struct sockaddr *)&address_sock,sizeof(struct sockaddr_in));

    if(r==0){
        r=listen(sock,0);
        while(1){
            struct sockaddr_in caller;
            socklen_t size=sizeof(caller);
            int *sock2=(int *)malloc(sizeof(int));
            *sock2=accept(sock,(struct sockaddr *)&caller,&size);
            if(sock2>=0){
                //Ici on affiche les informations du client

                printf("Port de l'appelant: %d\n",ntohs(caller.sin_port));
                printf("Adresse de l'appelant: %s\n",inet_ntoa(caller.sin_addr));
                struct client utilisateur = {.descripteur = *sock2, .ip = caller.sin_addr.s_addr};

                //creation du thread pour chaque client
                pthread_t th;
                int r2=pthread_create(&th,NULL,communication, &utilisateur);
                if(r2!=0){
                    printf("Problème de creation de thread");
                    exit(0);
                }
            }
        }
    }
    return 0;
}