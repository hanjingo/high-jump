#include <gtest/gtest.h>
#include <libcpp/algo/state_machine.hpp>
#include <string>

// Events
struct StartEvent {};
struct PauseEvent {};
struct ResumeEvent {};
struct StopEvent {};
struct ResetEvent {};

// States
struct IdleState {};
struct RunningState {};
struct PausedState {};
struct StoppedState {};

// Guards
auto is_valid = [](const auto& event) { return true; };

// Actions
auto on_start = []() { /* Start action */ };
auto on_pause = []() { /* Pause action */ };
auto on_resume = []() { /* Resume action */ };
auto on_stop = []() { /* Stop action */ };
auto on_reset = []() { /* Reset action */ };

// State machine definition
auto state_machine = []() {
    using namespace libcpp::sml;
    
    return make_transition_table(
        *state<IdleState> + event<StartEvent> / on_start = state<RunningState>,
        state<RunningState> + event<PauseEvent> / on_pause = state<PausedState>,
        state<PausedState> + event<ResumeEvent> / on_resume = state<RunningState>,
        state<RunningState> + event<StopEvent> / on_stop = state<StoppedState>,
        state<PausedState> + event<StopEvent> / on_stop = state<StoppedState>,
        state<StoppedState> + event<ResetEvent> / on_reset = state<IdleState>
    );
};

class StateMachineTest : public ::testing::Test {
protected:
    void SetUp() override {
        sm = std::make_unique<libcpp::sml::sm<decltype(state_machine)>>(state_machine);
    }

    std::unique_ptr<libcpp::sml::sm<decltype(state_machine)>> sm;
};

TEST_F(StateMachineTest, InitialState) {
    EXPECT_TRUE(sm->is(libcpp::sml::state<IdleState>));
}

TEST_F(StateMachineTest, StartTransition) {
    sm->process_event(StartEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<RunningState>));
}

TEST_F(StateMachineTest, PauseTransition) {
    sm->process_event(StartEvent{});
    sm->process_event(PauseEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<PausedState>));
}

TEST_F(StateMachineTest, ResumeTransition) {
    sm->process_event(StartEvent{});
    sm->process_event(PauseEvent{});
    sm->process_event(ResumeEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<RunningState>));
}

TEST_F(StateMachineTest, StopFromRunning) {
    sm->process_event(StartEvent{});
    sm->process_event(StopEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<StoppedState>));
}

TEST_F(StateMachineTest, StopFromPaused) {
    sm->process_event(StartEvent{});
    sm->process_event(PauseEvent{});
    sm->process_event(StopEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<StoppedState>));
}

TEST_F(StateMachineTest, ResetTransition) {
    sm->process_event(StartEvent{});
    sm->process_event(StopEvent{});
    sm->process_event(ResetEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<IdleState>));
}

TEST_F(StateMachineTest, CompleteWorkflow) {
    // Start -> Pause -> Resume -> Stop -> Reset
    sm->process_event(StartEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<RunningState>));
    
    sm->process_event(PauseEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<PausedState>));
    
    sm->process_event(ResumeEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<RunningState>));
    
    sm->process_event(StopEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<StoppedState>));
    
    sm->process_event(ResetEvent{});
    EXPECT_TRUE(sm->is(libcpp::sml::state<IdleState>));
}