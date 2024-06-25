#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h> // for usleep
#include "monitor.h" // Include the Monitor class definition
#include "helper.h" 
#include "WriteOutput.h" // Include the WriteOutput header
#include "semaphore.h"

using namespace std;

struct PathSegment {
    int connector_id;
    int connector_direction;
    std::string connector_type;
};

// Define a structure to represent car details
struct CarDetails {
    int id;
    int travel_time;
    std::vector<PathSegment> path;
};

// Define the NarrowBridge class inheriting from Monitor
class NarrowBridge {
public:
    int id;
    int travel_time;
    int max_wait_time;
    int current_dir;
    int car_inside;
    int startingtime1;
    int startingtime2;
    int finishingtime1;
    int finishingtime2;
    int waitingcount1;
    int waitingcount2;
    sem_t semaphore2;
    sem_t semaphore1;
    pthread_mutex_t bridge_mutex;
    pthread_mutex_t bridge_mutex2;

    NarrowBridge() : id(0), travel_time(0), max_wait_time(0), current_dir(-1), car_inside(0), startingtime1(0) , startingtime2(0) , finishingtime1(0) , finishingtime2(0), waitingcount1(0) , waitingcount2(0) {
        pthread_mutex_init(&bridge_mutex, NULL);
        pthread_mutex_init(&bridge_mutex2, NULL);
        sem_init(&semaphore2, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore1, 0, 100); // Initialize the semaphore with a value of 1
    } // Default constructor

    NarrowBridge(int _id, int _travel_time, int _max_wait_time) : id(_id), travel_time(_travel_time), max_wait_time(_max_wait_time), current_dir(-1), car_inside(0) , startingtime1(0) , startingtime2(0) , finishingtime1(0) , finishingtime2(0), waitingcount1(0) , waitingcount2(0) {
        pthread_mutex_init(&bridge_mutex, NULL);
        pthread_mutex_init(&bridge_mutex2, NULL);
        sem_init(&semaphore2, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore1, 0, 100); // Initialize the semaphore with a value of 1
    }

    void enterBridge(int car_id, int bridge_id, int arrival_timestamp, int connector_direction) {
        WriteOutput(car_id, 'N', bridge_id, TRAVEL); // Print AID: 0 traveling to connector
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'N', bridge_id, ARRIVE); // Notify arrival at the connector
        int value;
        sem_getvalue(&semaphore1, &value);
        //std::cout << "Semaphore value1: " << value<< "for car"<<car_id << std::endl;
        //waitingcount1+=1;
        pthread_mutex_lock(&bridge_mutex);

        Label:
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;  // No need to add seconds
        ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

        // Check if the nanosecond field overflowed
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        int ret;
        
