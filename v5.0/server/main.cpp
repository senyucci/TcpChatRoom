#include "reactor.h"

Reactor myReactor;

int main()
{
    myReactor.InitThreadpool();
    myReactor.StartEpoll(&myReactor);
    
    return 0;
}
