#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define TEAM_A 0
#define TEAM_B 1
#define NUM_ARCHERS 1
#define NUM_ARROWS 2
#define NUM_ASSISTS 1
#define INITIAL_STORAGE_COUNT 0

#define RECHARGE_TIME 10
#define HITTING_CHANCE 40
#define MAX_DAMAGE 100
#define DELIVER_ARROWS_INTERVAL 10
#define PRODUCE_ARROW_INTERVAL 10

int archers_life[2][NUM_ARCHERS];
int archers_arrows[2][NUM_ARCHERS];


pthread_t archersA[NUM_ARCHERS], archersB[NUM_ARCHERS];
pthread_t assistA[NUM_ASSISTS], assistB[NUM_ASSISTS];

pthread_mutex_t archer_life_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t archer_arrows_mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t archer_arrows_mutex_B = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t archer_cond_A = PTHREAD_COND_INITIALIZER;
pthread_cond_t archer_cond_B = PTHREAD_COND_INITIALIZER;
pthread_cond_t assist_cond_A = PTHREAD_COND_INITIALIZER;
pthread_cond_t assist_cond_B = PTHREAD_COND_INITIALIZER;

sem_t storage_A, storage_B;
sem_t idle_on_storage_A, idle_on_storage_B;





void produceArrows(int team, int id){
  int aux;

  sleep(rand() % PRODUCE_ARROW_INTERVAL);
  if(team == TEAM_A){
    printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] adicionou um conjunto de flechas no armazém..." ANSI_COLOR_RESET "\n", id);
    sem_post(&storage_A);
    sem_getvalue(&storage_A, &aux);
    printf("Semáforo A: %d\n", aux);
  }

  else {
    printf(ANSI_COLOR_GREEN "[ASSISTENTE %d] adicionou um conjunto de flechas no armazém..." ANSI_COLOR_RESET "\n", id);
    sem_post(&storage_B);
    sem_getvalue(&storage_B, &aux);
    printf("Semáforo B: %d\n", aux);
  }
}


void searchAndHelpArcher(int team, int id){
  int aux;

  sleep(rand() % DELIVER_ARROWS_INTERVAL);

  if(team == TEAM_A){
    sem_wait(&storage_A);
      sem_getvalue(&storage_A, &aux);
      printf("Semáforo A: %d\n", aux);
      for(int i = 0; i < NUM_ARCHERS; i++ ) {
        pthread_mutex_lock(&archer_arrows_mutex_A);
          if(archers_arrows[team][i] <= 0){
            printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] irá entregar %d flechas para Arqueiro %d..." ANSI_COLOR_RESET "\n", id, NUM_ARROWS, i );
            archers_arrows[team][i] += NUM_ARROWS;
            pthread_cond_broadcast(&archer_cond_A);
            pthread_mutex_unlock(&archer_arrows_mutex_A);
            return;
          }
        pthread_mutex_unlock(&archer_arrows_mutex_A);
      }

  }

  else{
    sem_wait(&storage_B);
      sem_getvalue(&storage_B, &aux);
      printf("Semáforo B: %d\n", aux);
      for(int i = 0; i < NUM_ARCHERS; i++ ) {
        pthread_mutex_lock(&archer_arrows_mutex_B);
          if(archers_arrows[team][i] <= 0){
            printf(ANSI_COLOR_GREEN "[ASSISTENTE %d] irá entregar %d flechas para Arqueiro %d..." ANSI_COLOR_RESET "\n", id, NUM_ARROWS, i );
            archers_arrows[team][i] += NUM_ARROWS;
            pthread_cond_broadcast(&archer_cond_B);
            pthread_mutex_unlock(&archer_arrows_mutex_B);
            return;
          }
        pthread_mutex_unlock(&archer_arrows_mutex_B);
      }

  }

}


