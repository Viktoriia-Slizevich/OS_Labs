#include <iostream>
#include <pthread.h>
#include <unistd.h> 

using namespace std;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int ready = 0; 

void* providerThread(void*) {
    while (true) {
        pthread_mutex_lock(&lock);
        
        while (ready == 1) {
            pthread_cond_wait(&cond2, &lock); // Ожидание сигнала от потребителя
        }

        ready = 1;
        cout << "Event sent!." << endl;
        
        pthread_cond_signal(&cond1); // Сигнал потребителю
        
        pthread_mutex_unlock(&lock);
    }
    return nullptr;
}

void* consumerThread(void*) {
    while (true) {
        pthread_mutex_lock(&lock);
        
        while (ready == 0) {
            pthread_cond_wait(&cond1, &lock); // Ожидание сигнала от поставщика
            cout << "Event got." << endl;
        }

        ready = 0;
        cout << "Event processed!." << endl;
        
        pthread_cond_signal(&cond2); // Сигнал поставщику
        
        pthread_mutex_unlock(&lock);
    }
    return nullptr;
}

int main() {
    pthread_t provider, consumer;
    
    pthread_create(&provider, nullptr, providerThread, nullptr);
    pthread_create(&consumer, nullptr, consumerThread, nullptr);
    
    pthread_join(provider, nullptr);
    pthread_join(consumer, nullptr);
    
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);
    
    return 0;
}
