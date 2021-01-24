#include <oven-queue.hpp>

int main(){
    OvenQueue fila;
    std::cout << fila.get_next() << std::endl;
    fila.push(0);
    fila.push(2);
    fila.push(4);
    std::cout << fila.get_next() << std::endl;
    std::cout << fila.get_next() << std::endl;
    return 0;
}