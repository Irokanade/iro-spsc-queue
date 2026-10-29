#include <cstddef>
#include <iostream>
#include <thread>

#include <iro/lockfree/spsc_queue.hpp>


iro::lockfree::spsc_queue<int> q(1024);

void producer() {
    for (int i = 0; i < 100; i++) {
        q.push(i);
    }
}

void consumer() {
    for (int i = 0; i < 100; i++) {
        int* item;
        while ((item = q.front()) == nullptr) {
            std::this_thread::yield();
        }
        std::cout << *item << '\n';
        q.pop();
    }
}

int main(int argc, char* argv[]) {
    std::thread consumer_thread(consumer);
    std::thread producer_thread(producer);

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
