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

Na declaração acima, `with_so` (lê-se "_with significant other_") é uma variável booleana que indica se o cônjuge do personagem se encontra na fila. **Essa variável merece uma atenção especial**, pois é uma decisão de implementação que faz com que não precisemos inserir na fila um nó para cada pessoa do casal. Dessa forma, o próprio nó do personagem indica se seu parceiro ou sua parceira se encontra na fila, e isso impacta diretamente a função `get_next()` da classe `OvenQueue`, descrita a seguir. As outras variáveis dessa classe indicam o ID do personagem e de seu parceiro ou parceira (`rank` e `so_rank`), seu nome e um ponteiro para o próximo nó na fila (`Character *next`).

### Fila para utilização do forno

A classe `OvenQueue` foi criada para gerenciar a fila de personagens aguardando para utilizar o forno. Essa é, possivelmente, a estrutura mais complexa implementada nesse sistema. Não vamos descrevê-la por completo nessa documentação, mas citaremos os pontos principais.

- A função `push()` realiza uma operação similar ao que se esperaria de uma inserção de um nó em uma lista encadeada. Entretanto, ao contrário de uma inserção padrão, **essa função também trata da inserção de um parceiro ou parceira na fila quando uma pessoa do casal já está presente**. Para isso, ela varre a fila procurando pela pessoa do casal que já está presente, e quando a encontra, altera o valor de `with_so` da pessoa presente para `true`.
- A função `get_next()` recupera o próximo personagem que poderá utilizar o forno, assim que a pessoa que está utilizando atualmente acaba. Esta função varre a fila e procura pela próxima pessoa que pode utilizar o forno de acordo com as precedências definidas. Ademais, antes de retornar o personagem que utilizará a fila, essa função checa se seu parceiro ou parceira também está na fila, e define ele ou ela como a pessoa que pode utilizar em seguida. Ou seja, além de definir a próxima pessoa que utilizará a fila, se essa pessoa estiver em casal, essa função também define que a pessoa seguinte deve ser seu parceirou ou parceira.
- A função `monitor()` é chamada pelo Raj a cada 5 segundos, e verifica se há algum ciclo na fila que gere deadlock. Caso positivo, escolhe alguém do ciclo para liberar aleatoriamente.
- A variável `pthread_mutex_t lock` é um mecanismo de _lock_ para realizar operações na fila, necessário para que todos as _threads_ consigam operar sobre a fila sem causar erros devidos ao _multithreading_.

### Locks de recursos

Duas variáveis do tipo `pthread_mutex_t` foram criadas. A primeira define um _lock_ para inserção/remoção de personagens na fila de _threads_ que aguardam a utilização do forno. A segunda define um _lock_ para a utilização do forno, dado que apenas um personagem pode utilizar o forno por vez.



Autores:

- @[LucasStarlingdePaulaSalles](https://github.com/LucasStarlingdePaulaSalles)
- @[lucasclopesr](https://github.com/lucasclopesr)

