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
#define NUM_ARROWS 1
#define NUM_ASSISTS 2
#define INITIAL_STORAGE_COUNT 2

#define RECHARGE_TIME 10
#define HITTING_CHANCE 40
#define MAX_DAMAGE 100
#define DELIVER_ARROWS_INTERVAL 10
#define PRODUCE_ARROW_INTERVAL 10
#define INVADE_TIME 20
#define DELAY_TIME 5

int archers_life[2][NUM_ARCHERS];
int archers_arrows[2][NUM_ARCHERS];
int on_storage_A = 0, on_storage_B = 0;


pthread_t archersA[NUM_ARCHERS], archersB[NUM_ARCHERS];
pthread_t assistA[NUM_ASSISTS], assistB[NUM_ASSISTS];

pthread_mutex_t archer_life_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t archer_arrows_mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t archer_arrows_mutex_B = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t on_storage_A_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t on_storage_B_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t can_enter_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t can_enter_B = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t archer_cond_A = PTHREAD_COND_INITIALIZER;
pthread_cond_t archer_cond_B = PTHREAD_COND_INITIALIZER;
pthread_cond_t assist_cond_A = PTHREAD_COND_INITIALIZER;
pthread_cond_t assist_cond_B = PTHREAD_COND_INITIALIZER;

sem_t storage_A, storage_B;
sem_t idle_on_storage_A, idle_on_storage_B;



void produceArrows(int team, int id){
  int aux;

  sleep(PRODUCE_ARROW_INTERVAL);
  if(team == TEAM_A){
    printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] adicionou um conjunto de flechas no armazém..." ANSI_COLOR_RESET "\n", id);
    sem_post(&storage_A);
    sem_getvalue(&storage_A, &aux);
    printf("Semáforo A: %d\n", aux);
  }

  else {
    printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] adicionou um conjunto de flechas no armazém..." ANSI_COLOR_RESET "\n", id);
    sem_post(&storage_B);
    sem_getvalue(&storage_B, &aux);
    printf("Semáforo B: %d\n", aux);
  }
}