        ret = sem_timedwait(&semaphore1, &ts);
        if (ret == -1 && errno == ETIMEDOUT) {
            //std::cout << "Timeout occurred!\n"<< car_id<<endl;
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore2) == 0);
            while(current_dir!=-1 && current_dir!= connector_direction) usleep(1000);
            //pthread_mutex_unlock(&bridge_mutex);
        } else if (ret == 0) {
            //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
            //sem_post(&semaphore2);  // Release the semaphore
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore2) == 0);
            //pthread_mutex_unlock(&bridge_mutex);
        }
        //sem_wait(&semaphore1);
        //waitingcount1-=1;
        //while (sem_trywait(&semaphore2) == 0);
        sem_getvalue(&semaphore2, &value);
        //std::cout << "Semaphore value2: " << value<< "for car"<<car_id  << std::endl;
        //pthread_mutex_lock(&bridge_mutex); // Lock the mutex

        if (car_inside == 0) {
            current_dir = connector_direction;
        }
        if(car_inside) sleep_milli(PASS_DELAY);
        
        

        //pthread_mutex_unlock(&bridge_mutex); // Unlock the mutex
        sem_getvalue(&semaphore1, &value);
        if(value==0){
            //cout<<"dir 1 "<<car_id<<endl;
            goto Label;
        }
        pthread_mutex_unlock(&bridge_mutex); // Unlock the mutex

        car_inside += 1;
        //sleep_milli(PASS_DELAY/20);
         // Increment the semaphore value to signal release

        WriteOutput(car_id, 'N', bridge_id, START_PASSING); // Start passing the connector
        usleep(travel_time * 1000); // Simulate the time taken to pass the bridge

        WriteOutput(car_id, 'N', bridge_id, FINISH_PASSING); 
        car_inside -= 1;

        if (car_inside == 0) {
            current_dir = -1; // Reset current direction if the bridge is empty
            for (int i = 0; i < 100; ++i) {
                sem_post(&semaphore2);
            }// Increment the semaphore value to allow passing in the opposite direction
        }
        //sem_post(&semaphore1); 

    }

    void enterBridge2(int car_id, int bridge_id, int arrival_timestamp, int connector_direction) {
        WriteOutput(car_id, 'N', bridge_id, TRAVEL); // Print AID: 0 traveling to connector

        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'N', bridge_id, ARRIVE); // Notify arrival at the connector
        int value;
        sem_getvalue(&semaphore2, &value);
        //std::cout << "Semaphore value1: " << value<< "for car"<<car_id << std::endl;
        //waitingcount2+=1;
        //sem_wait(&semaphore2);
        //waitingcount2-=1;
        pthread_mutex_lock(&bridge_mutex2);

        Label2:
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;  // No need to add seconds
        ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

        // Check if the nanosecond field overflowed
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        int ret = sem_timedwait(&semaphore2, &ts);
        if (ret == -1 && errno == ETIMEDOUT) {
            //std::cout << "Timeout occurred2!\n"<<endl;
            //pthread_mutex_lock(&bridge_mutex2);
            while (sem_trywait(&semaphore1) == 0);
            while(current_dir!=-1 && current_dir!= connector_direction) usleep(1000);
            //pthread_mutex_unlock(&bridge_mutex2);
        } else if (ret == 0) {
            //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
            //sem_post(&semaphore2);  // Release the semaphore
            //pthread_mutex_lock(&bridge_mutex2);
            while (sem_trywait(&semaphore1) == 0);
            //pthread_mutex_unlock(&bridge_mutex2);
        }
        //while (sem_trywait(&semaphore1) == 0); // Wait until no car is passing through the bridge
        //sem_getvalue(&semaphore2, &value);
        //std::cout << "Semaphore value2: " << value<< "for car"<<car_id  << std::endl;
        //pthread_mutex_lock(&bridge_mutex2); // Lock the mutex

        if (car_inside == 0) {
            current_dir = connector_direction; // Update current direction if the bridge is empty
        }
        if(car_inside) sleep_milli(PASS_DELAY);
        
        

        //pthread_mutex_unlock(&bridge_mutex2); // Unlock the mutex
        sem_getvalue(&semaphore2, &value);
        if(value==0){
            //cout<<"dir 2 "<<car_id<<endl;

            goto Label2;
        }
        pthread_mutex_unlock(&bridge_mutex2); // Unlock the mutex
        car_inside += 1;
        //sleep_milli(PASS_DELAY/20);
         // Increment the semaphore value to signal release

        WriteOutput(car_id, 'N', bridge_id, START_PASSING); // Start passing the connector
        usleep(travel_time * 1000); // Simulate the time taken to pass the bridge

        WriteOutput(car_id, 'N', bridge_id, FINISH_PASSING); 
        car_inside -= 1;
        
        if (car_inside == 0) {
            current_dir = -1; // Reset current direction if the bridge is empty
            for (int i = 0; i < 100; ++i) {
                sem_post(&semaphore1);
            }// Increment the semaphore value to allow passing in the opposite direction
        }
        //sem_post(&semaphore2); 
    }

    void exitBridge() {
        // No need to implement this method as it's not used in the semaphore-based synchronization
    }
};

class CrossRoad {
public:
    public:
    int id;
    int travel_time;
    int max_wait_time;
    int current_dir;
    int car_inside;
    int startingtime1;
    int startingtime2;
    int finishingtime1;
    int finishingtime2;
    int waitingcount1;
    int waitingcount2;
    int waiting1;
    int waiting2;
    int waiting3;
    int waiting4;
    sem_t semaphore2;
    sem_t semaphore1;
    sem_t semaphore3;
    sem_t semaphore4;
    pthread_mutex_t crossroad_mutex;
    pthread_mutex_t crossroad_mutex2;
    pthread_mutex_t crossroad_mutex3;
    pthread_mutex_t crossroad_mutex4;

    CrossRoad() : id(0), travel_time(0), max_wait_time(0), current_dir(-1), car_inside(0),waiting1(0), waiting2(0), waiting3(0), waiting4(0), startingtime1(0) , startingtime2(0) , finishingtime1(0) , finishingtime2(0), waitingcount1(0) , waitingcount2(0) {
        pthread_mutex_init(&crossroad_mutex, NULL);
        pthread_mutex_init(&crossroad_mutex2, NULL);
        pthread_mutex_init(&crossroad_mutex3, NULL);
        pthread_mutex_init(&crossroad_mutex4, NULL);
        sem_init(&semaphore2, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore1, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore3, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore4, 0, 100); // Initialize the semaphore with a value of 1
    } // Default constructor

    CrossRoad(int _id, int _travel_time, int _max_wait_time) : id(_id), travel_time(_travel_time), max_wait_time(_max_wait_time), current_dir(-1), waiting1(0), waiting2(0), waiting3(0), waiting4(0),car_inside(0) , startingtime1(0) , startingtime2(0) , finishingtime1(0) , finishingtime2(0), waitingcount1(0) , waitingcount2(0) {
        pthread_mutex_init(&crossroad_mutex, NULL);
        pthread_mutex_init(&crossroad_mutex2, NULL);
        pthread_mutex_init(&crossroad_mutex3, NULL);
        pthread_mutex_init(&crossroad_mutex4, NULL);
        sem_init(&semaphore2, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore1, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore3, 0, 100); // Initialize the semaphore with a value of 1
        sem_init(&semaphore4, 0, 100); // Initialize the semaphore with a value of 1
    }

