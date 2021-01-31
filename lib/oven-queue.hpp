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



class Character {
    public:
        bool with_so;
        int rank;
        int so_rank;
        Character *next;
        Character(int);
        static std::string char_name(int);
};

class OvenQueue {
    public:
        int get_next();
        void push(int);
        int size();
        OvenQueue();
        ~OvenQueue();
        void monitor();
        void print();
        void change_turn(int);
    private:
        int _size;
        int _next;
        int _curr;
        bool _agents[CHARECTERS];
        Character *_first;
        void remove(int);
        void add(int);
        void addCouple();
        bool nextHasPriority(int, int);
        bool inDeadlock();
        pthread_mutex_t lock;
        pthread_cond_t raj;
};


#endif
