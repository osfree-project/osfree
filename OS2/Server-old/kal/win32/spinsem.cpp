#include <windows.h>
#include "spinsem.hpp"

static const spin_retries=100;

void SpinMutexSemaphore::Request() {
        HANDLE current_thread=GetCurrentThread();
        if(current_owner==current_thread) {
                own_count++;
                return;
        }
        //spin
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(InterlockedExchange(&toggle,1)==0) {
                        current_owner = current_thread;
                        own_count ++;
                        return;
                }
        }
        //give up - revert to system API
        InterlockedIncrement(&waiters);
        for(;;) {
                if(InterlockedExchange(&toggle,1)==0) {
                        InterlockedDecrement(&waiters);
                        current_owner = current_thread;
                        own_count ++;
                        return;
                }
                WaitForSingleObject(try_again, INFINITE);
        }
}

void SpinMutexSemaphore::Release() {
        if(--own_count==0) {
                current_owner = 0;
                toggle=0;
                if(waiters!=0)
                        PulseEvent(try_again);
        }
}