    void enterCrossRoad1(int car_id, int crossroad_id, int arrival_timestamp, int connector_direction) {
        WriteOutput(car_id, 'C', crossroad_id, TRAVEL); // Print AID: 0 traveling to crossroad
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'C', crossroad_id, ARRIVE); // Notify arrival at the crossroad
        int value;

        //sem_wait(&semaphore1);

        //while (sem_trywait(&semaphore2) == 0);
        //while (sem_trywait(&semaphore3) == 0);
        //while (sem_trywait(&semaphore4) == 0);
        waiting1++;
        pthread_mutex_lock(&crossroad_mutex);

        Label3:
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;  // No need to add seconds
        ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

        // Check if the nanosecond field overflowed
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        int ret;
        
        ret = sem_timedwait(&semaphore1, &ts);
        if (ret == -1 && errno == ETIMEDOUT) {
            //std::cout << "Timeout occurred1!\n"<< car_id<<endl;
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore2) == 0);
            while (sem_trywait(&semaphore3) == 0);
            while (sem_trywait(&semaphore4) == 0);
            while(current_dir!=-1 && current_dir!= connector_direction) usleep(1000);
            //pthread_mutex_unlock(&bridge_mutex);
        } else if (ret == 0) {
            //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
            //sem_post(&semaphore2);  // Release the semaphore
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore2) == 0);
            while (sem_trywait(&semaphore3) == 0);
            while (sem_trywait(&semaphore4) == 0);
            //pthread_mutex_unlock(&bridge_mutex);
        }

        //pthread_mutex_lock(&crossroad_mutex); // Lock the mutex
        if (car_inside == 0) {
            current_dir = connector_direction;
        }
        if(car_inside) sleep_milli(PASS_DELAY);


        // Simulate passing through the crossroad
        //sleep_milli(PASS_DELAY/10);
        sem_getvalue(&semaphore1, &value);
        //cout<<"semm: "<<value<<endl;
        if(value==0){
            //cout<<"dir 1 "<<car_id<<endl;
            goto Label3;
        }
        pthread_mutex_unlock(&crossroad_mutex);

        car_inside += 1;

        WriteOutput(car_id, 'C', crossroad_id, START_PASSING);   
        usleep(travel_time * 1000); // Simulate the time taken to pass the bridge
     
        WriteOutput(car_id, 'C', crossroad_id, FINISH_PASSING);
        car_inside -= 1;
        waiting1--;
        if (car_inside == 0) {
            current_dir = -1; // Reset current direction if the bridge is empty
            for (int i = 0; i < 100; ++i) {
                if(waiting2>0){
                    sem_post(&semaphore2);
                }
                else if(waiting3>0){
                    sem_post(&semaphore3);
                }
                else if(waiting4>0){
                    sem_post(&semaphore4);
                }
                else{
                    sem_post(&semaphore2);
                    sem_post(&semaphore3);
                    sem_post(&semaphore4);
                }

            }// Increment the semaphore value to allow passing in the opposite direction
        }
        //sem_post(&semaphore1);  // Notify finish passing the crossroad
    }
    void enterCrossRoad2(int car_id, int crossroad_id, int arrival_timestamp, int connector_direction) {
        WriteOutput(car_id, 'C', crossroad_id, TRAVEL); // Print AID: 0 traveling to crossroad
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'C', crossroad_id, ARRIVE); // Notify arrival at the crossroad
        int value;

        //sem_wait(&semaphore2);

        //while (sem_trywait(&semaphore1) == 0);
        //while (sem_trywait(&semaphore3) == 0);
        //while (sem_trywait(&semaphore4) == 0);
        waiting2++;

        pthread_mutex_lock(&crossroad_mutex2); // Lock the mutex
        Label4:
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;  // No need to add seconds
        ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

        // Check if the nanosecond field overflowed
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        int ret;
        
        ret = sem_timedwait(&semaphore2, &ts);
        if (ret == -1 && errno == ETIMEDOUT) {
            //std::cout << "Timeout occurred2!\n"<< car_id<<endl;
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore1) == 0);
            while (sem_trywait(&semaphore3) == 0);
            while (sem_trywait(&semaphore4) == 0);
            while(current_dir!=-1 && current_dir!= connector_direction) usleep(1000);
            //pthread_mutex_unlock(&bridge_mutex);
        } else if (ret == 0) {
            //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
            //sem_post(&semaphore2);  // Release the semaphore
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore1) == 0);
            while (sem_trywait(&semaphore3) == 0);
            while (sem_trywait(&semaphore4) == 0);
            //pthread_mutex_unlock(&bridge_mutex);
        }

        //pthread_mutex_lock(&crossroad_mutex); // Lock the mutex
        if (car_inside == 0) {
            current_dir = connector_direction;
        }
        if(car_inside) sleep_milli(PASS_DELAY);


        // Simulate passing through the crossroad
        //sleep_milli(PASS_DELAY/10);
        sem_getvalue(&semaphore2, &value);
        if(value==0){
            //cout<<"dir 1 "<<car_id<<endl;
            goto Label4;
        }
        pthread_mutex_unlock(&crossroad_mutex2);

        car_inside += 1;

        WriteOutput(car_id, 'C', crossroad_id, START_PASSING);   
        usleep(travel_time * 1000); // Simulate the time taken to pass the bridge
     
        WriteOutput(car_id, 'C', crossroad_id, FINISH_PASSING);
        car_inside -= 1;
        waiting2--;

        if (car_inside == 0) {
            current_dir = -1; // Reset current direction if the bridge is empty
            for (int i = 0; i < 100; ++i) {
                if(waiting3>0){
                    sem_post(&semaphore3);
                }
                else if(waiting4>0){
                    sem_post(&semaphore4);
                }
                else if(waiting1>0){
                    sem_post(&semaphore1);
                }
                else{
                    sem_post(&semaphore1);
                    sem_post(&semaphore3);
                    sem_post(&semaphore4);
                }
            }// Increment the semaphore value to allow passing in the opposite direction
        }
        //sem_post(&semaphore2);  // Notify finish passing the crossroad
    }
    void enterCrossRoad3(int car_id, int crossroad_id, int arrival_timestamp, int connector_direction) {
        WriteOutput(car_id, 'C', crossroad_id, TRAVEL); // Print AID: 0 traveling to crossroad
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'C', crossroad_id, ARRIVE); // Notify arrival at the crossroad
        int value;

        //sem_wait(&semaphore2);

        //while (sem_trywait(&semaphore1) == 0);
        //while (sem_trywait(&semaphore3) == 0);
        //while (sem_trywait(&semaphore4) == 0);
        waiting3++;

        pthread_mutex_lock(&crossroad_mutex3); // Lock the mutex
        Label5:
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;  // No need to add seconds
        ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

        // Check if the nanosecond field overflowed
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        int ret;
        
        ret = sem_timedwait(&semaphore3, &ts);
        if (ret == -1 && errno == ETIMEDOUT) {
            //std::cout << "Timeout occurred3!\n"<< car_id<<endl;
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore1) == 0);
            while (sem_trywait(&semaphore2) == 0);
            while (sem_trywait(&semaphore4) == 0);
            while(current_dir!=-1 && current_dir!= connector_direction) usleep(1000);
            //pthread_mutex_unlock(&bridge_mutex);
        } else if (ret == 0) {
            //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
            //sem_post(&semaphore2);  // Release the semaphore
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore1) == 0);
            while (sem_trywait(&semaphore2) == 0);
            while (sem_trywait(&semaphore4) == 0);
            //pthread_mutex_unlock(&bridge_mutex);
        }

        //pthread_mutex_lock(&crossroad_mutex); // Lock the mutex
        if (car_inside == 0) {
            current_dir = connector_direction;
        }
        if(car_inside) sleep_milli(PASS_DELAY);


        // Simulate passing through the crossroad
        //sleep_milli(PASS_DELAY/10);
        sem_getvalue(&semaphore3, &value);
        if(value==0){
            //cout<<"dir 1 "<<car_id<<endl;
            goto Label5;
        }
        pthread_mutex_unlock(&crossroad_mutex3);

        car_inside += 1;

        WriteOutput(car_id, 'C', crossroad_id, START_PASSING);   
        usleep(travel_time * 1000); // Simulate the time taken to pass the bridge
     
        WriteOutput(car_id, 'C', crossroad_id, FINISH_PASSING);
        car_inside -= 1;
        waiting3--;

        if (car_inside == 0) {
            current_dir = -1; // Reset current direction if the bridge is empty
            for (int i = 0; i < 100; ++i) {
                if(waiting4>0){
                    sem_post(&semaphore4);
                }
                else if(waiting1>0){
                    sem_post(&semaphore1);
                }
                else if(waiting2>0){
                    sem_post(&semaphore2);
                }
                else{
                    sem_post(&semaphore2);
                    sem_post(&semaphore1);
                    sem_post(&semaphore4);
                }

            }// Increment the semaphore value to allow passing in the opposite direction
        }
        //sem_post(&semaphore2);  // Notify finish passing the crossroad
    }
    void enterCrossRoad4(int car_id, int crossroad_id, int arrival_timestamp, int connector_direction) {
        WriteOutput(car_id, 'C', crossroad_id, TRAVEL); // Print AID: 0 traveling to crossroad
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'C', crossroad_id, ARRIVE); // Notify arrival at the crossroad
        int value;

        //sem_wait(&semaphore2);

        //while (sem_trywait(&semaphore1) == 0);
        //while (sem_trywait(&semaphore3) == 0);
        //while (sem_trywait(&semaphore4) == 0);
        waiting4++;

        pthread_mutex_lock(&crossroad_mutex4); // Lock the mutex
        Label6:
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;  // No need to add seconds
        ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

        // Check if the nanosecond field overflowed
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        
        int ret;
        
        ret = sem_timedwait(&semaphore4, &ts);
        if (ret == -1 && errno == ETIMEDOUT) {
            //std::cout << "Timeout occurred4!\n"<< car_id<<endl;
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore1) == 0);
            while (sem_trywait(&semaphore3) == 0);
            while (sem_trywait(&semaphore2) == 0);
            while(current_dir!=-1 && current_dir!= connector_direction) usleep(1000);
            //pthread_mutex_unlock(&bridge_mutex);
        } else if (ret == 0) {
            //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
            //sem_post(&semaphore2);  // Release the semaphore
            //pthread_mutex_lock(&bridge_mutex);
            while (sem_trywait(&semaphore1) == 0);
            while (sem_trywait(&semaphore3) == 0);
            while (sem_trywait(&semaphore2) == 0);
            //pthread_mutex_unlock(&bridge_mutex);
        }

        //pthread_mutex_lock(&crossroad_mutex); // Lock the mutex
        if (car_inside == 0) {
            current_dir = connector_direction;
        }
        if(car_inside) sleep_milli(PASS_DELAY);


        // Simulate passing through the crossroad
        //sleep_milli(PASS_DELAY/10);
        sem_getvalue(&semaphore4, &value);
        if(value==0){
            //cout<<"dir 1 "<<car_id<<endl;
            goto Label6;
        }
        pthread_mutex_unlock(&crossroad_mutex4);

        car_inside += 1;

        WriteOutput(car_id, 'C', crossroad_id, START_PASSING);   
        usleep(travel_time * 1000); // Simulate the time taken to pass the bridge
     
        WriteOutput(car_id, 'C', crossroad_id, FINISH_PASSING);
        car_inside -= 1;
        waiting4--;

        if (car_inside == 0) {
            current_dir = -1; // Reset current direction if the bridge is empty
            for (int i = 0; i < 100; ++i) {
                if(waiting1>0){
                    sem_post(&semaphore1);
                }
                else if(waiting2>0){
                    sem_post(&semaphore2);
                }
                else if(waiting3>0){
                    sem_post(&semaphore3);
                }
                else{
                    sem_post(&semaphore2);
                    sem_post(&semaphore3);
                    sem_post(&semaphore1);
                }

            }// Increment the semaphore value to allow passing in the opposite direction
        }
        //sem_post(&semaphore2);  // Notify finish passing the crossroad
    }

    void exitCrossRoad() {
        // No need to implement this method as it's not used in the semaphore-based synchronization
    }
};



