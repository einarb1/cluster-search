#include <thread>
#include <atomic>
#include <iostream>

template<typename T>
class Cluster {
private:
    T* dataArray;
    int arraySize;
    T target;
    unsigned int threadLimit;
    std::atomic<int> resultIndex{-1};
    std::thread** threads; // * to arr of thread pointers
public:
    // dynamically allocate memory for array of size threadLimit
    Cluster(T* array, int size, const T& targetValue) : dataArray(array), arraySize(size), target(targetValue) {
        threadLimit = std::thread::hardware_concurrency();
        threads = new std::thread*[threadLimit];
    }
    
    // join threads before destruction
    ~Cluster() {
        for (unsigned int i = 0; i < threadLimit; ++i) {
            if (threads[i]) {
                if (threads[i]->joinable()) {
                    threads[i]->join();
                }
                delete threads[i]; // free pointers as needed
            }
        }
        delete[] threads; // free array of thread pointers
    }

    void searchCluster(int start, int end, int threadIndex) {
        for (int i = start; i < end && resultIndex.load() == -1; ++i) {
            if (dataArray[i] == target) {
                resultIndex.store(i);
                break; // success
            }
        }
    }
    // returns index if found, -1 if not found
    int Search() {
        // ensure even distribution
        int clusterSize = arraySize / threadLimit + (arraySize % threadLimit != 0);
        for (unsigned int i = 0; i < threadLimit; ++i) {
            int start = i * clusterSize;
            int end = ((i + 1) * clusterSize > arraySize) ? arraySize : (i + 1) * clusterSize;
            threads[i] = new std::thread(&Cluster::searchCluster, this, start, end, i); // make and store thread pointer
        }
        // ensure wait for all threads to finish
        for (unsigned int i = 0; i < threadLimit; ++i) {
            if (threads[i]->joinable()) {
                threads[i]->join();
            }
        }

        int foundIndex = resultIndex.load();
        return foundIndex != -1 ? foundIndex : -1;
    }
};
