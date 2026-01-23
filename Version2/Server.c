#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>

    //GNU/linux
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>
    //constante
#define port_ecoute 5094
#define nombre_clients 20
#define BUFFER_size 1024
int main(){

    //tableau des clients connectes
    int clients[nombre_clients];
    //initialisation du tebleau
    for ( int i = 0; i < nombre_clients; i++)
    {
        clients[i] = 0;
    }
    


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

    while (1)
    {
        fd_set ensembles_sockets; //ensemble des sockets a surveiller
        FD_ZERO(&ensembles_sockets); //reset l'ensemble
        FD_SET(socketserver,&ensembles_sockets); //ajout du socket server a l'ensemble
        int max_decripteur = socketserver; //reccupere le descripteur du socket serveur
        
        //ajouter les clients connectes a l'ensembles des sockets a surveiller
        for (int i = 0; i < nombre_clients; i++)
        {
            int descripteur_socket = clients[i];
            if(descripteur_socket > 0){
                FD_SET(descripteur_socket,&ensembles_sockets);
            }
            if(descripteur_socket > max_decripteur){
                max_decripteur =descripteur_socket;
            }
        }

        //attendre l'activite d'au moins un des sockets a surveiller: demande de connection/deconnection,envoi de message
        int activite = select(max_decripteur + 1,&ensembles_sockets,NULL,NULL,NULL);
        if(activite < 0){
                perror("(SERVEUR) Erreur de select");
            }

        //acceptation des connexions
        struct sockaddr_in clientAddress;

        if (FD_ISSET(socketserver,&ensembles_sockets))// si le socket serveur est en activite: demande de connection
        {
           
            
            socklen_t taille_adresse_client = sizeof(clientAddress);
            int client_connecte = accept(socketserver,(struct sockaddr*)&clientAddress,&taille_adresse_client);
           
            if (client_connecte == -1)
            {
                fprintf(stderr,"(SERVEUR) echec d'etablissement de la connexion\n");
                continue;
            }

            //ajout du client au tableau de client
            int ajoute = 0;
            for(int i = 0; i<nombre_clients; i++){
                if (clients[i] == 0)
                {
                    clients[i] = client_connecte;
                    ajoute +=  1;
                    printf("(SERVEUR) Nouveau client accepté \n");
                    break;
                }
                
            }
        
             //un message est envoyer au client si le nombre de connection est atteint
                if (!ajoute)
                {
                    const char *message = "Server cannot accept incoming connections anymore. Try again later.";
                    send(client_connecte,message,strlen(message),0);
                    close(client_connecte);
                }

            
        }
         //
            for (int i = 0; i < nombre_clients; i++)//on parcour le tableau de client
            {
                int descripteur = clients[i];//on reccupere le descripteur de chaque client
                if (FD_ISSET(descripteur,&ensembles_sockets))//on verifie si le client a eu une activite: envoie de message ou deconnecter 
                {   char tampon[BUFFER_size];
                    memset(tampon,0,BUFFER_size);
                    int message_recu = recv(descripteur,tampon,BUFFER_size,0);

                    //verification du message du client
                    if (message_recu <= 0)
                    {
                        close(descripteur);//fermeture de la session
                        clients[i] = 0;
                        printf("(SERVEUR) client déconnecté.\n"); 
                    }
                    else
                    {
                        printf("client[%d]: %s\n",i,tampon);

                        if (strcmp(tampon, "/quit") == 0)
                         { 
                            const char *message = "[Server] : You will be terminated\n";
                            send(descripteur, message, strlen(message), 0); 
                            close(descripteur);
                            clients[i] = 0; 
                            puts("(SERVEUR) Déconnexion demandée par client.");
                        }
                        else{
                            char reponse[BUFFER_size + 20];
                            snprintf(reponse,sizeof(reponse),"[Server] : %s\n",tampon);
                            send(descripteur,reponse,strlen(reponse),0);
                        }
                }
                
            }
    }
        }


        //fermeture et liberation des ressources
        close(socketserver);
        return 0;


}