class Ferry {
public:
    int id;
    int travel_time;
    int max_wait_time;
    int capacity;
    int capacity2;
    int current_dir;
    int car_inside;
    int limit;
    int limit2;
    sem_t semaphore2;
    sem_t semaphore1;    
    sem_t semaphore3;
    sem_t semaphore4;
    pthread_mutex_t ferry_mutex;
    pthread_mutex_t ferry_mutex2;

    Ferry() : id(0), travel_time(0), max_wait_time(0), current_dir(-1), car_inside(0), limit(900) , limit2(900) {
        pthread_mutex_init(&ferry_mutex, NULL);
        pthread_mutex_init(&ferry_mutex2, NULL);
        sem_init(&semaphore2, 0, 0); // Initialize the semaphore with a value of 1
        sem_init(&semaphore1, 0, 900); // Initialize the semaphore with a value of 1
        sem_init(&semaphore4, 0, 0); // Initialize the semaphore with a value of 1
        sem_init(&semaphore3, 0, 900); // Initialize the semaphore with a value of 1        
    } // Default constructor

    Ferry(int _id, int _travel_time, int _max_wait_time, int _capacity) : id(_id), travel_time(_travel_time), max_wait_time(_max_wait_time),capacity(_capacity), capacity2(_capacity), current_dir(-1), car_inside(0), limit(900) , limit2(900) {
        pthread_mutex_init(&ferry_mutex, NULL);
        pthread_mutex_init(&ferry_mutex2, NULL);
        sem_init(&semaphore2, 0, 0); // Initialize the semaphore with a value of 1
        sem_init(&semaphore1, 0, 900); // Initialize the semaphore with a value of 1
        sem_init(&semaphore4, 0, 0); // Initialize the semaphore with a value of 1
        sem_init(&semaphore3, 0, 900); // Initialize the semaphore with a value of 1  
    }

