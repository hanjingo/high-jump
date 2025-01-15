#include <iostream>

#include <libcpp/db/redis.hpp>

int main(int argc, char* argv[])
{
    auto ctx = redisConnect("127.0.0.1", 6379);
    std::cout << "redisConnect(\"127.0.0.1\", 6379)" << std::endl;

    auto rep = static_cast<redisReply*>(redisCommand(ctx, "SET %s %s", "name", "he"));
    std::cout << "redisCommand(ctx, \"SET %s %s\", \"name\", \"he\")->str = " << rep->str << std::endl;
    freeReplyObject(rep);

    rep = static_cast<redisReply*>(redisCommand(ctx, "GET %s", "name"));
    std::cout << "redisCommand(ctx, \"GET %s %s\", \"name\")->str = " << rep->str << std::endl;
    freeReplyObject(rep);

    rep = static_cast<redisReply*>(redisCommand(ctx, "INCR counter"));
    std::cout << "redisCommand(ctx, \"INCR counter\")->integer = " << rep->integer << std::endl;
    freeReplyObject(rep);

    rep = static_cast<redisReply*>(redisCommand(ctx, "DEL name"));
    if (rep->integer != 1) {
        std::cout << "redisCommand(ctx, \"DEL name\") FAIL" << std::endl;
        return 0;
    }
    std::cout << "redisCommand(ctx, \"DEL name\") SUCC" << std::endl;
    freeReplyObject(rep);

    rep = static_cast<redisReply*>(redisCommand(ctx, "LPUSH li %s", "1"));
    std::cout << "redisCommand(ctx, \"LPUSH li %s\", \"1\")" << std::endl;
    freeReplyObject(rep);

    rep = static_cast<redisReply*>(redisCommand(ctx, "LPUSH li %s", "2"));
    std::cout << "redisCommand(ctx, \"LPUSH li %s\", \"2\")" << std::endl;
    freeReplyObject(rep);

    rep = static_cast<redisReply*>(redisCommand(ctx, "LRANGE li 0 -1"));
    if (rep->type = REDIS_REPLY_ARRAY) {
        for (auto i = 0; i < rep->elements; i++) {
            std::cout << "redisCommand(ctx, \"LRANGE li 0 -1\") >> " << rep->element[i]->str << std::endl;
        }
    }
    freeReplyObject(rep);

    redisFree(ctx);
    std::cout << "redisFree(ctx)" << std::endl;

    std::cin.get();
    return 0;
}