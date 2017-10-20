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

#define NUM_ARCHERS 10
#define TEAM_A 0
#define TEAM_B 1
#define RECHARGE_TIME 10
#define HITTING_CHANCE 80
#define MAX_DAMAGE 100

pthread_mutex_t archer_life_mutex = PTHREAD_MUTEX_INITIALIZER;
int archers_life[2][NUM_ARCHERS];
pthread_t archersA[NUM_ARCHERS];
pthread_t archersB[NUM_ARCHERS];

void isArcherAlive(int team, int id){
    if( archers_life[team][id] <= 0){
      printf(ANSI_COLOR_RED "Arqueiro do time %d, id: %d morreu" ANSI_COLOR_RESET "\n", team, id );
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

  if(team == TEAM_A)
    printf(ANSI_COLOR_BLUE "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] está mirando em um inimigo.."ANSI_COLOR_RESET " \n", team, id, archers_life[team][id]);
  else
    printf(ANSI_COLOR_GREEN "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] está mirando em um inimigo..." ANSI_COLOR_RESET " \n", team, id, archers_life[team][id]);

  if (rand() % 100 < HITTING_CHANCE) {

    do{
      enemy_id = rand() % NUM_ARCHERS;
    }while(archers_life[enemy_team][enemy_id] < 0);

    damage = rand() % MAX_DAMAGE;
    pthread_mutex_lock(&archer_life_mutex);
      archers_life[enemy_team][enemy_id] -= damage;

      if(enemy_team == TEAM_A)
        printf(ANSI_COLOR_BLUE "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] sofreu %d de dano." ANSI_COLOR_RESET  "\n", enemy_team, enemy_id, archers_life[enemy_team][enemy_id], damage);
      else
        printf(ANSI_COLOR_GREEN "[ARQUEIRO - Time: %d, Id: %d, Vida: %d] sofreu %d de dano." ANSI_COLOR_RESET  "\n", enemy_team, enemy_id, archers_life[enemy_team][enemy_id], damage);

      isArcherAlive(enemy_team, enemy_id);
    pthread_mutex_unlock(&archer_life_mutex);
  }

  else {
    printf("[ARQUEIRO - Time: %d, Id: %d, Vida: %d] errou.\n", team, id, archers_life[team][id] );
  }

}

void* archerFunction(void* array){
  int *a = ((int *) array);
  int team = a[0];
  int id = a[1];

  while(1){

    if(archers_life[team][id] <= 0){
      printf("Arqueiro do time %d, id: %d morreu.\n", team, id );
      pthread_exit(0);
    }

    sleep(rand() % RECHARGE_TIME);

    shoot(team, id);


  }

}


void initialize(){
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < NUM_ARCHERS; j++){
      archers_life[i][j] = 100;
    }
  }
}

int main(){
  int *array;
  srand(time(NULL));

  initialize();


  for (int i = 0; i < NUM_ARCHERS ; i++) {
    array = (int *) malloc(2 * sizeof(int));
    array[0] = TEAM_A;
    array[1] = i;
    pthread_create(&archersA[i], NULL, archerFunction , (void *) array);
	}

  for (int i = 0; i < NUM_ARCHERS ; i++) {
    array = (int *) malloc(2 * sizeof(int));
    array[0] = TEAM_B;
    array[1] = i;
    pthread_create(&archersB[i], NULL, archerFunction , (void *) array);
  }

  while(1){

  }

}
