#include <iostream>
#include <new>
#include <pthread.h>

/* Global variable */
int thread_count = 9;

void *Hello(void* rank);

int main(int argc, char* argv[]) {
  long thread;
  pthread_t thread_handles[thread_count];

  for (thread = 0; thread < thread_count; thread++)
    pthread_create(&thread_handles[thread], NULL, Hello, (void*) thread);
  
  std::cout << "Hello from the main thread" << std::endl;

  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);

  return 0;
}

void *Hello(void* rank) {
  long my_rank = (long) rank;

  std::cout << "Hello from thread " << my_rank << " of " << thread_count << std::endl;
  return NULL;
}

