#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mutex); 
    _cond.wait(lck, [this] {return !_queue.empty(); }); 

    T msg = std::move(_queue.front()); 
    _queue.pop_front(); 

    return msg; // return value optimization will use move semantics here
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex); // locks mutex 
    _queue.push_back(std::move(msg)); 
    _cond.notify_one(); 
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        TrafficLightPhase phase = _messageQueue.receive(); 
        if (phase == TrafficLightPhase::green) 
            return; 
    }

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    long cycleDurationMin = 4.0 * 1000;  // in ms 
    long cycleDurationMax = 6.0 * 1000; 
    long currCycleDuration = getRandomCycleTime(cycleDurationMin, cycleDurationMax); 

    // stop watch 
    std::chrono::time_point<std::chrono::system_clock> lastUpdate; 
    lastUpdate = std::chrono::system_clock::now();

    // ininite while loop 
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 

        long timeSinceLastUpdate =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if ( timeSinceLastUpdate >= currCycleDuration ) {
            // toggle phase
            _currentPhase = (_currentPhase == TrafficLightPhase::red ) ? TrafficLightPhase::green : TrafficLightPhase::red; 
            // pass update to message queue 
            TrafficLightPhase phaseMessage = _currentPhase; 
            _messageQueue.send(std::move(phaseMessage)); 
            // update time and cycleDuration
            lastUpdate = std::chrono::system_clock::now(); 
            currCycleDuration = getRandomCycleTime(cycleDurationMin, cycleDurationMax); 
            
        }
    }

}

// private helper function to get a random cycle time in between given bounds 
long TrafficLight::getRandomCycleTime(long minCycleTime, long maxCycleTime){
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<long> unif(minCycleTime, maxCycleTime); 
    long randomValue = unif(eng); 
    return randomValue; 
}
