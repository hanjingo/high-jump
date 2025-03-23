#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <libcpp/encoding/flatbuffers.hpp>
#include "monster_generated.h"

TEST(flatbuffers, flatbuffers)
{
    libcpp::flatbuffer_builder builder;
    auto monster = MyGame::CreateMonster(
        builder, 1, builder.CreateString("Dragon"), 100, 35.5f);
    builder.Finish(monster);
    uint8_t *buf = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    std::cout << "size=" << size << std::endl;

    auto dmonster = MyGame::GetMonster(buf);
    std::cout << "Monster ID: " << dmonster->id() << std::endl;
    std::cout << "Monster Name: " << dmonster->name()->str() << std::endl;
    std::cout << "Monster HP: " << dmonster->hp() << std::endl;
    std::cout << "Monster Damage: " << dmonster->damage() << std::endl;
}