#ifndef YAML_HPP
#define YAML_HPP

// #include <string>
// #include <sstream>
// #include <yaml-cpp/yaml.h>
// #include <fstream>

// namespace libcpp
// {

// class yaml : public YAML::Node
// {
// public:
//     yaml() : YAML::Node() {}
//     yaml(const yaml& rhs) : YAML::Node(rhs) {}
//     yaml(const YAML::Node& node) : YAML::Node(node) {}
//     ~yaml() {}

//     yaml& operator=(const yaml& rhs)
//     {
//         YAML::Node::operator=(rhs);
//         return *this;
//     }

//     yaml& operator=(const YAML::Node& rhs)
//     {
//         YAML::Node::operator=(rhs);
//         return *this;
//     }

//     // Parse YAML from string
//     static yaml parse(const char* text)
//     {
//         std::stringstream ss(text);
//         YAML::Node node = YAML::Load(ss);
//         return yaml(node);
//     }

//     // Read YAML from file
//     bool read_file(const char* filepath)
//     {
//         std::ifstream fin(filepath);
//         if (!fin.is_open())
//             return false;
//         YAML::Node node = YAML::Load(fin);
//         *this = node;
//         return true;
//     }

//     // Write YAML to file
//     void write_file(const char* filepath)
//     {
//         std::ofstream fout(filepath);
//         fout << YAML::Dump(*this);
//     }

//     // Serialize YAML to string
//     std::string str()
//     {
//         return YAML::Dump(*this);
//     }
// };

// }

#endif  // YAML_HPP