void* assistFunctionA(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];
  int sem_val = 0;
  int action = 0;

  while(1){
    printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] irá realizar nova ação!" ANSI_COLOR_RESET "\n", id );
    action = rand() % 3;


    if(action == 0){
        printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] irá produzir flechas..." ANSI_COLOR_RESET "\n", id );
        produceArrows(team, id);
    }

    else if(action == 1){
      printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] entrou em Idle..." ANSI_COLOR_RESET "\n", id );
      sem_post(&idle_on_storage_A);

        pthread_mutex_lock(&archer_arrows_mutex_A);
          pthread_cond_wait(&assist_cond_A, &archer_arrows_mutex_A);
        pthread_mutex_unlock(&archer_arrows_mutex_A);

      sem_wait(&idle_on_storage_A);

      sem_getvalue(&storage_A, &sem_val);

      while(sem_val == 0){
        printf("[ASSISTENTE %d] acordou: valor do semáforo: %d \n",id, sem_val);
        printf("[ASSISTENTE %d] irá produzir flechas...\n", id);
        produceArrows(team, id);
        sem_getvalue(&storage_A, &sem_val);
      }

      printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] irá auxiliar um arqueiro..." ANSI_COLOR_RESET "\n", id );
      searchAndHelpArcher(team, id);

    }

    else if(action == 2){
      printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] irá invadir o armazém adversário..." ANSI_COLOR_RESET "\n", id );

      if (sem_trywait(&idle_on_storage_B) != 0) {
        if (sem_trywait(&storage_B) == 0) { // TEM PERMISSÂO
          sem_wait(&storage_B);
          printf(ANSI_COLOR_BLUE "[ASSISTENTE %d] retirou um conjunto de flechas do armazém inimigo..." ANSI_COLOR_RESET "\n", id );
        }
      }

    }

  }
}


void* assistFunctionB(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];
  int sem_val = 0;
  int action = 0;

  while(1){
    printf(ANSI_COLOR_GREEN "[ASSISTENTE %d] irá realizar nova ação!" ANSI_COLOR_RESET "\n", id );
    action = rand() %2;


    if(action == 0){
      printf(ANSI_COLOR_GREEN "[ASSISTENTE %d] irá produzir flechas..." ANSI_COLOR_RESET "\n", id );
      produceArrows(team, id);
    }

    else if(action == 1){
      printf(ANSI_COLOR_GREEN "[ASSISTENTE %d] entrou em Idle..." ANSI_COLOR_RESET "\n", id );
      pthread_mutex_lock(&archer_arrows_mutex_B);
        pthread_cond_wait(&assist_cond_B, &archer_arrows_mutex_B);
      pthread_mutex_unlock(&archer_arrows_mutex_B);

      sem_getvalue(&storage_B, &sem_val);

      while(sem_val == 0){
        printf("Assistente acordou: valor do semáforo: %d \n", sem_val);
        printf("assistente vai produzir flechas...\n");
        produceArrows(team,id);
        sem_getvalue(&storage_B, &sem_val);
      }

      printf(ANSI_COLOR_GREEN "[ASSISTENTE %d] irá auxiliar um arqueiro..." ANSI_COLOR_RESET "\n", id );
      searchAndHelpArcher(team, id);
    }
  }

}


void isArcherAlive(int team, int id){
    if( archers_life[team][id] <= 0){
      printf(ANSI_COLOR_RED "\n\nArqueiro do time %d, id: %d morreu" ANSI_COLOR_RESET "\n\n", team, id );
      if(team == TEAM_A){
        pthread_cancel(archersA[id]);
      }
      else {
        pthread_cancel(archersB[id]);
      }
    }
}

void shoot(int team, int id){
  int enemy_team,enemy_id, damage;

  if(team == TEAM_A) enemy_team = TEAM_B; else enemy_team = TEAM_A;

  if(team == TEAM_A){
    printf(ANSI_COLOR_BLUE "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] atirou em um inimigo..."ANSI_COLOR_RESET " \n", team, id, archers_life[team][id]);
  }
  else{
    printf(ANSI_COLOR_GREEN "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] atirou em um inimigo...." ANSI_COLOR_RESET " \n", team, id, archers_life[team][id]);
  }

  if (rand() % 100 < HITTING_CHANCE) {

    do{
      enemy_id = rand() % NUM_ARCHERS;
    }while(archers_life[enemy_team][enemy_id] < 0);

    damage = rand() % MAX_DAMAGE;

    pthread_mutex_lock(&archer_life_mutex);
      archers_life[enemy_team][enemy_id] -= damage;
      printf(ANSI_COLOR_RED "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] sofreu %d de dano." ANSI_COLOR_RESET  "\n", enemy_team, enemy_id, archers_life[enemy_team][enemy_id], damage);
      isArcherAlive(enemy_team, enemy_id);
    pthread_mutex_unlock(&archer_life_mutex);
  }

  else {
    if (team == TEAM_A)
      printf(ANSI_COLOR_BLUE "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] errou." ANSI_COLOR_RESET "\n", team, id, archers_life[team][id] );
    else
      printf(ANSI_COLOR_GREEN "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] errou." ANSI_COLOR_RESET "\n", team, id, archers_life[team][id] );
  }

}