    void enterFerry(int car_id, int ferry_id, int arrival_timestamp, int connector_direction) {
        // Implement ferry entry logic here
        WriteOutput(car_id, 'F', ferry_id, TRAVEL); // Print AID: 0 traveling to connector
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }
        //std::cout<<"enterferryy girdii"<<endl;

        WriteOutput(car_id, 'F', ferry_id, ARRIVE);
        int value;
        pthread_mutex_lock(&ferry_mutex);
        //sem_wait(&semaphore1);
        //sem_getvalue(&semaphore1, &value);
        ++limit;
        value=limit;
        //pthread_mutex_unlock(&ferry_mutex);
        //cout<<"value for car id :" << car_id<< " is " << value<<endl;
        if(value%capacity == 0){
            //pthread_mutex_lock(&ferry_mutex);
            //cout<<"buraya girdii"<<endl;

            for(int a =0; a<capacity; a++){
                sem_post(&semaphore2); 

            }
            sem_wait(&semaphore2);
            pthread_mutex_unlock(&ferry_mutex);
        }
        else{
            //sem_wait(&semaphore2);
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 0;  // No need to add seconds
            ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

            // Check if the nanosecond field overflowed
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            pthread_mutex_unlock(&ferry_mutex);
            int ret = sem_timedwait(&semaphore2, &ts);
            if (ret == -1 && errno == ETIMEDOUT) {
                //std::cout << "Timeout occurred!\n"<< limit<<endl;
                pthread_mutex_lock(&ferry_mutex);
                while(limit%capacity!=0){
                //    cout<<"limitdöndü"<<endl;
                    limit--;
                    sem_post(&semaphore2);

                }
                sem_wait(&semaphore2);
                pthread_mutex_unlock(&ferry_mutex);
            } else if (ret == 0) {
                //std::cout << "Semaphore acquired successfully!\n"<<limit<<endl;
                //sem_post(&semaphore2);  // Release the semaphore
            }
            
            

            

            
        }
        WriteOutput(car_id, 'F', ferry_id, START_PASSING); // Start passing the connector
        usleep(travel_time * 1000);
        WriteOutput(car_id, 'F', ferry_id, FINISH_PASSING); 


    }

    void enterFerry2(int car_id, int ferry_id, int arrival_timestamp, int connector_direction) {
        // Implement ferry entry logic here
        WriteOutput(car_id, 'F', ferry_id, TRAVEL); // Print AID: 0 traveling to connector
        int sleep_duration = arrival_timestamp;
        if (sleep_duration > 0) {
            usleep(sleep_duration * 1000); // Wait until the arrival timestamp
        }

        WriteOutput(car_id, 'F', ferry_id, ARRIVE);
        int value;
        pthread_mutex_lock(&ferry_mutex2);
        ++limit2;
        value=limit2;
        //sem_wait(&semaphore3);
        //sem_getvalue(&semaphore3, &value);
        pthread_mutex_unlock(&ferry_mutex2);
        if(value%capacity2 == 0){
            pthread_mutex_lock(&ferry_mutex2);
            //cout<<"buraya girdii2"<<endl;
            for(int a =0; a<capacity2; a++){
                sem_post(&semaphore4); 

            }
            sem_wait(&semaphore4);
            pthread_mutex_unlock(&ferry_mutex2);
        }
        else{
            //sem_wait(&semaphore4);
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 0;  // No need to add seconds
            ts.tv_nsec += max_wait_time*1000000;  // Add 5 milliseconds (5,000,000 nanoseconds)

            // Check if the nanosecond field overflowed
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }            
            int ret = sem_timedwait(&semaphore4, &ts);
            if (ret == -1 && errno == ETIMEDOUT) {
                //std::cout << "Timeout occurred2!\n"<< max_wait_time<<endl;
                pthread_mutex_lock(&ferry_mutex2);
                while(limit2%capacity!=0){
                    limit2--;
                    sem_post(&semaphore4); 
                }
                sem_wait(&semaphore4);
                pthread_mutex_unlock(&ferry_mutex2);
            } else if (ret == 0) {
                //std::cout << "Semaphore acquired successfully! for car:\n"<<endl ;
                //sem_post(&semaphore4);  // Release the semaphore
            }
            
        }
        WriteOutput(car_id, 'F', ferry_id, START_PASSING); // Start passing the connector
        usleep(travel_time * 1000);
        WriteOutput(car_id, 'F', ferry_id, FINISH_PASSING); 


    }

    void exitFerry() {
        // Implement ferry exit logic here
    }
};




