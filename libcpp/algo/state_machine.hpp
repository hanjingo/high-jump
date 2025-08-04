#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

// #include <boost/sml.hpp>
// #include <string>
// #include <iostream>
// #include <cassert>

// namespace libcpp
// {
// namespace sml = boost::sml;

// namespace event
// {
// // Basic control events
// struct start_event {};
// struct stop_event {};
// struct pause_event {};
// struct resume_event {};
// struct reset_event {};
// } // namespace event

// namespace state
// {
// // State definitions using SML's state syntax
// struct idle_state {};
// struct running_state {};
// struct paused_state {};
// struct error_state {};
// struct processing_state {};
// } // namespace state


// // Data events with payload
// struct data_received
// {
//     std::string data;
//     explicit data_received(const std::string& d) : data(d) {}
// };

// struct error_occurred
// {
//     int error_code;
//     std::string message;
//     error_occurred(int code, const std::string& msg) : error_code(code),
//     message(msg) {}
// };

// struct timeout_event
// {
//     int duration_ms;
//     explicit timeout_event(int ms) : duration_ms(ms) {}
// };


// // Guard functions to control transitions
// struct guards
// {
//     // Check if data is valid
//     static bool is_valid_data(const data_received& event) {return
//     !event.data.empty() && event.data.length() > 0;}

//     // Check if error is recoverable
//     static bool is_recoverable_error(const error_occurred& event) { return
//     event.error_code < 1000; }

//     // Check if timeout is acceptable
//     static bool is_timeout_acceptable(const timeout_event& event) { return
//     event.duration_ms < 5000; }
// };

// // ================================ Actions ================================

// // Action functions executed during transitions
// struct actions
// {
//     // Logging actions
//     auto log_start = []() {
//         std::cout << "[ACTION] Starting system..." << std::endl;
//     };

//     auto log_stop = []() {
//         std::cout << "[ACTION] Stopping system..." << std::endl;
//     };

//     auto log_pause = []() {
//         std::cout << "[ACTION] Pausing system..." << std::endl;
//     };

//     auto log_resume = []() {
//         std::cout << "[ACTION] Resuming system..." << std::endl;
//     };

//     auto log_reset = []() {
//         std::cout << "[ACTION] Resetting system..." << std::endl;
//     };

//     // Data processing actions
//     auto process_data = [](const data_received& event) {
//         std::cout << "[ACTION] Processing data: " << event.data << std::endl;
//     };

//     auto handle_error = [](const error_occurred& event) {
//         std::cout << "[ACTION] Handling error " << event.error_code
//                   << ": " << event.message << std::endl;
//     };

//     auto handle_timeout = [](const timeout_event& event) {
//         std::cout << "[ACTION] Handling timeout: " << event.duration_ms <<
//         "ms" << std::endl;
//     };

//     // State entry/exit actions
//     auto on_idle_entry = []() {
//         std::cout << "[STATE] Entered idle state" << std::endl;
//     };

//     auto on_running_entry = []() {
//         std::cout << "[STATE] Entered running state" << std::endl;
//     };

//     auto on_paused_entry = []() {
//         std::cout << "[STATE] Entered paused state" << std::endl;
//     };

//     auto on_error_entry = []() {
//         std::cout << "[STATE] Entered error state" << std::endl;
//     };

//     auto on_processing_entry = []() {
//         std::cout << "[STATE] Entered processing state" << std::endl;
//     };
// };

// // ================================ Simple State Machine
// ================================

// // Basic state machine with minimal functionality
// struct simple_state_machine {
//     auto operator()() const {
//         using namespace sml;

//         const auto log_transition = [](auto event, auto from, auto to) {
//             std::cout << "[TRANSITION] " << typeid(from).name()
//                      << " -> " << typeid(to).name()
//                      << " on " << typeid(event).name() << std::endl;
//         };

//         return make_transition_table(
//             // Source State       + Event         = Target State      /
//             Action *idle_state{}         + event<start_event>    =
//             running_state{}   / actions{}.log_start,
//              running_state{}      + event<stop_event>     = idle_state{} /
//              actions{}.log_stop, running_state{}      + event<pause_event> =
//              paused_state{}    / actions{}.log_pause, paused_state{}       +
//              event<resume_event>   = running_state{}   /
//              actions{}.log_resume, X                    + event<reset_event>
//              = idle_state{}      / actions{}.log_reset
//         );
//     }
// };

// // ================================ Advanced State Machine
// ================================

// // More complex state machine with guards, actions, and data processing
// struct advanced_state_machine {
//     auto operator()() const {
//         using namespace sml;

//         return make_transition_table(
//             // Initial state
//             *idle_state{} + sml::on_entry<_> / actions{}.on_idle_entry,

//             // Basic control transitions
//             idle_state{}      + event<start_event> = running_state{}      /
//             actions{}.log_start, running_state{}   + sml::on_entry<_> /
//             actions{}.on_running_entry, running_state{}   + event<stop_event>
//             = idle_state{}         / actions{}.log_stop, running_state{}   +
//             event<pause_event>                           = paused_state{} /
//             actions{}.log_pause,

