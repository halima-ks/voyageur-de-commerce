#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <sys/time.h>


typedef unsigned short Shu;

typedef struct strand {
    int destination;
    Shu distance;
    struct strand *next;
} strand;

typedef struct Node {
    int ville;
    strand *aretes;
} Node;
size_t memoireTotaleAllouee = 0;

void *allouerMemoire(size_t taille) {
    void *memoire = malloc(taille);
    if (!memoire) {
        perror("Erreur d'allocation de mémoire");
        exit(EXIT_FAILURE);
    }
    memoireTotaleAllouee += taille; // Ajoute la taille de la mémoire allouée au compteur global
    return memoire;
}


void ajouterArete(Node *graph, int source, int destination, Shu distance) {
    // Ajoute arête de source à destination
    strand *nouvelleArete = (strand *)allouerMemoire(sizeof(strand));
    nouvelleArete->destination = destination;
    nouvelleArete->distance = distance;
    nouvelleArete->next = graph[source].aretes;
    graph[source].aretes = nouvelleArete;

    // Ajoute arête de destination à source pour un graphe non orienté
    nouvelleArete = (strand *)allouerMemoire(sizeof(strand));
    nouvelleArete->destination = source;
    nouvelleArete->distance = distance;
    nouvelleArete->next = graph[destination].aretes;
    graph[destination].aretes = nouvelleArete;
}


// Node *creerGraphe(int nbs) {
//     srand(57); // Graine fixe pour la reproductibilité
//     Node *graph = (Node *)allouerMemoire(nbs * sizeof(Node));
//     float taux = 10.0 / 100.0; // Taux de connexion

//     printf("taux %g\n", taux);

//     for (int i = 0; i < nbs; i++) {
//         graph[i].ville = i;
//         graph[i].aretes = NULL;
//     }

//     for (int i = 0; i < nbs; i++) {
//         for (int j = i + 1; j < nbs; j++) {
//             if (((float) rand() / RAND_MAX) < taux) {
//                 Shu distance = (Shu)(rand() % 100 + 1);
//                 ajouterArete(graph, i, j, distance);
//             }
//         }
//     }

//     return graph;
// }

Node *creerGraphe(int nbs) {
    Node *graph = (Node *)allouerMemoire(nbs * sizeof(Node));
    
    srand(45);
    float taux = 20.0 /100.0;

    for (int i = 0; i < nbs; i++) {
        graph[i].ville = i;
        graph[i].aretes = NULL;
    }

    for (int i = 0; i < nbs; i++) {
        for (int j = i + 1; j < nbs; j++) {
            float v = (float) rand() / RAND_MAX; // Génére une nouvelle valeur aléatoire v
            if (v < taux) {
                Shu distance = (Shu)(v * 1000) + 1; // Calcule la distance en fonction de v
                ajouterArete(graph, i, j, distance);
            }
        }
    }

    return graph;
}


void afficherGraphe(Node *graph, int nbs) {
    for (int i = 0; i < nbs; i++) {
        printf("Ville %d:\n", graph[i].ville);
        for (strand *arete = graph[i].aretes; arete != NULL; arete = arete->next) {
            printf("  Destination: %d, Distance: %hu\n", arete->destination, arete->distance);
        }
    }
}

void libererGraphe(Node *graph, int nbs) {
    for (int i = 0; i < nbs; i++) {
        strand *arete = graph[i].aretes;
        while (arete) {
            strand *temp = arete;
            arete = arete->next;
            free(temp);
        }
    }
    free(graph);
}

void DFS(Node *graph, int ville, bool *visited) {
    visited[ville] = true;
    for (strand *arete = graph[ville].aretes; arete != NULL; arete = arete->next) {
        if (!visited[arete->destination]) {
            DFS(graph, arete->destination, visited);
        }
    }
}

void trouverComposantesConnexes(Node *graph, int nbs, bool *visited, int **composantes, int *tailleComposante) {
    *composantes = allouerMemoire(nbs * sizeof(int));
    *tailleComposante = 0;
    for (int i = 0; i < nbs; i++) {
        if (!visited[i] && graph[i].aretes != NULL) { // Vérifie si la ville a des connexions
            DFS(graph, i, visited);
            (*composantes)[(*tailleComposante)++] = i;
        }
    }
}


