#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// taille maximal des matrices
#define MAX_SIZE 10 
// nombre maximal de threads
#define MAX_THREADS 10 

int B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE], A[MAX_SIZE][MAX_SIZE];
// dimensions des matrices B et C
int n1, m1, n2, m2; 

// tampon pour les resultats 
int *T; 
// index de tampon
int bufferIndex = 0; 

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

// fonction pour produire des valeurs aleatoires dans les matrices B et C
void fillMatrix(int matrix[MAX_SIZE][MAX_SIZE], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() % 10; 
        }
    }
}

// fonction pour calculer le produit de deux lignes de matrices
int calculateRowProduct(int rowB[MAX_SIZE], int colC[MAX_SIZE]) {
    int result = 0;
    for (int i = 0; i < MAX_SIZE; i++) {
        result += rowB[i] * colC[i];
    }
    return result;
}

// producteur
void *producer(void *arg) {
    int row = *(int*)arg;
    for (int i = 0; i < m2; i++) {
        int result = calculateRowProduct(B[row], C[i]);
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        T[bufferIndex] = result;
        bufferIndex = (bufferIndex + 1) % (n1 * m2); 
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
    return NULL;
}

// consommateur
void *consumer(void *arg) {
    int totalElements = n1 * m2;
    int elementsConsumed = 0;
    while (elementsConsumed < totalElements) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int result = T[bufferIndex];
        bufferIndex = (bufferIndex + 1) % (n1 * m2);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        // placer le resultat dans la matrice A
        int row = elementsConsumed / m2;
        int col = elementsConsumed % m2;
        A[row][col] = result;
        elementsConsumed++;
    }
    return NULL;
}

int main() {
    printf("Entrez le nombre de lignes de la matrice A : ");
    scanf("%d", &n1);
    printf("Entrez le nombre de colonnes de la matrice A : ");
    scanf("%d", &m1);
    printf("Entrez le nombre de lignes de la matrice B : ");
    scanf("%d", &n2);
    printf("Entrez le nombre de colonnes de la matrice B : ");
    scanf("%d", &m2);

    printf("\n");

    // verification des dimensions pour la multiplication de matrices
    if (m1 != n2) {
        printf("Erreur : Les dimensions des matrices ne permettent pas la multiplication.\n");
        return -1;
    }

    // informations utilisateur
    printf("Les deux matrices sont remplies avec des valeurs aleatoires.\n");
    printf("La matrice A est le résultat de la multiplication de ces deux matrices.\n");

    // allocation de l'espace pour le tampon
    T = (int *)malloc(n1 * m2 * sizeof(int));

    // remplissage des matrices B et C avec des valeurs aléatoires
    fillMatrix(B, n1, m1);
    fillMatrix(C, n2, m2);

    pthread_t producerThreads[MAX_THREADS], consumerThread;
    int producerArgs[MAX_THREADS];

    // initialisation des semaphores et du mutex
    sem_init(&empty, 0, n1 * m2); 
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // creation des threads producteurs
    for (int i = 0; i < n1; i++) {
        producerArgs[i] = i;
        pthread_create(&producerThreads[i], NULL, producer, &producerArgs[i]);
    }

    // creation du thread consommateur
    pthread_create(&consumerThread, NULL, consumer, NULL);

    // attente des threads producteurs
    for (int i = 0; i < n1; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    // attente du thread consommateur
    pthread_join(consumerThread, NULL);

    // affichage de la matrice résultante A
    printf("Matrice A:\n");
    for (int i = 0; i < n1; i++) {
        for (int j = 0; j < m2; j++) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }

    // destruction des semaphores et du mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    // liberation de l'espace alloue 
    free(T);

    return 0;
}
