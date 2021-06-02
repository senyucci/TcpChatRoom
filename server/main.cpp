#include "reactor.h"

Reactor myReactor;

int main()
{
    myReactor.InitServer();
    myReactor.StartEpoll(&myReactor);
    
    return 0;
}
