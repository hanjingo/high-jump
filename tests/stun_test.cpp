#include <gtest/gtest.h>
#include <libcpp/net/stun.hpp>
// #include <string>
// #include <vector>
// #include <iostream>

// // NOTE: Requires network access and a public STUN server (e.g., stun.l.google.com:19302)

// TEST(StunTest, GetLocalCandidates) {
//     libcpp::stun_client client("stun.l.google.com", 19302);
//     auto candidates = client.get_local_candidates();
//     ASSERT_FALSE(candidates.empty());
//     for (const auto& cand : candidates) {
//         std::cout << "Candidate: " << cand.first << ":" << cand.second << std::endl;
//         EXPECT_FALSE(cand.first.empty());
//         EXPECT_GT(cand.second, 0);
//     }
// }