// Define the CarThreadData structure to pass as thread argument
struct CarBridgeThreadData {
    NarrowBridge* bridge;
    CrossRoad* crossroad;
    Ferry* ferry;

    CarDetails* car;
    int from_idx;
    int to_idx;
};
struct CarCrossRoadThreadData {
    CrossRoad* crossroad;
    CarDetails* car;
    int from_idx;
    int to_idx;
};
struct CarFerryThreadData {
    Ferry* ferry;
    CarDetails* car;
    int from_idx;
    int to_idx;
};

// Define the car_thread function to simulate car passing through narrow bridges
void* car_bridge_thread(void* arg) {
    CarBridgeThreadData* data = (CarBridgeThreadData*)arg;
    int current_timestamp = 0;
    //cout<< data->from_idx <<endl;
    for (int i = data->from_idx; i < data->to_idx; ++i) {
        std::string type= data->car->path[i].connector_type;
        if(type=="N"){
            int bridge_id = data->car->path[i].connector_id;
            //cout<< "bridge id: "<< data->car->path[i].connector_id <<endl;
            int direction = data->car->path[i].connector_direction;
            // Record the timestamp when the car arrives at the connector
            // Increment the current timestamp by the travel time of the car
            current_timestamp = data->car->travel_time;
            // Enter the bridge after arriving at the connector
            if(direction ==0) data->bridge[bridge_id].enterBridge(data->car->id, bridge_id, current_timestamp, direction);        // Exit the bridge after finishing passing
            if(direction ==1) data->bridge[bridge_id].enterBridge2(data->car->id, bridge_id, current_timestamp, direction);        // Exit the bridge after finishing passing
            data->bridge[bridge_id].exitBridge();
        }
        else if(type=="C"){
            int crossroad_id = data->car->path[i].connector_id;
            int direction = data->car->path[i].connector_direction;
            current_timestamp = data->car->travel_time;
            if(direction ==0) data->crossroad[crossroad_id].enterCrossRoad1(data->car->id, crossroad_id, current_timestamp, direction);
            if(direction ==1) data->crossroad[crossroad_id].enterCrossRoad2(data->car->id, crossroad_id, current_timestamp, direction);
            if(direction ==2) data->crossroad[crossroad_id].enterCrossRoad3(data->car->id, crossroad_id, current_timestamp, direction);
            if(direction ==3) data->crossroad[crossroad_id].enterCrossRoad4(data->car->id, crossroad_id, current_timestamp, direction);
            data->crossroad[crossroad_id].exitCrossRoad();
        }
        if(type=="F"){
            //std::cout<<"enterferryy thread girdii"<<endl;
            //std::cout << "Timeout occurred!\n"<<endl;

            int ferry_id = data->car->path[i].connector_id;
            //cout<< "bridge id: "<< data->car->path[i].connector_id <<endl;
            int direction = data->car->path[i].connector_direction;
            // Record the timestamp when the car arrives at the connector
            // Increment the current timestamp by the travel time of the car
            current_timestamp = data->car->travel_time;
            // Enter the bridge after arriving at the connector
            if(direction ==0) data->ferry[ferry_id].enterFerry(data->car->id, ferry_id, current_timestamp, direction);        // Exit the bridge after finishing passing
            if(direction ==1) data->ferry[ferry_id].enterFerry2(data->car->id, ferry_id, current_timestamp, direction);        // Exit the bridge after finishing passing
            data->ferry[ferry_id].exitFerry();
        }
    }
    return NULL;
}

