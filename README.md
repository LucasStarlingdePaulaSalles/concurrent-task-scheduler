# Big Bang Theory agendador de forno
Este repositório contém o código relativo ao trabalho prático do curso de Sistemas Operacionais ofertado pelo Departamento de Ciência da Computação. O trabalho consiste de implementar a sincronização de _threads_ que precisam acessar um mesmo recurso, causando condição de corrida. A proposta do trabalho é descrita na próxima seção. Em seguida, apresentamos as decisões que tomamos durante a implementação para resolver algumas ambiguidades da proposta. Por fim, apresentamos e destacamos as estruturas importantes da implementação do código.

## O Problema

O problema envolve o escalonamento do uso de um forno pelos personagens do programa de comédia estadunidense _The Big Bang Theory_. A utilização do forno segue algumas regras de priorização que serão descritas a seguir.

> Sheldon, Leonard e Howard compraram um micro-ondas juntos, que compartilham com Stuart e Kripke. Para decidir quem podia usar o forno, definiram o seguinte conjunto de regras: 
>
> - se o micro-ondas estiver liberado, quem chegar primeiro pode usar;
> - caso contrário, obviamente, quem chegar depois tem que esperar; 
> - se mais de uma pessoa estiver esperando para usar, valem as precedências: 
>   - Sheldon pode usar antes do Howard; 
>   - Howard pode usar antes do Leonard; 
>   - Leonard pode usar antes do Sheldon; 
>   - Stuart pode usar depois dos três; 
>   - Kripke é sempre o último. 
> - Quando alguém termina de usar o forno, deve liberá-lo para a próxima pessoa de maior prioridade, exceto em dois casos:
>   - Para evitar inanição (discutido a seguir);
>   - Quando houver deadlock (um ciclo de espera).
>
> Como eles têm namoradas (Leonard tem a Penny, Howard tem a Bernardette e Sheldon tem a Amy), elas entram no esquema do forno da seguinte forma: quando a namorada de um deles chega, aquele casal tem a preferencia do micro-ondas [...].
>
> [...] Como Stuart e Kripke não possuem namoradas eles são sempre relegados ao final da fila, podendo usar o micro-ondas somente quando todos os casais terminarem de utilizar, e outro casal não for utilizar.
>
> [...] Isso pode levar à inanição em casos de uso intenso do forno, daí é preciso criar uma regra para resolver o problema: após o almoço, os personagens voltam ao trabalho, o que pode dar uma chance para que Stuart e Kripke tenham alguma chance de utilizar o micro-ondas, mas pode ser que eles ainda sintam fome e voltem para casa para almoçar de novo.
>
> [...] Se você reparar as precedências definidas, vai notar que é possível ocorrer um deadlock -- é, eles sabem, mas são muito teimosos para mudar. Para evitar isso, o Raj periodicamente (a cada 5 segundos) confere a situação e, se encontrar o pessoal travado, pode escolher um deles aleatoriamente e liberá-lo para usar o forno.

Na prática, os personagens são implementados como _threads_ concorrentes, que concorrem pela utilização de um recurso centralizado, o forno. O desafio é implementar a sincronização do recurso central obedecendo as regras de prioridade dos processos descritas acima.

## Escolhas de implementação

Para implementar esse sistema, fizemos as seguintes decisões:

- Na ausência de seus cônjuges, as namoradas de Sheldon, Howard e Leonard herdam suas prioridades. Ou seja:
  - Amy pode usar antes de Bernardette.
  - Bernardette pode usar antes de Penny.
  - Penny pode usar antes de Amy.
- **Isso implica que, na ausência de casais, as namoradas também têm prioridade sobre os cônjuges de outras namoradas**. Por exemplo:
  - Amy pode usar antes de Howard.
  - Bernardette pode usar antes de Leonard.
  - Penny pode usar antes de Sheldon.
- **Isso também implica que o _deadlock_ pode ocorrer quando estão presentes quaisquer ciclos envolvendo as namoradas e os personagens, desde que não haja casais**. Por exemplo, se estiverem na fila Amy, Howard e Penny, ocorrerá _deadlock_. Nesse caso, Raj obedecerá as precedências da descrição do projeto, levando em consideração os itens acima.

## Detalhes de implementação

O sistema foi implementado na linguagem C++, utilizando a biblioteca `pthread.h` para criação e sincronização das _threads_. 

### Representação dos personagens

Primeiramente, definimos uma classe que representa os personagens, chamada `Character`. Essa classe, além de conter informações sobre o processo correspondente àquele personagem, possui as estruturas básicas de um nó em uma lista encadeada:

```c++
class Character {
    public:
        bool with_so;
        int rank;
        int so_rank;
        Character *next;
        Character(int);
        static std::string char_name(int);
};
```

Na declaração acima, `with_so` (lê-se "_with significant other_") é uma variável booleana que indica se o cônjuge do personagem se encontra na fila. **Essa variável merece uma atenção especial**, pois é uma decisão de implementação que faz com que não precisemos inserir na fila um nó para cada pessoa do casal. Dessa forma, o próprio nó do personagem indica se seu parceiro ou sua parceira se encontra na fila, e isso impacta diretamente a função `get_next()` da classe `OvenQueue`, descrita a seguir. As outras variáveis dessa classe indicam o ID do personagem (que pode ser utilizado para recuperar seu nome, através da função `char_name()`) e de seu parceiro ou parceira (`rank` e `so_rank`) e um ponteiro para o próximo nó na fila (`Character *next`).

