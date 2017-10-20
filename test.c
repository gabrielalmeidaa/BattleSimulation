#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int want_input_flag = 0;
int input = 0;

void* testFunction(void* arg){
	int id = *((int *) arg);

  while(1){
    pthread_mutex_lock(&mutex);
      if(want_input_flag){
        pthread_cond_wait(&cond, &mutex);
      }
    pthread_mutex_unlock(&mutex);

    printf("Processo %d está acordado.\n", id);
    sleep(rand() % 2 );
  }
}

void* inputFunction(void* arg){
  int id = *((int *) arg);

  while(1){
    sleep(rand() % 10);

    printf("Quero pedir um input.\n");

    pthread_mutex_lock(&mutex);
      want_input_flag = 1;
      printf("Digite seu input: ");
      scanf("%d\n", &input);
      want_input_flag = 0;
      pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

  }

}

int main(){
  int *id;
  srand(time(NULL));
  pthread_t threads[2];
  pthread_t ithreads[1];

  for (int i = 0; i < 2 ; i++) {
    id = (int *) malloc(sizeof(int));
    *id = i;
    pthread_create(&threads[i], NULL, testFunction , (void *) id);
  }

  for (int i = 0; i < 1; i++){
    id = (int *) malloc(sizeof(int));
    *id = i;
    pthread_create(&ithreads[i], NULL, inputFunction , (void *) id);
  }

  pthread_join(threads[0],NULL);
  return 0;
}
