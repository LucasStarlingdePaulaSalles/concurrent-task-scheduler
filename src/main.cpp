#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <oven-queue.hpp>

/* Sheldon, Leonard e Howard compraram um micro-ondas juntos, que compartilham com 
 * Stuart e Kripke. Para decidir quem podia usar o forno, definiram o seguinte 
 * conjunto de regras
 *
 * - se o micro-ondas estiver liberado, quem chegar primeiro pode usar;
 * - caso contrário, obviamente, quem chegar depois tem que esperar;
 * - se mais de uma pessoa estiver esperando para usar, valem as precedências:
 *
 *   - Sheldon pode usar antes do Howard;
 *   - Howard pode usar antes do Leonard;
 *   - Leonard pode usar antes do Sheldon;
 *   - Stuart pode usar depois dos três;
 *   - Kripke é sempre o último.
 *
 * - Quando alguém termina de usar o forno, deve liberá-lo para a próxima pessoa 
 *   de maior prioridade, exceto em dois casos:
 *
 *   - Para evitar inanição (discutido a seguir);
 *   - Quando houver deadlock (um ciclo de espera).
 *
 * -  Quando a namorada de um deles chega, aquele casal tem a preferencia do 
 *    micro-ondas:
 *
 *    - Leonard namora Penny
 *    - Howard namora Bernardette
 *    - Sheldon namora Amy
 *
 * - Raj periodicamente (a cada 5 segundos) confere a situação e se encontrar deadlock
 *   pode escolher um personagem aleatoriamente e liberá-lo para usar o forno
 *
 *
 * - Exemplos:
 *   Sheldon está usando o forno
 *   Leonard quer usar o forno
 *   Howard quer usar o forno
 *   (Howard tem preferência)
 *   Penny chega
 *   (Leonard+Penny tem preferência)
 *   Bernardette chega
 *   (Howard_bernardette tem preferência sobre Leonard+Penny)
 *
 */
#define HOWARD 2
#define BERNARDETTE 3
#define LEONARD 4
#define PENNY 5
#define STUART 6
#define KRIPKE 7
#define RAJ 8

/* Global variables */
int num_executions = 1;
int thread_count = 9;
int turn = -1;
int done = 0;

pthread_mutex_t lock;
pthread_mutex_t lock_queue;
OvenQueue queue;
pthread_cond_t can_use[RAJ];
bool in_use = false;

void *enter_queue(void* rank);
void *use_oven(void* rank);
void *check(void* rank);
void *await_turn(void* rank);
void *leave_oven(void* rank);

int main(int argc, char* argv[]) {
  long thread;

  if (argc == 2) {
    num_executions = strtol(argv[1], NULL, 10);
  }

  pthread_mutex_init(&lock, NULL); // Declara o lock para o forno
  pthread_mutex_init(&lock_queue, NULL); // Declara o lock para a fila

  // Declara as threads correspondentes a cada personagem (e.g., thread_handles[0] = Sheldon)
  pthread_t thread_handles[thread_count];

  for (thread = thread_count - 2; thread >= 0; thread--)
    pthread_create(&thread_handles[thread], NULL, enter_queue, (void*) thread);
  
  // Cria thread correspondente ao Raj
  pthread_create(&thread_handles[thread_count - 1], NULL, check, (void*) thread);
  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);

  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock_queue);
  return 0;
}


void *enter_queue(void* rank) {
  long my_rank = (long) rank;

  for (int i = 0; i < num_executions; i++) { 
    pthread_mutex_lock(&lock_queue);
    // Personagem declara que quer utilizar o forno!
    std::cout << Charecter::char_name(my_rank) << " quer usar o forno" << std::endl;
    queue.push(my_rank);
    done++;
    std::cout << std::endl;
    std::cout << "turn = " << turn << std::endl;
    std::cout << "size = " << queue.size() << std::endl;
    queue.print();
    std::cout << std::endl;

    if( turn == -1 && queue.size() == 1)
      turn = queue.get_next();
    pthread_mutex_unlock(&lock_queue);
    await_turn((void*)my_rank);
  }

  return NULL;
}

void *await_turn(void* rank){
  long my_rank = (long) rank;

  pthread_mutex_lock(&lock); // Cria lock para utilizar o forno
  while ( in_use || my_rank != turn)
  {
    pthread_cond_wait(&can_use[my_rank], &lock);
  }
  in_use = true;
  use_oven((void*)my_rank);
  in_use = false; //Comida fica pronta
  pthread_mutex_unlock(&lock); // Libera o forno
  if(turn > -1 && turn < RAJ){
    pthread_cond_signal(&can_use[turn]); //Chama o proximo
  }
  leave_oven((void*)my_rank);
 
  return NULL;
}

void *use_oven(void* rank) {
  long my_rank = (long) rank;

  std::cout << Charecter::char_name(my_rank) << " começa a esquentar algo" << std::endl;
  sleep(1);

  pthread_mutex_lock(&lock_queue);
  turn = queue.get_next();
  pthread_mutex_unlock(&lock_queue);

  return NULL;
}

void *leave_oven(void* rank){
  long my_rank = (long) rank;

  int eat_time = 3 + (int)(drand48()*3);
  std::cout << Charecter::char_name(my_rank) <<" vai comer" << std::endl;  
  sleep(eat_time); // Comendo!

  int work_time = 3 + (int)(drand48()*3);
  std::cout << Charecter::char_name(my_rank) <<" Voltou para o trabalho" << std::endl;
  sleep(work_time); // Trabalhando!

  return NULL;
}

// Raj checa se há deadlock!
void *check(void* rank) {
  while(done < num_executions * (thread_count - 1)) {
    std::cout << "dones: " << done << std::endl;
  //for (int i = 0; i < num_executions; i++) { 
    sleep(5);
    queue.monitor();
  }

  return NULL;
}