void heuristique_ppv(Node *graph, int villeDepart, int nbs, int *itineraire, bool *visited) {
    bool *localVisited = calloc(nbs, sizeof(bool));
    int villeActuelle = villeDepart;
    int count = 0;

    itineraire[count++] = villeDepart;
    localVisited[villeDepart] = true;

    while (count < nbs) {
        int proche = -1;
        int minDist = INT_MAX;
        for (strand *arete = graph[villeActuelle].aretes; arete != NULL; arete = arete->next) {
            if (!localVisited[arete->destination] && arete->distance < minDist) {
                minDist = arete->distance;
                proche = arete->destination;
            }
        }

        if (proche == -1) {
            break; // Plus de voisins ou impasse
        } else {
            localVisited[proche] = true;
            villeActuelle = proche;
            itineraire[count++] = proche;
        }
    }

    // Remplir le reste de l'itinéraire avec -1 pour indiquer la fin
    for (int i = count; i < nbs; i++) {
        itineraire[i] = -1;
    }

    free(localVisited);
}
////////////////////////////////
// pour le circuit retour à la ville de départ utiliser en plus dijkstra ?? our trouver un chemin de retour ou ajouter directement la ville de départ si elle est connectée
// changer carrement d'heuristique ?
///////////////////////////////

Shu calculerCoutTotal(Node *graph, int *itineraire) {
    Shu totalCost = 0;
    for (int i = 0; itineraire[i] != -1 && itineraire[i + 1] != -1; i++) {
        bool areteTrouvee = false;
        for (strand *arete = graph[itineraire[i]].aretes; arete != NULL; arete = arete->next) {
            if (arete->destination == itineraire[i + 1]) {
                totalCost += arete->distance;
                areteTrouvee = true;
                break;
            }
        }
        if (!areteTrouvee) {
            printf("Aucune arête trouvée entre %d et %d\n", itineraire[i], itineraire[i + 1]);
            return -1; // Itinéraire invalide
        }
    }
    return totalCost;
}



int main() {
    int nbs = 5; // Nombre de sommets dans le graphe
    Node *graph = creerGraphe(nbs);
    afficherGraphe(graph, nbs);

    bool *visited = calloc(nbs, sizeof(bool));
    int *composantes;
    int tailleComposante;
    trouverComposantesConnexes(graph, nbs, visited, &composantes, &tailleComposante);

    // for (int i = 0; i < tailleComposante; i++) {
    //     int *itineraire = malloc(nbs * sizeof(int));
    //     printf("Composante Connexe %d, partant de la ville %d:\n", i + 1, composantes[i]);
    //     heuristique_ppv(graph, composantes[i], nbs, itineraire, visited);

    //     for (int j = 0; itineraire[j] != -1 && j < nbs; j++) {
    //         printf("%d ", itineraire[j]);
    //     }
    //     printf("\n");
    //     free(itineraire);
    // }
    struct timeval start, end;
    gettimeofday(&start, NULL);

     for (int i = 0; i < tailleComposante; i++) {
        int *itineraire = allouerMemoire(nbs * sizeof(int));
        printf("Composante Connexe %d, partant de la ville %d:\n", i + 1, composantes[i]);
        heuristique_ppv(graph, composantes[i], nbs, itineraire, visited);
        

        printf("Itinéraire trouvé : ");
        for (int j = 0; itineraire[j] != -1 && j < nbs; j++) {
            printf("%d ", itineraire[j]);
        }
        printf("\n");

        Shu coutTotal = calculerCoutTotal(graph, itineraire);
        if (coutTotal != (Shu)-1) {
            printf("Coût total de l'itinéraire : %hu\n", coutTotal);
        } else {
            printf("Itinéraire invalide.\n");
        }

        free(itineraire);
    }
    gettimeofday(&end, NULL);
    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

    printf("Temps d'exécution : %ld secondes et %ld microsecondes\n", seconds, micros);
    // Affiche la mémoire totale allouée à la fin du programme
    printf("Total memoire alloué : %zu bytes\n", memoireTotaleAllouee);

    free(composantes);
    libererGraphe(graph, nbs);
    free(visited);



    return 0;
}