//             // Paused state handling
//             paused_state{}    + sml::on_entry<_> / actions{}.on_paused_entry,
//             paused_state{}    + event<resume_event> = running_state{}      /
//             actions{}.log_resume, paused_state{}    + event<stop_event> =
//             idle_state{}         / actions{}.log_stop,

//             // Data processing transitions with guards
//             running_state{}   + event<data_received> [guards{}.is_valid_data]
//             = processing_state{}   / actions{}.process_data,
//             processing_state{} + sml::on_entry<_> /
//             actions{}.on_processing_entry, processing_state{} +
//             event<start_event>                           = running_state{},
//             // Processing complete

//             // Error handling with conditional recovery
//             X + event<error_occurred> [guards{}.is_recoverable_error] =
//             running_state{}      / actions{}.handle_error, X +
//             event<error_occurred> [!guards{}.is_recoverable_error]        =
//             error_state{}        / actions{}.handle_error,

//             // Error state handling
//             error_state{}     + sml::on_entry<_> / actions{}.on_error_entry,
//             error_state{}     + event<reset_event> = idle_state{}         /
//             actions{}.log_reset,

//             // Timeout handling
//             X + event<timeout_event> [guards{}.is_timeout_acceptable] /
//             actions{}.handle_timeout, X + event<timeout_event>
//             [!guards{}.is_timeout_acceptable]        = error_state{}        /
//             actions{}.handle_timeout,

//             // Global reset from any state
//             X + event<reset_event> = idle_state{}         /
//             actions{}.log_reset
//         );
//     }
// };

// // ================================ State Machine Factory
// ================================

// // Factory class to create and manage state machines
// template<typename StateMachineDefinition>
// class state_machine_factory {
// public:
//     using sm_type = sml::sm<StateMachineDefinition>;

//     // Create a new state machine instance
//     static sm_type create() {
//         return sm_type{};
//     }

//     // Create state machine with custom logger
//     template<typename Logger>
//     static sm_type create_with_logger(Logger&& logger) {
//         return sm_type{std::forward<Logger>(logger)};
//     }
// };

// // ================================ State Machine Utilities
// ================================

// // Utility functions for state machine introspection and control
// struct state_machine_utils
// {
//     // Check if state machine is in a specific state
//     template<typename SM, typename State>
//     static bool is_in_state(const SM& sm) {
//         return sm.template is<State>();
//     }

//     // Get current state name as string
//     template<typename SM>
//     static std::string get_current_state_name(const SM& sm) {
//         std::string state_name = "unknown";

//         sm.visit_current_states([&state_name](auto state) {
//             state_name = typeid(state).name();
//         });

//         return state_name;
//     }

//     // Process multiple events safely
//     template<typename SM, typename... Events>
//     static void process_events(SM& sm, Events&&... events) {
//         (sm.process_event(std::forward<Events>(events)), ...);
//     }

//     // Check if event can be processed in current state
//     template<typename SM, typename Event>
//     static bool can_process_event(const SM& sm, const Event& event) {
//         // Note: This is a simplified check, SML doesn't provide direct API
//         for this
//         // In practice, you might need to maintain this information
//         separately return true; // Placeholder implementation
//     }
// };

// // ================================ Type Aliases
// ================================

// // Convenient type aliases for common use cases
// using simple_sm = sml::sm<simple_state_machine>;
// using advanced_sm = sml::sm<advanced_state_machine>;

// // Factory aliases
// using simple_factory = state_machine_factory<simple_state_machine>;
// using advanced_factory = state_machine_factory<advanced_state_machine>;

// // ================================ Example Usage Helper
// ================================

// // Helper class to demonstrate usage patterns
// class state_machine_example {
// public:
//     static void demonstrate_simple_usage() {
//         std::cout << "\n=== Simple State Machine Demo ===\n";

//         auto sm = simple_factory::create();

//         std::cout << "Initial state: " <<
//         state_machine_utils::get_current_state_name(sm) << std::endl;

//         sm.process_event(start_event{});
//         sm.process_event(pause_event{});
//         sm.process_event(resume_event{});
//         sm.process_event(stop_event{});
//         sm.process_event(reset_event{});
//     }

//     static void demonstrate_advanced_usage() {
//         std::cout << "\n=== Advanced State Machine Demo ===\n";

//         auto sm = advanced_factory::create();

//         std::cout << "Initial state: " <<
//         state_machine_utils::get_current_state_name(sm) << std::endl;

//         // Normal flow
//         sm.process_event(start_event{});
//         sm.process_event(data_received{"test_data"});
//         sm.process_event(start_event{}); // Complete processing

//         // Error scenarios
//         sm.process_event(error_occurred{500, "Recoverable error"});
//         sm.process_event(error_occurred{1500, "Non-recoverable error"});
//         sm.process_event(reset_event{});

//         // Timeout scenarios
//         sm.process_event(start_event{});
//         sm.process_event(timeout_event{3000}); // Acceptable timeout
//         sm.process_event(timeout_event{10000}); // Unacceptable timeout
//         sm.process_event(reset_event{});
//     }
// };

// } // namespace libcpp

#endif  // STATE_MACHINE_HPP