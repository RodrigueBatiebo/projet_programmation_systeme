#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>

    //GNU/linux
#include <arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>

    //constante
#define localhost "127.0.0.1"
#define port_connexion 5094
#define BUFFER_size 1024

int main(void){

    //creation du socket
    int socketclient = socket(AF_INET,SOCK_STREAM,0);
    if(socketclient == -1){
        fprintf(stderr,"(CLIENT) echec d'initialisation du socket");
        exit(1);
    }

    //configuration du socket
    struct sockaddr_in adresse_socket;
    adresse_socket.sin_family = AF_INET;
    adresse_socket.sin_port = htons(port_connexion);

    //conversion de l'adresse du serveur en binaire
    int conversionbinaire = inet_pton(AF_INET, localhost, &adresse_socket.sin_addr);
    if(conversionbinaire == -1){
        fprintf(stderr,"(CLIENT) adresse invalide ou non prise en charge\n");
        exit(1);
    }

    //connection du sockets au serveur
    int taille_adressesocket = sizeof(adresse_socket);
    int etat_connection = connect(socketclient,(struct sockaddr*) &adresse_socket,taille_adressesocket);
    if(etat_connection == -1){
        fprintf(stderr,"(CLIENT) echec de la connection au serveur\n");
        exit(1);
    }

    //envoi d'un message au serveur et reception d'un message du serveur
    char buffer[BUFFER_size] = {0};
    while (1){
        printf("Entrez un message ('/quit' pour quitter)");
        fgets(buffer,BUFFER_size,stdin);
        buffer[strcspn(buffer,"\n")] = '\0'; //supprime le \n

        //envoi du message
        if(send(socketclient,buffer,strlen(buffer),0) == -1){
            perror("(CLIENT) Erreur d'envoi");
            break;
        }

        if(strcmp(buffer,"/quit") == 0){
            puts("(CLIENT) Déconnexion demandée.");
            break;
        }

        //reception de message

        //vider le buffer
        memset(buffer,0,BUFFER_size);

        int message_recu = recv(socketclient, buffer, BUFFER_size, 0); 
        if (message_recu <= 0) { 
            puts("(CLIENT) Connexion fermée par le serveur."); 
            break; 
        } 
        printf("serveur : %s\n", buffer);
    }
    
    

    // fermerture du socket
        close(socketclient);

        return 0;
}