void searchAndHelpArcher(int team, int id){
  int aux;

  sleep(DELIVER_ARROWS_INTERVAL);

  if(team == TEAM_A){

    if(sem_trywait(&storage_A) == 0){

      for(int i = 0; i < NUM_ARCHERS; i++ ) {
        pthread_mutex_lock(&archer_arrows_mutex_A);
          if(archers_arrows[team][i] <= 0){
            printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] entregou %d flechas para [ARQUEIRO %d]..." ANSI_COLOR_RESET "\n", id, NUM_ARROWS, i );
            archers_arrows[team][i] += NUM_ARROWS;
            pthread_cond_broadcast(&archer_cond_A);
            pthread_mutex_unlock(&archer_arrows_mutex_A);
            return;
          }
        pthread_mutex_unlock(&archer_arrows_mutex_A);
      }
    }

    else {
      printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] tentou auxiliar ARQUEIRO porém não haviam flechas... Irá fabricar flechas." ANSI_COLOR_RESET "\n", id);
      produceArrows(TEAM_A, id);
      searchAndHelpArcher(TEAM_A, id);
    }
  }

  else{

    if(sem_trywait(&storage_B) == 0){
      for(int i = 0; i < NUM_ARCHERS; i++ ) {
        pthread_mutex_lock(&archer_arrows_mutex_B);
          if(archers_arrows[team][i] <= 0){
            printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] entregou %d flechas para [ARQUEIRO %d]..." ANSI_COLOR_RESET "\n", id, NUM_ARROWS, i );
            archers_arrows[team][i] += NUM_ARROWS;
            pthread_cond_broadcast(&archer_cond_B);
            pthread_mutex_unlock(&archer_arrows_mutex_B);
            return;
          }
        pthread_mutex_unlock(&archer_arrows_mutex_B);
      }
    }

    else {
      printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] tentou auxiliar ARQUEIRO porém não haviam flechas... Irá fabricar flechas." ANSI_COLOR_RESET "\n", id);
      produceArrows(TEAM_B, id);
      searchAndHelpArcher(TEAM_B, id);
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
    sleep(DELAY_TIME);
    action = rand() % 3;


    if(action == 1){
        printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] irá produzir flechas..." ANSI_COLOR_RESET "\n", id );
        produceArrows(team, id);
    }

    else if(action == 0){

      pthread_mutex_lock(&on_storage_A_mutex);
        on_storage_A += 1;
        if(on_storage_A == 1 ){ pthread_mutex_lock(&can_enter_A); }
      pthread_mutex_unlock(&on_storage_A_mutex);

      pthread_mutex_lock(&archer_arrows_mutex_A);
        printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] entrou em Idle..." ANSI_COLOR_RESET "\n", id );
        pthread_cond_broadcast(&archer_cond_A);
        pthread_cond_wait(&assist_cond_A, &archer_arrows_mutex_A);
      pthread_mutex_unlock(&archer_arrows_mutex_A);

      pthread_mutex_lock(&on_storage_A_mutex);
        on_storage_A -= 1;
        if(on_storage_A == 0) {pthread_mutex_unlock(&can_enter_A);}
      pthread_mutex_unlock(&on_storage_A_mutex);

      printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] irá auxiliar um arqueiro..." ANSI_COLOR_RESET "\n", id );
      searchAndHelpArcher(team, id);

    }

    else if(action == 2){
      printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] irá invadir o armazém adversário..." ANSI_COLOR_RESET "\n", id );
      sleep(INVADE_TIME);

      pthread_mutex_lock(&can_enter_B);
        if(sem_trywait(&storage_B) == 0){
          printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] invadiu o armazém adversário e retirou um conjunto de flechas..." ANSI_COLOR_RESET "\n", id );
        }

        else{
            printf(ANSI_COLOR_BLUE "\n[ASSISTENTE %d] invadiu o armazém adversário porém não havia nada para retirar..." ANSI_COLOR_RESET "\n", id );
        }
      pthread_mutex_unlock(&can_enter_B);

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
    sleep(DELAY_TIME);

    action = rand() % 3;


    if(action == 1){
      printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] irá produzir flechas..." ANSI_COLOR_RESET "\n", id );
      produceArrows(team, id);
    }

    else if(action == 0){

      pthread_mutex_lock(&on_storage_B_mutex);
        on_storage_B += 1;
        if(on_storage_B == 1 ){ pthread_mutex_lock(&can_enter_B); }
      pthread_mutex_unlock(&on_storage_B_mutex);

      pthread_mutex_lock(&archer_arrows_mutex_B);
        printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] entrou em Idle..." ANSI_COLOR_RESET "\n", id );
        pthread_cond_broadcast(&archer_cond_B);
        pthread_cond_wait(&assist_cond_B, &archer_arrows_mutex_B);
      pthread_mutex_unlock(&archer_arrows_mutex_B);

      pthread_mutex_lock(&on_storage_B_mutex);
        on_storage_B -= 1;
        if(on_storage_B == 0 ) { pthread_mutex_unlock(&can_enter_B); }
      pthread_mutex_unlock(&on_storage_B_mutex);

      printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] irá auxiliar um arqueiro..." ANSI_COLOR_RESET "\n", id );
      searchAndHelpArcher(team, id);
    }

    else if(action == 2){
      printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] irá invadir o armazém adversário..." ANSI_COLOR_RESET "\n", id );
      sleep(INVADE_TIME);

      pthread_mutex_lock(&can_enter_A);
          if(sem_trywait(&storage_A) == 0){
            printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] invadiu o armazém adversário e retirou um conjunto de flechas..." ANSI_COLOR_RESET "\n", id );
          }

          else{
              printf(ANSI_COLOR_GREEN "\n[ASSISTENTE %d] invadiu o armazém adversário porém não havia nada para retirar..." ANSI_COLOR_RESET "\n", id );
          }
      pthread_mutex_unlock(&can_enter_A);
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
    printf(ANSI_COLOR_BLUE "[ARQUEIRO %d, Vida: %d] atirou em um inimigo..."ANSI_COLOR_RESET " \n", id, archers_life[team][id]);
  }
  else{
    printf(ANSI_COLOR_GREEN "[ARQUEIRO %d, Vida: %d] atirou em um inimigo...." ANSI_COLOR_RESET " \n", id, archers_life[team][id]);
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
      printf(ANSI_COLOR_BLUE "[ARQUEIRO %d, Vida: %d] errou." ANSI_COLOR_RESET "\n", id, archers_life[team][id] );
    else
      printf(ANSI_COLOR_GREEN "[ARQUEIRO %d, Vida: %d] errou." ANSI_COLOR_RESET "\n", id, archers_life[team][id] );
  }

}

void* archerFunctionA(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];

  while(1){

    pthread_mutex_lock(&archer_arrows_mutex_A);
      while(archers_arrows[team][id] <= 0){
        printf(ANSI_COLOR_BLUE "[ARQUEIRO %d, Vida: %d] está sem flechas. Esperando recarga..." ANSI_COLOR_RESET "\n", id, archers_life[team][id] );
        pthread_cond_signal(&assist_cond_A);
        pthread_cond_wait(&archer_cond_A, &archer_arrows_mutex_A);
      }
    pthread_mutex_unlock(&archer_arrows_mutex_A);

    shoot(team, id);

    pthread_mutex_lock(&archer_arrows_mutex_A);
      archers_arrows[team][id] -= 1;
    pthread_mutex_unlock(&archer_arrows_mutex_A);

    sleep(RECHARGE_TIME);

  }
}

void* archerFunctionB(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];

  while(1){

    pthread_mutex_lock(&archer_arrows_mutex_B);
      while(archers_arrows[team][id] <= 0){
        printf(ANSI_COLOR_GREEN "[ARQUEIRO %d, Vida: %d] está sem flechas. Esperando recarga..." ANSI_COLOR_RESET "\n", id, archers_life[team][id] );
        pthread_cond_signal(&assist_cond_B);
        pthread_cond_wait(&archer_cond_B, &archer_arrows_mutex_B);
      }
    pthread_mutex_unlock(&archer_arrows_mutex_B);

    shoot(team, id);

    pthread_mutex_lock(&archer_arrows_mutex_B);
      archers_arrows[team][id] -= 1;
    pthread_mutex_unlock(&archer_arrows_mutex_B);

    sleep(RECHARGE_TIME);

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
  if ( pthread_mutex_init( &on_storage_A_mutex, NULL) != 0 )
    printf( "mutex init failed\n" );
    if ( pthread_mutex_init( &can_enter_A, NULL) != 0 )
    printf( "mutex init failed\n" );


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
