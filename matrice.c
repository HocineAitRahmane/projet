#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


// Taille maximal des matrices
#define MAX_SIZE 10 
// Nombre maximal de threads
#define MAX_THREADS 10 


int B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE], A[MAX_SIZE][MAX_SIZE];
// Dimensions des matrices B et C
int n1, m1, n2, m2; 


// Tampon pour les resultats intermediaires
int *T; 
// Index de tampon
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

// fonction pour calculer le produit de deux ligne de matrices
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
        // plasse le resultat dans la matrice A
        int row = elementsConsumed / m2;
        int col = elementsConsumed % m2;
        A[row][col] = result;
        elementsConsumed++;
    }
    return NULL;
}

int main() {

    // inisialisation des valeurs et des matrices
    n1 = 3; m1 = 3; n2 = 3; m2 = 3; 
    fillMatrix(B, n1, m1);
    fillMatrix(C, n2, m2);

     
	// alloer de l'espace pour le tampon
	T = (int *)malloc(n1 * m2 * sizeof(int));


    pthread_t producerThreads[MAX_THREADS], consumerThread;
    int producerArgs[MAX_THREADS];

    // inisialisation des semaphores et du mutex
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


    // affichage de la matrice resultante A
    printf("Matrice A:\n");
    for (int i = 0; i < n1; i++) {
        for (int j = 0; j < m2; j++) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }


    // destruction des semaphores et les mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    // liberation de l'espace alloui pour le tampon
    free(T);

    return 0;
}
