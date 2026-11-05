#include <stdio.h>
#include <process.h>
#include <dos.h>

// Объявляем TLS-переменную. Ключевое слово static здесь важно,
// так как __declspec(thread) применим только к статическим данным.
__declspec(thread) int tls_var = 0;

void thread_func(void *arg) {
    // Каждый поток должен получить свою копию tls_var.
    printf("Thread %d: Initial tls_var = %d\n", (int)arg, tls_var);
    tls_var = (int)arg * 10;
    printf("Thread %d: Set tls_var to %d\n", (int)arg, tls_var);
    sleep(1); // Даём время другому потоку изменить свою копию
    printf("Thread %d: Final tls_var = %d\n", (int)arg, tls_var);
}

int main(void) {
    printf("Main: Initial tls_var = %d\n", tls_var);
    tls_var = 999;
    printf("Main: Set tls_var to 999\n");

    // Запускаем два потока. В OS/2 у _beginthread четыре аргумента.
    _beginthread(thread_func, NULL, 4096, (void*)1);
    _beginthread(thread_func, NULL, 4096, (void*)2);

    sleep(2); // Ждём завершения потоков
    printf("Main: Final tls_var = %d\n", tls_var);
    return 0;
}
