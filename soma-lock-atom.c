#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define INCR_PER_THREAD 100000

long int contador = 0;                 
pthread_mutex_t trava;                
pthread_cond_t cond_atingiu;     
pthread_cond_t cond_impresso;     

int precisa_imprimir = 0;              
long int valor_para_imprimir = 0;            

int terminou = 0;                   

void *ExecutaTarefa (void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);

    for (int i = 0; i < INCR_PER_THREAD; i++) {
        pthread_mutex_lock(&trava);

        while (precisa_imprimir) {
            pthread_cond_wait(&cond_impresso, &trava);
        }

        contador++;

        if ((contador % 1000) == 0 && !precisa_imprimir) {
            precisa_imprimir = 1;
            valor_para_imprimir = contador;
           
            pthread_cond_signal(&cond_atingiu);

            while (precisa_imprimir) {
                pthread_cond_wait(&cond_impresso, &trava);
            }
        }

        pthread_mutex_unlock(&trava);
    }

    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

void *extra (void *args) {
    printf("Extra : esta executando...\n");

    pthread_mutex_lock(&trava);
    for (;;) {
        while (!precisa_imprimir && !terminou) {
            pthread_cond_wait(&cond_atingiu, &trava);
        }

        if (precisa_imprimir) {
            long int v = valor_para_imprimir;   
            printf("soma = %ld \n", v);
            precisa_imprimir = 0;
            pthread_cond_broadcast(&cond_impresso);
            continue;
        }

        if (terminou && !precisa_imprimir) {
            pthread_mutex_unlock(&trava);
            break;
        }
    }

    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t *threads;          
    int qtd_threads;          
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    qtd_threads = atoi(argv[1]);
    if (qtd_threads <= 0) {
        puts("Numero de threads deve ser positivo.");
        return 1;
    }

    threads = (pthread_t*) malloc(sizeof(pthread_t) * (qtd_threads + 1));
    if (threads == NULL) { puts("ERRO--malloc"); return 2; }

    pthread_mutex_init(&trava, NULL);
    pthread_cond_init(&cond_atingiu, NULL);
    pthread_cond_init(&cond_impresso, NULL);

    for (long int t = 0; t < qtd_threads; t++) {
        if (pthread_create(&threads[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }

    if (pthread_create(&threads[qtd_threads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
    }

    for (int t = 0; t < qtd_threads; t++) {
        if (pthread_join(threads[t], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1);
        }
    }

    pthread_mutex_lock(&trava);
    terminou = 1;
    pthread_cond_signal(&cond_atingiu);
    pthread_mutex_unlock(&trava);

    if (pthread_join(threads[qtd_threads], NULL)) {
        printf("--ERRO: pthread_join() \n"); exit(-1);
    }

    pthread_cond_destroy(&cond_atingiu);
    pthread_cond_destroy(&cond_impresso);
    pthread_mutex_destroy(&trava);
    free(threads);

    printf("Valor final de 'soma' = %ld\n", contador);
    return 0;
}
