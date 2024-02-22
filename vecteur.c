#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <sys/time.h>

typedef unsigned short Shu;

typedef struct {
    int destination;
    Shu distance;
} Vecteur;

typedef struct {
    int ville;
    bool vu;
    int vecteurCount;
    Vecteur *vecteurs;
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
    graph[source].vecteurCount++;
    graph[source].vecteurs = (Vecteur *)realloc(graph[source].vecteurs, graph[source].vecteurCount * sizeof(Vecteur));
    graph[source].vecteurs[graph[source].vecteurCount - 1].destination = destination;
    graph[source].vecteurs[graph[source].vecteurCount - 1].distance = distance;

    // Ajoute arête de destination à source pour un graphe non orienté
    graph[destination].vecteurCount++;
    graph[destination].vecteurs = (Vecteur *)realloc(graph[destination].vecteurs, graph[destination].vecteurCount * sizeof(Vecteur));
    graph[destination].vecteurs[graph[destination].vecteurCount - 1].destination = source;
    graph[destination].vecteurs[graph[destination].vecteurCount - 1].distance = distance;
}

Node *creerGraphe(int nbs) {
    Node *graph = (Node *)allouerMemoire(nbs * sizeof(Node));
    
    srand(45);
    float taux = 10.0 / 100.0;

    for (int i = 0; i < nbs; i++) {
        graph[i].ville = i;
        graph[i].vu = false;
        graph[i].vecteurCount = 0;
        graph[i].vecteurs = NULL;
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
        for (int j = 0; j < graph[i].vecteurCount; j++) {
            printf("  Destination: %d, Distance: %hu\n", graph[i].vecteurs[j].destination, graph[i].vecteurs[j].distance);
        }
    }
}

void libererGraphe(Node *graph, int nbs) {
    for (int i = 0; i < nbs; i++) {
        free(graph[i].vecteurs);
    }
    free(graph);
}

void DFS(Node *graph, int ville, bool *visited) {
    visited[ville] = true;
    for (int i = 0; i < graph[ville].vecteurCount; i++) {
        int destination = graph[ville].vecteurs[i].destination;
        if (!visited[destination]) {
            DFS(graph, destination, visited);
        }
    }
}

void trouverComposantesConnexes(Node *graph, int nbs, bool *visited, int **composantes, int *tailleComposante) {
    *composantes = (int *)malloc(nbs * sizeof(int));
    *tailleComposante = 0;
    for (int i = 0; i < nbs; i++) {
        if (!visited[i] && graph[i].vecteurCount > 0) {
            DFS(graph, i, visited);
            (*composantes)[(*tailleComposante)++] = i;
        }
    }
}

void heuristique_ppv(Node *graph, int villeDepart, int nbs, int *itineraire, bool *visited) {
    bool *localVisited = (bool *)calloc(nbs, sizeof(bool));
    int villeActuelle = villeDepart;
    int count = 0;

    itineraire[count++] = villeDepart;
    localVisited[villeDepart] = true;

    while (count < nbs) {
        int proche = -1;
        int minDist = INT_MAX;
        for (int i = 0; i < graph[villeActuelle].vecteurCount; i++) {
            int destination = graph[villeActuelle].vecteurs[i].destination;
            Shu distance = graph[villeActuelle].vecteurs[i].distance;
            if (!localVisited[destination] && distance < minDist) {
                minDist = distance;
                proche = destination;
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

Shu calculerCoutTotal(Node *graph, int *itineraire) {
    Shu totalCost = 0;
    for (int i = 0; itineraire[i] != -1 && itineraire[i + 1] != -1; i++) {
        int villeActuelle = itineraire[i];
        int villeSuivante = itineraire[i + 1];
        bool areteTrouvee = false;
        for (int j = 0; j < graph[villeActuelle].vecteurCount; j++) {
            if (graph[villeActuelle].vecteurs[j].destination == villeSuivante) {
                totalCost += graph[villeActuelle].vecteurs[j].distance;
                areteTrouvee = true;
                break;
            }
        }
        if (!areteTrouvee) {
            printf("Aucune arête trouvée entre %d et %d\n", villeActuelle, villeSuivante);
            return -1; // Itinéraire invalide
        }
    }
    return totalCost;
}

int main() {
    int nbs = 5; // Nombre de sommets dans le graphe
    Node *graph = creerGraphe(nbs);
    afficherGraphe(graph, nbs);

    bool *visited = (bool *)calloc(nbs, sizeof(bool));
    int *composantes;
    int tailleComposante;
    trouverComposantesConnexes(graph, nbs, visited, &composantes, &tailleComposante);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < tailleComposante; i++) {
        int *itineraire = (int *)allouerMemoire(nbs * sizeof(int));
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
    printf("Total mémoire allouée : %zu bytes\n", memoireTotaleAllouee);

    free(composantes);
    libererGraphe(graph, nbs);
    free(visited);

    return 0;
}
