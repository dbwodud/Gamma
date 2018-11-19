#include<iostream>

extern "C"{
    int fib(int);
}

int main(){
    std::cout<<fib(5)<<std::endl;
    return 0;
}
