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
#define BUFFER_size 2048

    //structures
typedef struct 
{
   
   char nom[50];
   int membres[nombre_clients];
} Salon;

typedef struct 
{
   int descripteur;
   char pseudo[50];
   int salon_id;
} clients;


int main(){

    //tableau des clients connectes
    clients client[nombre_clients];
    //initialisation du tebleau
    for ( int i = 0; i < nombre_clients; i++)
    {
        client[i].descripteur = 0;
        strcpy(client[i].pseudo,"inconnu");
        client[i].salon_id = -1;
    }

    //declarer 10 salons
    Salon salons[10]; // max 10 salons 
    int nombre_salons = 0;
    
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
            int descripteur_socket = client[i].descripteur;
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
                if (client[i].descripteur == 0)
                {
                    client[i].descripteur = client_connecte;
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
                int descripteur = client[i].descripteur;//on reccupere le descripteur de chaque client
                if (FD_ISSET(descripteur,&ensembles_sockets))//on verifie si le client a eu une activite: envoie de message ou deconnecter 
                {   char tampon[BUFFER_size];
                    memset(tampon,0,BUFFER_size);
                    int message_recu = recv(descripteur,tampon,BUFFER_size,0);

                    //verification du message du client
                    if (message_recu <= 0)
                    {
                        close(descripteur);//fermeture de la session
                        client[i].descripteur = 0;
                        printf("(SERVEUR) client déconnecté.\n"); 
                    }
                    else
                    {
                        printf("client[%d]: %s\n",i,tampon);

                        // Gestion du /nick
                        if (strncmp(tampon,"/nick ",6) == 0)
                        {
                            char *nouveau_pseudo = tampon + 6;
                            strncpy(client[i].pseudo,nouveau_pseudo,49);
                            client[i].pseudo[49] = '\0';
                            char message[BUFFER_size];
                            snprintf(message,sizeof(message),"Pseudo enregistré : %s\n",client[i].pseudo);
                            send(descripteur,message,strlen(message),0);
                        }

                        //Gestion de /who
                        else if (strcmp(tampon,"/who") == 0)
                        {
                            char liste[BUFFER_size] = "Utilisateur connectés :\n";
                            for (int j = 0; j < nombre_clients; j++)
                            {
                                if (client[j].descripteur != 0)
                                {
                                    strcat(liste,"-");
                                    strcat(liste,client[j].pseudo);
                                    strcat(liste,"\n");
                                }
                                
                            }
                            send(descripteur,liste,strlen(liste),0);
                        }
                        //Gestion de /whois
                        else if (strncmp(tampon,"/whois ",7) == 0)
                        {
                            char *recherche = tampon + 7;
                            int trouve = 0;
                            for (int j = 0; j < nombre_clients; j++)
                            {
                                if (client[i].descripteur != 0 && strcmp(client[j].pseudo,recherche) == 0)
                                {
                                    char info[BUFFER_size];
                                    snprintf(info,sizeof(info),"[Serveur] : %s est connecté sur le socket %d\n",recherche,client[i].descripteur);
                                    send(descripteur,info,strlen(info),0);
                                    trouve = 1;
                                    break;

                                }
                                
                            }
                            if (!trouve)
                            {
                                send(descripteur,"[Serveur]: utilisateur introuvable\n",35,0);

                            }
                            
                        }
                        //gestion de /create <nom_salon>
                        else if (strncmp(tampon, "/create ", 8) == 0) {
                            char *nom = tampon + 8;
                            int existe = 0;
                            for (int s = 0; s < nombre_salons; s++) {
                                if (strcmp(salons[s].nom, nom) == 0) {
                                    existe = 1;
                                    break;
                                }
                            }
                            if (existe) {
                                send(descripteur, "Salon déjà existant\n", 33, 0);
                            } else {
                                strcpy(salons[nombre_salons].nom, nom);
                                for (int j = 0; j < nombre_clients; j++)
                                salons[nombre_salons].membres[j] = 0;
                                salons[nombre_salons].membres[i] = 1;
                                client[i].salon_id = nombre_salons;
                                nombre_salons++;
                                send(descripteur, "Salon créé et rejoint\n", 33, 0);
                            }
                        }
                        //gestion de /join <nom_salon>
                        else if (strncmp(tampon, "/join ", 6) == 0) {
                            char *nom = tampon + 6;
                            int trouve = -1;
                            for (int s = 0; s < nombre_salons; s++) {
                                if (strcmp(salons[s].nom, nom) == 0) {
                                    trouve = s;
                                    break;
                                }
                            }
                            if (trouve == -1) {
                                send(descripteur, "Salon introuvable\n", 30, 0);
                            } else {
                                salons[trouve].membres[i] = 1;
                                client[i].salon_id = trouve;
                                send(descripteur, "Salon rejoint\n", 27, 0);
                            }
                        }
                        //gestion de la broadcast /all
                        else if (strncmp(tampon, "/all ", 5) == 0) {
                            char *message = tampon + 5;
                            for (int j = 0; j < nombre_clients; j++) {
                                if (client[j].descripteur != 0 && j != i) {
                                    char reponse[BUFFER_size];
                                    snprintf(reponse, sizeof(reponse), "[%s] : %s\n", client[i].pseudo, message);
                                    send(client[j].descripteur, reponse, strlen(reponse), 0);
                                }
                            }
                        }
                        //gestion /msg
                        else if (strncmp(tampon, "/msg ", 5) == 0) {
                            char *cible = strtok(tampon + 5, " ");
                            char *message = strtok(NULL, "");
                            int trouve = 0;
                            for (int j = 0; j < nombre_clients; j++) {
                                if (client[j].descripteur != 0 && strcmp(client[j].pseudo, cible) == 0) {
                                    char reponse[BUFFER_size];
                                    snprintf(reponse, sizeof(reponse), "[Privé de %s] : %s\n", client[i].pseudo, message);
                                    send(client[j].descripteur, reponse, strlen(reponse), 0);
                                    trouve = 1;
                                    break;
                                }
                            }
                            if (!trouve) {
                                send(descripteur, "[Serveur] : Utilisateur introuvable", 35, 0);
                            }
                        }

                        //gestion de leave
                        else if (strcmp(tampon, "/leave") == 0) {
                            int s = client[i].salon_id;
                            if (s == -1) {
                                send(descripteur, "Vous n'êtes dans aucun salon", 40, 0);
                            } else {
                                salons[s].membres[i] = 0;
                                client[i].salon_id = -1;
                                send(descripteur, "Salon quitté\n", 27, 0);

                                // Vérifier si le salon est vide
                                int vide = 1;
                                for (int j = 0; j < nombre_clients; j++) {
                                    if (salons[s].membres[j] == 1) {
                                        vide = 0;
                                        break;
                                    }
                                }
                                if (vide) {
                                    for (int k = s; k < nombre_salons - 1; k++) {
                                        salons[k] = salons[k + 1];
                                        for (int j = 0; j < nombre_clients; j++) {
                                            if (client[j].salon_id == k + 1) client[j].salon_id = k;
                                        }
                                    }
                                    nombre_salons--;
                                }
                            }
                        }

                        //Gestion de /quit
                        else if (strcmp(tampon, "/quit") == 0)
                         { 
                            const char *message = "[Server] : You will be terminated\n";
                            send(descripteur, message, strlen(message), 0); 
                            close(descripteur);
                            client[i].descripteur = 0; 
                            puts("(SERVEUR) Déconnexion demandée par client.");
                        }
                        else {
                            int s = client[i].salon_id;
                            if (s != -1) {
                                for (int j = 0; j < nombre_clients; j++) {
                                    if (client[j].descripteur != 0 && salons[s].membres[j] == 1 && j != i) {
                                        char reponse[BUFFER_size];
                                        snprintf(reponse, sizeof(reponse), "[%s dans %s] : %.900s\n", client[i].pseudo, salons[s].nom, tampon);
                                        send(client[j].descripteur, reponse, strlen(reponse), 0);
                                    }
                                }
                            } else {
                                send(descripteur, "Vous n'êtes dans aucun salon\n", 40, 0);
                            }
                        }

                }
                
            }
    }
        }


        //fermeture et liberation des ressources
        close(socketserver);
        return 0;


}