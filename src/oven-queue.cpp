#include <oven-queue.hpp>

Charecter::Charecter(int char_rank){
    this->rank = char_rank;
    this->next = NULL;
    this->with_so = false;
    if(char_rank == SHELDON || char_rank == HOWARD || char_rank == LEONARD){
      this->so_rank = char_rank + 1;
    } else if( char_rank == AMY || char_rank == BERNARDETTE || char_rank == PENNY){
      this->so_rank = char_rank - 1;
    }else {
      this->so_rank = -1;
    }
}

std::string Charecter::char_name(int id){
  switch (id)
  {
    case SHELDON: return "Sheldon";
    case AMY: return "Amy";
    case HOWARD: return "Howard";
    case BERNARDETTE: return "Bernadette";
    case LEONARD: return "Leonard";
    case PENNY: return "Penny";
    case STUART: return "Stuart";
    case KRIPKE: return "Kripke"; 
    default: return "NULL";
  }
}

void OvenQueue::print() 
{ 
    Charecter *node = this->_first;

    while (node != NULL) { 
        std::cout << node->rank << " ";
        if(node->with_so){
        std::cout << node->so_rank << " ";
        }
        node = node->next; 
    } 
    std::cout << std::endl;
}

OvenQueue::OvenQueue(){
    this->_size = 0;
    this->_first = NULL;
    this->_next = -1;
    this->_curr = -1;
    for(int i = 0; i < CHARECTERS; i++){
        this->_agents[i] = false;
    }
    pthread_mutex_init(&this->lock,NULL);
}

OvenQueue::~OvenQueue(){
    pthread_mutex_destroy(&this->lock);
}

int OvenQueue::size(){
    int size = this->_size;
    return size;
}

void OvenQueue::push(int char_rank){    
    Charecter *new_node = NULL;
    Charecter *last = NULL;
    Charecter *curr = this->_first;
    new_node = new Charecter(char_rank);

    pthread_mutex_lock(&this->lock);

    if( this->_curr > -1 && new_node->so_rank == this->_curr){
        this->_next = char_rank;
    } else if(curr == NULL){
        this->_first = new_node;
        this->_agents[char_rank] = true;
        this->_size++;
    } else {
        this->_size++;
        while (curr != NULL) {
            if(curr->so_rank == char_rank){
                curr->with_so = true;
                this->_agents[char_rank] = true;
                last = NULL;
                break;
            }
            last = curr;
            curr = curr->next; 
        }
        if(last != NULL){
            this->_agents[char_rank] = true;
            last->next = new_node;
        }
    }

    pthread_mutex_unlock(&this->lock);
    return;
}

int OvenQueue::get_next(){
    Charecter *node = this->_first;
    Charecter *node_next = NULL;
    if(node != NULL){
        node_next = node->next;
    }
    int next_rank = -1;

    // mutex lock
    pthread_mutex_lock(&this->lock);
    while(this->_next == -1 && this->inDeadlock()){
        pthread_cond_wait(&this->raj,&this->lock);
    }

    if(this->_next == -1){
        while(node_next != NULL){
            if(node_next->with_so){
                if(node->with_so){
                    if(this->nextHasPriority(node_next->rank, node->rank)){
                        node = node_next;
                    }
                } else {
                    node = node_next;
                }
            }else {
                if(!node->with_so){
                    if(this->nextHasPriority(node_next->rank, node->rank)){
                        node = node_next;
                    }
                }
            }
            node_next = node_next->next;
        }

        if(node != NULL){
            next_rank = node->rank;
            this->remove(node->rank);
        }
    } else {
        next_rank = this->_next;
        this->_next = -1;
        if(this->inDeadlock()){
            remove(next_rank);
        }
    }
    this->_curr = next_rank;
    pthread_mutex_unlock(&this->lock);
    return next_rank;
}

bool OvenQueue::inDeadlock(){
    return (
        ((this->_agents[0] && this->_agents[2] && this->_agents[4]) &&
        (!this->_agents[1] && !this->_agents[3] && !this->_agents[5])) ||
        ((!this->_agents[0] && !this->_agents[2] && !this->_agents[4]) &&
        (this->_agents[1] && this->_agents[3] && this->_agents[5])) ||
        ((this->_agents[0] && this->_agents[2] && this->_agents[4]) &&
        (this->_agents[1] && this->_agents[3] && this->_agents[5]))    
    );
}

bool OvenQueue::nextHasPriority(int next, int curr){
    if( curr % 2 == 1 && curr < 6){
        curr--;
    }
    if( next % 2 == 1 && next < 6){
        next--;
    }
    if(curr + next == 4 && std::abs(curr - next) == 4){
        if(next == 4){
            return true;
        }else{
            return false;
        }
    } else {
        if(next < curr){
            return true;
        }else{
            return false;
        }
    }
}

void OvenQueue::remove(int target_rank){
    Charecter *curr = this->_first;
    Charecter *last = NULL;
    this->_agents[target_rank] = false;
    while (curr != NULL) { 
        if(curr->rank == target_rank ){
            if (last == NULL) {
                this->_first = curr->next;
            } else {
                last->next = curr->next;
            }

            this->_size--;
            if(curr->with_so){
                this->_next = curr->so_rank;
                this->_agents[curr->so_rank] = false;
                this->_size--;
            }

            delete(curr);

            return;
        }
        last = curr;
        curr = curr->next; 
    } 
}

void OvenQueue::monitor(){
    int chars[3];
    int curr_rank = -1;
    Charecter *node = this->_first;
    
    std::cout << std::endl;
    std::cout << "Raj monitora a situação: " << std::endl;
    std::cout << "size = " << this->size() << std::endl;
    this->print();
    std::cout << std::endl;

    if(this->inDeadlock()){
        pthread_mutex_lock(&this->lock);
        std::cout << "Raj verifica que há deadlock, liberando: ";
        while (node != NULL) {
            curr_rank = node->rank; 
            if(curr_rank == 0 || curr_rank == 1)
                chars[0] = curr_rank;
            if(curr_rank == 2 || curr_rank == 3)
                chars[1] = curr_rank;
            if(curr_rank == 4 || curr_rank == 5)
                chars[2] = curr_rank;
            node = node->next; 
        } 
        srand((unsigned) time(0));
        int raj_choice = (rand() % 3);
        this->_next = chars[raj_choice];
        std::cout << Charecter::char_name(this->_next) << std::endl;
        pthread_mutex_unlock(&this->lock);
        pthread_cond_signal(&this->raj);
    }else{
        std::cout << "Raj verifica que não há deadlock" << std::endl;
    }
}

void OvenQueue::change_turn(int turn_rank){
    this->_curr = turn_rank;
}