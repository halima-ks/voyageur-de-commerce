#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main() {
    char *prog1 = "./liste"; // Nom du premier programme (Liste)
    char *prog2 = "./vecteur"; // Nom du deuxième programme (Vecteur)
    int iterations = 5; // Nombre d'exécutions pour chaque programme

    for (int i = 0; i < iterations; i++) {
        printf("################################################################\n");
        printf("Comparaison entre Liste et Vecteur pour le voyageur de commerce (Itération %d):\n", i + 1);
        printf("\nLISTE :  \n\n");

        pid_t p1 = fork();
        if (p1 == 0) {
            execl(prog1, prog1, (char *)NULL);
            perror("Erreur lors de l'exécution du programme Liste");
            exit(EXIT_FAILURE);
        } else if (p1 < 0) {
            perror("Erreur lors de la création du processus enfant pour Liste");
            exit(EXIT_FAILURE);
        }
        waitpid(p1, NULL, 0);

        printf("\nVECTEUR :  \n\n");

        pid_t p2 = fork();
        if (p2 == 0) {
            execl(prog2, prog2, (char *)NULL);
            perror("Erreur lors de l'exécution du programme Vecteur");
            exit(EXIT_FAILURE);
        } else if (p2 < 0) {
            perror("Erreur lors de la création du processus enfant pour Vecteur");
            exit(EXIT_FAILURE);
        }
        waitpid(p2, NULL, 0);

        printf("\n");
    }

    printf("Les cinq exécutions des deux programmes sont terminées.\n");
    return 0;

}
