#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <string>
#include <iostream>

namespace libcpp
{

// Example events
struct EventStart {};
struct EventStop {};

// Example states
struct Idle : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { /* Entry action for Idle */ }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { /* Exit action for Idle */ }
};

struct Running : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { /* Entry action for Running */ }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { /* Exit action for Running */ }
};

// State machine front-end
struct SimpleStateMachine_ : public boost::msm::front::state_machine_def<SimpleStateMachine_>
{
    // Set initial state
    typedef Idle initial_state;

    // Transition table
    struct transition_table : boost::mpl::vector<
        //    Start     Event        Next      Action      Guard
        boost::msm::front::Row < Idle,    EventStart, Running,  boost::msm::front::none, boost::msm::front::none >,
        boost::msm::front::Row < Running, EventStop,  Idle,     boost::msm::front::none, boost::msm::front::none >
    > {};

    // Optional: actions and guards can be added here
};

// Back-end: state machine type
using SimpleStateMachine = boost::msm::back::state_machine<SimpleStateMachine_>;

} // namespace libcpp

#endif // STATE_MACHINE_HPP