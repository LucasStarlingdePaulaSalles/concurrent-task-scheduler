#include <oven-queue.hpp>

int main(){
    OvenQueue fila;
    fila.queue(0);
    fila.queue(4);
    fila.queue(1);
    std::cout << fila.get_next() << std::endl;
    std::cout << fila.get_next() << std::endl;
    std::cout << fila.get_next() << std::endl;
    return 0;
}