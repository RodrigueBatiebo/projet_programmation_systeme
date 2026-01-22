#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>

    //GNU/linux
#include<netinet/in.h>
#include<sys/socket.h>

    //constante
#define port_ecoute 5094
#define nombre_clients 1
#define BUFFER_size 1024
int main(){
    
    //creation du socket
    int socketserver = socket(AF_INET,SOCK_STREAM,0);
    if(socketserver == -1){
        fprintf(stderr,"(SERVEUR) echec d'initialisation du socket");
        exit(1);
    }

    //configuration du socket
    struct sockaddr_in adresse_socket;
    adresse_socket.sin_family = AF_INET;
    adresse_socket.sin_port = htons(port_ecoute);
    adresse_socket.sin_addr.s_addr = INADDR_ANY;

    //liaison du socket a une adresse ip et a un port
    int taille_adressesocket = sizeof(adresse_socket);
    int liaison_socket = bind(socketserver,(struct sockaddr*)&adresse_socket,taille_adressesocket);
    if(liaison_socket == -1){
        fprintf(stderr,"(SERVEUR) echec de liaison pour le socket\n");
        exit(1);
    }

    //le server en mode ecoute des clients
    if (listen(socketserver,nombre_clients) == -1)
    {
        fprintf(stderr,"(SERVEUR) echec de demarage de l'ecoute des connexions entrants\n");
        exit(1);
    }
    puts("en attente de nouvelles  connexions");

    //acceptation des connexions
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int client_connecte = accept(socketserver,(struct sockaddr*)&clientAddress,&clientAddressLength);

    if (client_connecte == -1)
    {
        fprintf(stderr,"(SERVEUR) echec d'etablissement de la connexion\n");
        exit(1);
    }


    //reception du message d'un client et reponse du serveur
    char buffer[BUFFER_size] = {0};
    while(1){

        memset(buffer,0,BUFFER_size);
        int message_recu = recv(client_connecte,buffer,BUFFER_size,0);
        if (message_recu <= 0) { 
            puts("(SERVEUR) Connexion fermée par le client.");
            break;

        }
        printf("client : %s\n",buffer);

        //si le client envoi /quit, on ferme
        if (strcmp(buffer, "/quit") == 0) { 
            puts("(SERVEUR) Déconnexion demandée par le client.");
            break; 
        }

        // reponse du seveur au client: envoi du meme message 
        if (send(client_connecte, buffer, strlen(buffer), 0) == -1) { 
            perror("(SERVEUR) Erreur d'envoi");
            break; 
        }
    }
        //fermeture et liberation des ressources
        close(client_connecte);
        close(socketserver);
        

        return 0;
}