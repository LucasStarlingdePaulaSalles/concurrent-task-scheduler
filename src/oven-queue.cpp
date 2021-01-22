#include <oven-queue.hpp>

Charecter::Charecter(int char_rank){
    this->rank = char_rank;
    this->next = NULL;
    if(char_rank == SHELDON || char_rank == HOWARD || char_rank == LEONARD){
        this->so_rank = char_rank + 1;
    } else if( char_rank == AMY || char_rank == BERNARDETTE || char_rank == PENNY){
        this->so_rank = char_rank - 1;
    }else {
        this->so_rank = -1;
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
    for(int i = 0; i < CHARECTERS; i++){
        this->_agents[i] = false;
    }
}

void OvenQueue::queue(int char_rank){    
    Charecter *new_node = NULL;
    Charecter *node = NULL;
    new_node = new Charecter(char_rank);

    node = this->_first;
    if(node == NULL){
        this->_first = new_node;
        return;
    }

    while (node->next != NULL) {
        if(node->so_rank == char_rank){
            node->with_so = true;
            return;
        }
        node = node->next; 
    }
    node->next = new_node;
    return;
}

int OvenQueue::get_next(){
    Charecter *node = this->_first;
    Charecter *node_next = node->next;
    int next_rank;

    // mutex lock
    // while(this->_next == -1 && detectaQualquerDeadlock){
    //     wait
    // }

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

        next_rank = node->rank;
        if(node->with_so){
            this->_next = node->so_rank;
        }
        this->remove(node->rank);  
    } else {
        next_rank = this->_next;
        this->_next = -1;
    }

    return next_rank;
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

void OvenQueue::remove(int target){
    Charecter *curr = this->_first;
    Charecter *last = NULL;

    while (curr != NULL) { 
        if(curr->rank == target){
            last->next = curr->next;
            delete(curr);
            return;
        }
        last = curr;
        curr = curr->next; 
    } 
}