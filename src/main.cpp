#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <vector>

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
 * - Detalhes:
 *   Sempre que uma namorada estiver na fila, seu namorado também estará
 *   Stuart e Kripke podem acabar morrendo de inanição
 *   Após comer, os personagens voltam ao trabalho (sleep [3-6])
 *   Usar o forno gasta 1 segundo
 *
 *
 */

#define SHELDON 0
#define AMY 1
#define HOWARD 2
#define BERNARDETTE 3
#define LEONARD 4
#define PENNY 5
#define STUART 6
#define KRIPKE 7
#define RAJ 8

/* Global variable */
int thread_count = 9;
pthread_mutex_t lock;
std::vector <int> queue;

void *enter_queue(void* rank);
void *use_oven(void* rank);
void *check(void* rank);

int main(int argc, char* argv[]) {
  long thread;
  pthread_mutex_init(&lock, NULL); // Declara o lock para o forno

  // Declara as threads correspondentes a cada personagem (e.g., thread_handles[0] = Sheldon)
  pthread_t thread_handles[thread_count];

  for (thread = 0; thread < thread_count-1; thread++)
    pthread_create(&thread_handles[thread], NULL, enter_queue, (void*) thread);
  
  // Cria thread correspondente ao Raj
  pthread_create(&thread_handles[thread_count], NULL, check, (void*) thread);

  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);

  pthread_mutex_destroy(&lock);
  return 0;
}

void *enter_queue(void* rank) {
  long my_rank = (long) rank;

  // Personagem declara que quer utilizar o forno!
  std::cout << my_rank << " quer usar o forno" << std::endl;
  //queue.push_back(my_rank);

  return NULL;
}

void *use_oven(void* rank) {
  long my_rank = (long) rank;

  pthread_mutex_lock(&lock); // Cria lock para utilizar o forno

  std::cout << my_rank << " começa a esquentar algo" << std::endl;
  sleep(5);

  pthread_mutex_unlock(&lock); // Libera o forno

  std::cout << my_rank << " vai comer" << std::endl;
  sleep(3); // Comendo!

  return NULL;
}

// Raj checa se há deadlock!
void *check(void* rank) {

  std::cout  << "Raj verifica se há deadlock" << std::endl;
  sleep(5);

  return NULL;
}

