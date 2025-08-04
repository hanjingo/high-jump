#ifndef STUN_HPP
#define STUN_HPP

// // STUN (Session Traversal Utilities for NAT) wrapper for libnice
// // Requires libnice and glib

// #include <nice/agent.h>
// #include <string>
// #include <vector>
// #include <stdexcept>
// #include <memory>

// namespace libcpp
// {

// // Simple wrapper for STUN discovery using libnice
// class stun_client {
// public:
//     // Construct with STUN server address and port
//     stun_client(const std::string& stun_server, uint16_t stun_port)
//         : stun_server_(stun_server), stun_port_(stun_port)
//     {
//         // Initialize GMainLoop and NiceAgent
//         loop_ = g_main_loop_new(nullptr, FALSE);
//         agent_ = nice_agent_new(g_main_loop_get_context(loop_),
//         NICE_COMPATIBILITY_RFC5245); if (!agent_)
//             throw std::runtime_error("Failed to create NiceAgent");

//         // Set STUN server
//         g_object_set(G_OBJECT(agent_), "stun-server", stun_server_.c_str(),
//         nullptr); g_object_set(G_OBJECT(agent_), "stun-server-port",
//         stun_port_, nullptr);

//         // Add a stream (1 component)
//         stream_id_ = nice_agent_add_stream(agent_, 1);
//         if (stream_id_ == 0)
//             throw std::runtime_error("Failed to add stream to NiceAgent");

//         // Attach to main context (required for signals)
//         nice_agent_attach_recv(agent_, stream_id_, 1,
//         g_main_loop_get_context(loop_), nullptr, nullptr);
//     }

//     ~stun_client() {
//         if (agent_) g_object_unref(agent_);
//         if (loop_) g_main_loop_unref(loop_);
//     }

//     // Get local candidate (discovers public IP/port via STUN)
//     // Returns a vector of (ip, port) pairs
//     std::vector<std::pair<std::string, uint16_t>> get_local_candidates() {
//         std::vector<std::pair<std::string, uint16_t>> result;

//         // Gather candidates
//         nice_agent_gather_candidates(agent_, stream_id_);

//         // Wait for gathering-done signal (simple polling for demo)
//         int max_wait = 100; // 10s max
//         while (max_wait-- > 0) {
//             g_main_context_iteration(g_main_loop_get_context(loop_), FALSE);
//             GSList* lcands = nice_agent_get_local_candidates(agent_,
//             stream_id_, 1); if (lcands) {
//                 for (GSList* l = lcands; l; l = l->next) {
//                     NiceCandidate* cand = (NiceCandidate*)l->data;
//                     if (cand->type == NICE_CANDIDATE_TYPE_SERVER_REFLEXIVE ||
//                         cand->type == NICE_CANDIDATE_TYPE_HOST) {
//                         char ip[INET6_ADDRSTRLEN] = {0};
//                         nice_address_to_string(&cand->addr, ip);
//                         uint16_t port = nice_address_get_port(&cand->addr);
//                         result.emplace_back(ip, port);
//                     }
//                 }
//                 nice_candidate_free(lcands->data);
//                 g_slist_free(lcands);
//                 break;
//             }
//             g_usleep(100000); // 100ms
//         }
//         return result;
//     }

// private:
//     std::string stun_server_;
//     uint16_t stun_port_;
//     NiceAgent* agent_ = nullptr;
//     GMainLoop* loop_ = nullptr;
//     guint stream_id_ = 0;
// };

// } // namespace libcpp

#endif // STUN_HPP