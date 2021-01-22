#ifndef OVEN_QUEUE
#define OVEN_QUEUE
#include <iostream>
#include <pthread.h>
#define CHARECTERS 9
#define SHELDON 0
#define AMY 1
#define HOWARD 2
#define BERNARDETTE 3
#define LEONARD 4
#define PENNY 5
#define STUART 6
#define KRIPKE 7
#define RAJ 8



class Charecter {
    public:
        bool with_so;
        int rank;
        int so_rank;
        Charecter *next;
        Charecter(int);
        void join_so();
};

class OvenQueue {
    public:
        int get_next();
        void queue(int);
        OvenQueue();
        void print();
    private:
        int _size;
        int _next;
        bool _agents[CHARECTERS];
        Charecter *_first;
        void add(int);
        void remove(int);
        void addCouple();
        void monitor();
        bool nextHasPriority(int, int);
        pthread_mutex_t lock;
        pthread_cond_t deadlock;
};


#endif