void* car_crossroad_thread(void* arg) {
    CarCrossRoadThreadData* data = (CarCrossRoadThreadData*)arg;
    int current_timestamp = 0;
    for (int i = data->from_idx; i < data->to_idx; ++i) {
        int crossroad_id = data->car->path[i].connector_id;
        int direction = data->car->path[i].connector_direction;
        current_timestamp = data->car->travel_time;
        if(direction ==0) data->crossroad[crossroad_id].enterCrossRoad1(data->car->id, crossroad_id, current_timestamp, direction);
        if(direction ==1) data->crossroad[crossroad_id].enterCrossRoad2(data->car->id, crossroad_id, current_timestamp, direction);
        if(direction ==2) data->crossroad[crossroad_id].enterCrossRoad3(data->car->id, crossroad_id, current_timestamp, direction);
        if(direction ==3) data->crossroad[crossroad_id].enterCrossRoad4(data->car->id, crossroad_id, current_timestamp, direction);
        data->crossroad[crossroad_id].exitCrossRoad();
    }
    return NULL;
}

void* car_ferry_thread(void* arg) {
    CarFerryThreadData* data = (CarFerryThreadData*)arg;
    int current_timestamp = 0;
    //cout<< data->from_idx <<endl;
    for (int i = data->from_idx; i < data->to_idx; ++i) {

        int ferry_id = data->car->path[i].connector_id;
        //cout<< "bridge id: "<< data->car->path[i].connector_id <<endl;
        int direction = data->car->path[i].connector_direction;
        // Record the timestamp when the car arrives at the connector
        // Increment the current timestamp by the travel time of the car
        current_timestamp = data->car->travel_time;
        // Enter the bridge after arriving at the connector
        if(direction ==0) data->ferry[ferry_id].enterFerry(data->car->id, ferry_id, current_timestamp, direction);        // Exit the bridge after finishing passing
        if(direction ==1) data->ferry[ferry_id].enterFerry2(data->car->id, ferry_id, current_timestamp, direction);        // Exit the bridge after finishing passing
        data->ferry[ferry_id].exitFerry();
    }
    return NULL;
}