### Fila para utilização do forno

A classe `OvenQueue` foi criada para gerenciar a fila de personagens aguardando para utilizar o forno. Essa é, possivelmente, a estrutura mais complexa implementada nesse sistema. Não vamos descrevê-la por completo nessa documentação, mas citaremos os pontos principais.

- A função `push()` realiza uma operação similar ao que se esperaria de uma inserção de um nó em uma lista encadeada. Entretanto, ao contrário de uma inserção padrão, **essa função também trata da inserção de um parceiro ou parceira na fila quando uma pessoa do casal já está presente**. Para isso, ela varre a fila procurando pela pessoa do casal que já está presente, e quando a encontra, altera o valor de `with_so` da pessoa presente para `true`.
- A função `get_next()` recupera o próximo personagem que poderá utilizar o forno, assim que a pessoa que está utilizando atualmente acaba. Esta função varre a fila e procura pela próxima pessoa que pode utilizar o forno de acordo com as precedências definidas. Ademais, antes de retornar o personagem que utilizará a fila, essa função checa se seu parceiro ou parceira também está na fila, e define ele ou ela como a pessoa que pode utilizar em seguida. Ou seja, além de definir a próxima pessoa que utilizará a fila, se essa pessoa estiver em casal, essa função também define que a pessoa seguinte deve ser seu parceirou ou parceira.
- A função `monitor()` é chamada pelo Raj a cada 5 segundos, e verifica se há algum ciclo na fila que gere deadlock. Caso positivo, escolhe alguém do ciclo para liberar aleatoriamente.
- A variável `pthread_mutex_t lock` é um mecanismo de _lock_ para realizar operações na fila, necessário para que todos as _threads_ consigam operar sobre a fila sem causar erros devidos ao _multithreading_.

### Locks de recursos

Duas variáveis do tipo `pthread_mutex_t` foram criadas. A primeira define um _lock_ para inserção/remoção de personagens na fila de _threads_ que aguardam a utilização do forno. A segunda define um _lock_ para a utilização do forno, dado que apenas um personagem pode utilizar o forno por vez.

Um vetor do tipo `pthread_cond_t` chamado `can_use` de tamanho 8 (para todos os personagens, menos Raj) é utilizado para estabelecer as condições de espera pelo forno. A condição de espera é controlada pelas variáveis `in_use`, que define se o forno está em uso e `turn`, que define quem é o próximo personagem que pode utilizar o forno (`turn = queue.get_next()`):

```c++
  while ( in_use || my_rank != turn)
  {
    pthread_cond_wait(&can_use[my_rank], &lock);
  }
```

### Funcionamento geral

Uma variável chamada `num_executions` recebe o valor do argumento passado ao programa para definir quantas vezes cada personagem deve entrar na fila para utilizar o forno. O personagem Raj realiza a verificação de _deadlock_ de 5 em 5 segundos até que todos os outros personagens tenham utilizado o forno todas as vezes (valor controlado por `num_executions * 8`).

Em seguida apresentamos o código da função `main()`, onde são todos os personagens são instanciados como _threads_ e cada execução é chamada:

```c++
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
```

Cada _thread_, ao ser instanciada, chama a função `enter_queue`, onde o personagem indica que quer utilizar o forno e entra na fila para utilizá-lo. Então, cada _thread_ espera o seu turno através da chamada da função `await_turn()`. 

Ao ser liberada para utilizar o forno, a _thread_ chama a função `use_oven`, que simula a utilização do forno e computa o próximo personagem que poderá utilizar o forno (chamando a função `queue.get_next()`). 

Por fim, ao terminar de utilizar o forno, a _thread_ chama a função `leave_oven()`, correspondente à ida ao trabalho.

A _thread_ de Raj chama a função `check()` a cada 5 segundos, em que verifica se há _deaclock_.

## Execução

O projeto é estruturado da seguinte forma:

```
.
├── doc
├── img
├── lib
│   └── oven-queue.hpp
├── Makefile
├── obj
├── README.md
├── src
│   ├── main
│   ├── main.cpp
│   ├── oven-queue.cpp
│   └── test.cpp
└── tests
    ├── saida10.txt
    ├── saida1.txt
    ├── saida2.txt
    ├── saida3.txt
    ├── saida4.txt
    ├── saida5.txt
    ├── saida6.txt
    ├── saida7.txt
    ├── saida8.txt
    └── saida9.txt
```

Ele deve ser compilado utilizando-se o comando `make`, que gerará o executável `./main`. Para testá-lo, basta chamar o executável. Se chamado sem parâmetro, cada personagem entrará na fila uma única vez. Para que os personagens entrem na fila mais de uma vez, basta passar a quantidade de vezes como argumento do programa, e.g.,  `./main 2`, `./main 4`, `./main 10`. Alguns arquivos de teste se encontram na pasta `tests`, embora esses arquivos tenham mensagens extras para efeito de depuração.

<hr>


Autores:

- @[LucasStarlingdePaulaSalles](https://github.com/LucasStarlingdePaulaSalles)
- @[lucasclopesr](https://github.com/lucasclopesr)

