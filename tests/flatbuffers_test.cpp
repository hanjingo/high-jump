
#include <gtest/gtest.h>
#include <string>
#include <hj/encoding/flatbuffers.hpp>
#include "monster_generated.h"


TEST(flatbuffers, build_and_parse)
{
    hj::flatbuffer_builder builder;
    auto                   monster = MyGame::CreateMonster(builder,
                                         1, // id
                                         builder.CreateString("Dragon"), // name
                                         100,  // hp
                                         35.5f // damage
    );
    builder.Finish(monster);
    uint8_t *buf  = builder.GetBufferPointer();
    size_t   size = builder.GetSize();
    ASSERT_GT(size, 0u);

    auto dmonster = MyGame::GetMonster(buf);
    ASSERT_NE(dmonster, nullptr);
    ASSERT_EQ(dmonster->id(), 1);
    ASSERT_EQ(dmonster->name()->str(), "Dragon");
    ASSERT_EQ(dmonster->hp(), 100);
    ASSERT_FLOAT_EQ(dmonster->damage(), 35.5f);
}