int main() {
    // Initialize WriteOutput
    InitWriteOutput();
    //std::ifstream std::cin("input7.txt");
    //if (!std::cin) {
    //    std::cerr << "Error: Unable to open input file." << std::endl;
    //    return 1;
    //}
    pthread_mutex_t mutexx;
    pthread_mutex_init(&mutexx, NULL);


    // Read narrow bridges information
    int num_bridges;
    std::cin >> num_bridges;
    std::vector<NarrowBridge> bridges(num_bridges);
    for (int i = 0; i < num_bridges; ++i) {
        int travel_time, max_wait_time;
        std::cin >> travel_time >> max_wait_time;
        bridges[i] = NarrowBridge(i, travel_time, max_wait_time);
    }

    int num_ferries;
    std::cin >> num_ferries;
    std::vector<Ferry> ferries(num_ferries);
    for (int i = 0; i < num_ferries; ++i) {
        int travel_time, max_wait_time, capacity;
        std::cin >> travel_time >> max_wait_time>>capacity;
        ferries[i] = Ferry(i, travel_time, max_wait_time, capacity);
    }

    int num_crossroads;
    std::cin >> num_crossroads;
    std::vector<CrossRoad> crossroads(num_crossroads);
    for (int i = 0; i < num_crossroads; ++i) {
        int travel_time2, max_wait_time2;
        std::cin >> travel_time2 >> max_wait_time2;
        crossroads[i] = CrossRoad(i, travel_time2, max_wait_time2);
    }
    // Read cars information
    int num_cars;
    //std::cin >> num_cars;
    //std::cin >> num_cars;
    std::cin >> num_cars;
    std::vector<CarDetails> cars(num_cars);
    //cout<<"num cars: "<<num_cars<<endl;
    for (int i = 0; i < num_cars; ++i) {
        int x;
        int travel_time, path_length;
        std::cin >> travel_time >> path_length;
        cars[i].id = i;
        cars[i].travel_time = travel_time;
        cars[i].path.resize(path_length);
        for (int j = 0; j < path_length; ++j) {
            std::string connector_type;
            int connector_direction;

            std::cin >> connector_type >> connector_direction>>x;
            //cout << connector_type << "and" << connector_direction <<endl;
            cars[i].path[j].connector_id = stoi(connector_type.substr(1,1));
            cars[i].path[j].connector_type = connector_type.substr(0,1);
            cars[i].path[j].connector_direction = connector_direction;
        }
    }

    // Close the input file
    //std::cin.close();

    // Simulate car passing through narrow bridges
    pthread_t bridge_threads[num_cars*4];
    pthread_t crossroad_threads[num_cars*4];
    pthread_t ferry_threads[num_cars*4];
    CarBridgeThreadData bridge_thread_data[num_cars*4];
    CarCrossRoadThreadData crossroad_thread_data[num_cars*4];
    CarFerryThreadData ferry_thread_data[num_cars*4];

    int bridge_thread_count = 0;
    int bridge_thread_count2 = 0;
    int crossroad_thread_count = 0;
    int ferry_thread_count = 0;

    for (int i = 0; i < num_cars; ++i) {
        int from_idx = 0;
        int to_idx = cars[i].path.size();
        if (cars[i].path[0].connector_type=="N") {
            //cout<<"Bridge girdiii"<<endl;

            bridge_thread_data[bridge_thread_count].bridge = bridges.data();
            bridge_thread_data[bridge_thread_count].crossroad = crossroads.data();
            bridge_thread_data[bridge_thread_count].ferry = ferries.data();
            bridge_thread_data[bridge_thread_count].car = &cars[i];
            bridge_thread_data[bridge_thread_count].from_idx = from_idx;
            bridge_thread_data[bridge_thread_count].to_idx = to_idx;
            pthread_create(&bridge_threads[bridge_thread_count], NULL, car_bridge_thread, &bridge_thread_data[bridge_thread_count]);
            ++bridge_thread_count;
            ++bridge_thread_count2;
        } else if (cars[i].path[0].connector_type=="C") {
            //cout<<"Crossa girdiii"<<endl;
            bridge_thread_data[bridge_thread_count].bridge = bridges.data();
            bridge_thread_data[bridge_thread_count].crossroad = crossroads.data();
            bridge_thread_data[bridge_thread_count].ferry = ferries.data();            
            bridge_thread_data[bridge_thread_count].car = &cars[i];
            bridge_thread_data[bridge_thread_count].from_idx = from_idx;
            bridge_thread_data[bridge_thread_count].to_idx = to_idx;
            pthread_create(&crossroad_threads[bridge_thread_count], NULL, car_bridge_thread, &bridge_thread_data[bridge_thread_count]);
            ++bridge_thread_count;
            ++crossroad_thread_count;

        } else if (cars[i].path[0].connector_type == "F") {
            //cout<<"ferryy girdiii"<<endl;
            bridge_thread_data[bridge_thread_count].bridge = bridges.data();
            bridge_thread_data[bridge_thread_count].crossroad = crossroads.data();
            bridge_thread_data[bridge_thread_count].ferry = ferries.data();            
            bridge_thread_data[bridge_thread_count].car = &cars[i];
            bridge_thread_data[bridge_thread_count].from_idx = from_idx;
            bridge_thread_data[bridge_thread_count].to_idx = to_idx;
            pthread_create(&ferry_threads[bridge_thread_count], NULL, car_bridge_thread, &bridge_thread_data[bridge_thread_count]);
            ++bridge_thread_count;
            ++ferry_thread_count;

        }
    }
    // Wait for all threads to finish
    //pthread_mutex_lock(&mutexx); // Lock the mutex

    //cout<< "b2: "<<bridge_thread_count2 <<endl;
    //cout<< "b3: "<<bridge_thread_count <<endl;
       

    //pthread_mutex_unlock(&mutexx); // Unlock the mutex
    //pthread_join(bridge_threads[0], NULL);
    //pthread_join(bridge_threads[1], NULL);
    //pthread_join(bridge_threads[2], NULL);
    //pthread_join(bridge_threads[3], NULL);
    //sleep_milli(100*PASS_DELAY);
    for (int i = 0; i < bridge_thread_count2; ++i) {
        //cout<< "ZZZZW" <<endl;

        pthread_join(bridge_threads[i], NULL);
    }

    for (int i = 0; i < ferry_thread_count; ++i) {
        pthread_join(ferry_threads[i], NULL);
    }

    // Wait for all crossroad threads to finish
    for (int i = 0; i < crossroad_thread_count; ++i) {
        pthread_join(crossroad_threads[i], NULL);
    }

    return 0;
}
