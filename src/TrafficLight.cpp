#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // DONE: FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    //       to wait for and receive new messages and pull them from the queue using move semantics.
    //       The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    _conditionVariable.wait(uniqueLock, [this] { return !_messages.empty(); } );

    T message = std::move(_messages.back());
    _messages.pop_back();

    return message; // no copy due to Return Value Optimization
}

template <typename T>
void MessageQueue<T>::send(T &&message)
{
    // DONE: FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    //       as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lockGuard(_mutex);

    _messages.push_back(std::move(message));
    _conditionVariable.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // DONE: FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    //       runs and repeatedly calls the receive function on the message queue.
    //      Once it receives TrafficLightPhase::green, the method returns.

    while (true)
    {
        TrafficLightPhase trafficLight = _lightPhaseQueue.receive();
        if (trafficLight == TrafficLightPhase::green)
        {
          return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // DONE: FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // DONE: FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    //       and toggles the current phase of the traffic light between red and green and sends an update method
    //      to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    //      Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::random_device random;  //Will be used to obtain a seed for the random number engine
    std::mt19937 generate(random()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> distribution(4.0, 6.0);
    double cycleDuration = distribution(generate);

    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // TODO: can this be simpler in place of 8 lines? without making single line if statements
            //       maybe a boolean scoped enum? with operator! overloaded?
            {
                std::lock_guard<std::mutex> lck(_mutex);
                if (_currentPhase == TrafficLightPhase::red)
                {
                    _currentPhase = TrafficLightPhase::green;
                }
                else if (_currentPhase == TrafficLightPhase::green)
                {
                    _currentPhase = TrafficLightPhase::red;
                }
            }

            _lightPhaseQueue.send(std::move(getCurrentPhase()));

            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = distribution(generate);
        }
    }
}




