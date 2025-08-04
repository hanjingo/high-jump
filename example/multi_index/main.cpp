#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include <libcpp/algo/multi_index.hpp>

class student
{
  public:
    student(int id, std::string name, int score)
        : id(id), name(name), score(score)
    {
    }

    void print() const
    {
        std::cout << "id:" << id << " name:" << name << " score:" << score
                  << std::endl;
    }

    int id;
    std::string name;
    int score;
};

int main(int argc, char* argv[])
{
    // struct _id{};
    // struct _name{};
    // struct _score{};

    // libcpp::multi_index<
    //     student,
    //     libcpp::unique_index<int, &student::id, _id>,
    //     libcpp::nonunique_index<std::string, &student::name, _name>,
    //     libcpp::nonunique_index<int, &student::score, _score>
    // > student_table;

    LIBCPP_INDEX_TAGS_3(_id, _name, _score)
    libcpp::multi_index<
        student,
        libcpp::unique_index<int, &student::id, _id>,
        libcpp::nonunique_index<std::string, &student::name, _name>,
        libcpp::nonunique_index<int, &student::score, _score> >
        student_table;

    student_table.insert(student(1, "Alice", 85));
    student_table.insert(student(2, "Bob", 92));
    student_table.insert(student(3, "Charlie", 85));
    student_table.insert(student(4, "Alice", 78));

    std::cout << "\nprint all" << std::endl;
    for (const auto& s : student_table)
    {
        std::cout << "ID: " << s.id << ", Name: " << s.name
                  << ", Score: " << s.score << std::endl;
    }

    std::cout << "\nfind by id" << std::endl;
    auto& id_index = student_table.get<_id>();
    auto it_id = id_index.find(2);
    if (it_id != id_index.end())
    {
        std::cout << "ID=2: " << it_id->name << ", Score: " << it_id->score
                  << std::endl;
    }

    std::cout << "\nfind by name" << std::endl;
    auto& name_index = student_table.get<_name>();
    auto range = name_index.equal_range("Alice");
    for (auto it = range.first; it != range.second; ++it)
    {
        std::cout << "Alice: ID=" << it->id << ", Score=" << it->score
                  << std::endl;
    }

    std::cout << "\nfind by score" << std::endl;
    auto& score_index = student_table.get<_score>();
    auto score_range = score_index.equal_range(85);
    for (auto it = score_range.first; it != score_range.second; ++it)
    {
        std::cout << "Score == 85: ID=" << it->id << ", Name: " << it->name
                  << std::endl;
    }

    std::cout << "\nfind by score range" << std::endl;
    auto lower = score_index.lower_bound(85);
    for (auto it = lower; it != score_index.end(); ++it)
    {
        std::cout << "score >= 85:" << it->name << " (" << it->score << ")"
                  << std::endl;
    }

    std::cout << "\nalter score" << std::endl;
    auto bob_it = id_index.find(2);
    if (bob_it != id_index.end())
    {
        id_index.modify(bob_it, [](student& s) { s.score = 95; });
        std::cout << "new score:" << bob_it->score << std::endl;
    }

    std::cout << "\ndelete by id" << std::endl;
    auto charlie_it = id_index.find(3);
    if (charlie_it != id_index.end())
    {
        id_index.erase(charlie_it);
        std::cout << "deleted Charlie" << std::endl;
    }

    std::cout << "\nprint all" << std::endl;
    for (const auto& s : student_table)
    {
        std::cout << "ID: " << s.id << ", Name: " << s.name
                  << ", Score: " << s.score << std::endl;
    }

    std::cin.get();
    return 0;
}