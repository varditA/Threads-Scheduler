#include <iostream>
#include "uthreads.h"

void funcExample()
{
}

int main() {
    uthread_init(1000);
    uthread_spawn(&funcExample);
    std::cout << uthread_get_quantums(1);

    return 0;
}

