#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// GNU/linux
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

// constante
#define localhost "127.0.0.1"
#define port_connexion 5094
#define BUFFER_size 1024

int main(void)
{

    // creation du socket
    int socketclient = socket(AF_INET, SOCK_STREAM, 0);
    if (socketclient == -1)
    {
        fprintf(stderr, "(CLIENT) echec d'initialisation du socket");
        exit(1);
    }

    // configuration du socket
    struct sockaddr_in adresse_socket;
    adresse_socket.sin_family = AF_INET;
    adresse_socket.sin_port = htons(port_connexion);

    // conversion de l'adresse du serveur en binaire
    int conversionbinaire = inet_pton(AF_INET, localhost, &adresse_socket.sin_addr);
    if (conversionbinaire == -1)
    {
        fprintf(stderr, "(CLIENT) adresse invalide ou non prise en charge\n");
        exit(1);
    }

    // connection du sockets au serveur
    int taille_adressesocket = sizeof(adresse_socket);
    int etat_connection = connect(socketclient, (struct sockaddr *)&adresse_socket, taille_adressesocket);
    if (etat_connection == -1)
    {
        fprintf(stderr, "(CLIENT) echec de la connection au serveur\n");
        exit(1);
    }

    // demande du pseudo du client
    char buffer[BUFFER_size] = {0};
    char pseudo[50];
    printf("Entrez votre pseudo(ex: rodrigue) : ");
    fgets(pseudo, sizeof(pseudo), stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0';
    char commande_nick[BUFFER_size];
    snprintf(commande_nick, sizeof(commande_nick), "/nick %s", pseudo);
    send(socketclient, commande_nick, strlen(commande_nick), 0);

    // reception de confirmation
    recv(socketclient, buffer, BUFFER_size, 0);
    printf("serveur : %s\n", buffer);

    // Enumeration des commandes prise en charge
    puts("Commandes disponibles :");
    puts("  /who       → liste des utilisateurs");
    puts("  /whois nom → infos sur un utilisateur");
    puts("  /all message      → broadcast à tous sauf a vous");
    puts("  /msg pseudo → message privé");
    puts("  /create nom_salon → créer un salon");
    puts("  /join nom_salon → rejoindre un salon");
    puts("  /sendfile pseudo nomfichier → envoyer un fichier");
    puts("  /join nom_salon → rejoindre un salon");
    puts("  /leave      → quitter le salon");
    puts("  /quit      → se deconnecter du serveur");
    puts("Tapez un message ou une commande ci-dessous :");

    // envoi d'un message au serveur et reception d'un message du serveur

    while (1)
    {

        fd_set ensemble;
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble); // surveille le clavier
        FD_SET(socketclient, &ensemble); // surveille le serveur
        int maxfd = (STDIN_FILENO > socketclient ? STDIN_FILENO : socketclient) + 1;

        if (select(maxfd, &ensemble, NULL, NULL, NULL) > 0)
        {
            // activité clavier
            if (FD_ISSET(STDIN_FILENO, &ensemble))
            {
                char buffer_envoi[BUFFER_size];
                printf("➤ Message : ");
                fgets(buffer_envoi, BUFFER_size, stdin);
                buffer_envoi[strcspn(buffer_envoi, "\n")] = '\0';

                if (send(socketclient, buffer_envoi, strlen(buffer_envoi), 0) == -1)
                {
                    perror("(CLIENT) Erreur d'envoi");
                    break;
                }

                if (strcmp(buffer_envoi, "/quit") == 0)
                {
                    puts("(CLIENT) Déconnexion demandée.");
                    break;
                }
                // gestion de /sendfile
                if (strncmp(buffer_envoi, "/sendfile ", 10) == 0)
                {
                    char *cible = strtok(buffer_envoi + 10, " ");
                    char *nom_fichier = strtok(NULL, "");

                    if (cible == NULL || nom_fichier == NULL)
                    {
                        puts("(CLIENT) Usage : /sendfile <pseudo> <nom_fichier>");
                    }
                    else
                    {
                        int fd = open(nom_fichier, O_RDONLY);
                        if (fd == -1)
                        {
                            perror("(CLIENT) Erreur ouverture fichier");
                        }
                        else
                        {
                            // informer le serveur
                            char commande[BUFFER_size];
                            snprintf(commande, sizeof(commande), "/sendfile %s %s", cible, nom_fichier);
                            send(socketclient, commande, strlen(commande), 0);

                            // envoyer le contenu
                            char bloc[BUFFER_size];
                            int lu;
                            while ((lu = read(fd, bloc, BUFFER_size)) > 0)
                            {
                                send(socketclient, bloc, lu, 0);
                            }
                            close(fd);
                            puts("(CLIENT) Fichier envoyé au serveur.");
                        }
                    }
                }
            }

            // activité serveur
            if (FD_ISSET(socketclient, &ensemble))
            {
                char buffer_reception[BUFFER_size];
                int recu = recv(socketclient, buffer_reception, BUFFER_size, 0);
                if (recu <= 0)
                {
                    puts("(CLIENT) Connexion fermée par le serveur.");
                    break;
                }
                if (strncmp(buffer_reception, "[FILE]", 6) == 0) {
    int fd = open("recu.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1) {
        write(fd, buffer_reception + 6, recu - 6);
        close(fd);
        puts("(CLIENT) Fichier reçu et sauvegardé dans 'recu.txt'.");
    } else {
        perror("(CLIENT) Erreur écriture fichier");
    }
} else {
    buffer_reception[recu] = '\0';
    printf("serveur : %s\n", buffer_reception);
}

            }
        }
    }

    // fermerture du socket
    close(socketclient);

    return 0;
}