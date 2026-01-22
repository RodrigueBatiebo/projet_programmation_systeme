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

    //envoi d'un message au serveur
    const char message[] = "hello world!";
    int message_envoye = send(socketclient,message,strlen(message),0);
    if(message_envoye == -1){
        fprintf(stderr,"(CLIENT) echec d'envoi du message au serveur'\n");
        exit(1);
    } 
    // reception d'un message 
    char buffer[BUFFER_size] = {0};
    int message_recu = recv(socketclient,buffer,BUFFER_size,0);
    if(message_recu == -1){
        fprintf(stderr,"(CLIENT) echec de reception du message du seveur'\n");
        exit(1);
    }
    printf("serveur : %s\n",buffer);

    // fermerture du socket
        close(socketclient);

        return 0;
}