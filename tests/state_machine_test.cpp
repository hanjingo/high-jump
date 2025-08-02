#include <gtest/gtest.h>
#include <libcpp/algo/state_machine.hpp>

// using namespace libcpp;

// TEST(StateMachineTest, InitialState) {
//     SimpleStateMachine sm;
//     sm.start();
//     // Should be in Idle state initially
//     EXPECT_TRUE(sm.is_flag_active<Idle>());
// }

// TEST(StateMachineTest, TransitionToRunning) {
//     SimpleStateMachine sm;
//     sm.start();
//     sm.process_event(EventStart{});
//     EXPECT_TRUE(sm.is_flag_active<Running>());
// }

// TEST(StateMachineTest, TransitionBackToIdle) {
//     SimpleStateMachine sm;
//     sm.start();
//     sm.process_event(EventStart{});
//     sm.process_event(EventStop{});
//     EXPECT_TRUE(sm.is_flag_active<Idle>());
// }