void* archerFunctionA(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];

  while(1){

    pthread_mutex_lock(&archer_arrows_mutex_A);
      while(archers_arrows[team][id] <= 0){
        printf(ANSI_COLOR_BLUE "[ARQUEIRO - Id: %d, Vida: %d] está sem flechas. Esperando recarga..." ANSI_COLOR_RESET "\n", id, archers_life[team][id] );
        pthread_cond_signal(&assist_cond_A);
        pthread_cond_wait(&archer_cond_A, &archer_arrows_mutex_A);
      }
    pthread_mutex_unlock(&archer_arrows_mutex_A);

    shoot(team, id);

    pthread_mutex_lock(&archer_arrows_mutex_A);
      archers_arrows[team][id] -= 1;
    pthread_mutex_unlock(&archer_arrows_mutex_A);

    sleep(rand() % RECHARGE_TIME);

  }
}

void* archerFunctionB(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];

  while(1){

    pthread_mutex_lock(&archer_arrows_mutex_B);
      while(archers_arrows[team][id] <= 0){
        printf(ANSI_COLOR_GREEN "[ARQUEIRO - Id: %d, Vida: %d] está sem flechas. Esperando recarga..." ANSI_COLOR_RESET "\n", id, archers_life[team][id] );
        pthread_cond_signal(&assist_cond_B);
        pthread_cond_wait(&archer_cond_B, &archer_arrows_mutex_B);
      }
    pthread_mutex_unlock(&archer_arrows_mutex_B);

    shoot(team, id);

    pthread_mutex_lock(&archer_arrows_mutex_B);
      archers_arrows[team][id] -= 1;
    pthread_mutex_unlock(&archer_arrows_mutex_B);

    sleep(rand() % RECHARGE_TIME);

  }
}


void initialize(){
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < NUM_ARCHERS; j++){
      archers_life[i][j] = 100;
      archers_arrows[i][j] = NUM_ARROWS;
    }
  }

  sem_init(&storage_A, 0, INITIAL_STORAGE_COUNT);
  sem_init(&storage_B, 0, INITIAL_STORAGE_COUNT);
  sem_init(&idle_on_storage_A, 0, 0);
  sem_init(&idle_on_storage_B, 0, 0);


}

int main(){
  int *array;
  srand(time(NULL));

  initialize();

  for (int i = 0; i < NUM_ASSISTS ; i++) {
    array = (int *) malloc(2 * sizeof(int));
    array[0] = TEAM_A;
    array[1] = i;
    pthread_create(&assistA[i], NULL, assistFunctionA , (void *) array);
  }

  for (int i = 0; i < NUM_ASSISTS ; i++) {
    array = (int *) malloc(2 * sizeof(int));
    array[0] = TEAM_B;
    array[1] = i;
    pthread_create(&assistB[i], NULL, assistFunctionB , (void *) array);
  }

  for (int i = 0; i < NUM_ARCHERS ; i++) {
    array = (int *) malloc(2 * sizeof(int));
    array[0] = TEAM_A;
    array[1] = i;
    pthread_create(&archersA[i], NULL, archerFunctionA , (void *) array);
	}

  for (int i = 0; i < NUM_ARCHERS ; i++) {
    array = (int *) malloc(2 * sizeof(int));
    array[0] = TEAM_B;
    array[1] = i;
    pthread_create(&archersB[i], NULL, archerFunctionB , (void *) array);
  }

  while(1){

  }

}
