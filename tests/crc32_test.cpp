#include <gtest/gtest.h>
#include <libcpp/algo/crc32.h>

TEST(crc32, standard_test_vectors)
{
    const char* test1 = "123456789";
    uint32_t result1 = crc32_calc(test1, strlen(test1), 0);
    EXPECT_EQ(result1, 0xCBF43926) << "Standard test vector '123456789' failed";
    
    const char* test2 = "";
    uint32_t result2 = crc32_calc(test2, strlen(test2), 0);
    EXPECT_EQ(result2, 0x00000000) << "Empty string test failed";
    
    const char* test3 = "a";
    uint32_t result3 = crc32_calc(test3, strlen(test3), 0);
    EXPECT_EQ(result3, 0xE8B7BE43) << "Single character 'a' test failed";
     
    const char* test4 = "abc";
    uint32_t result4 = crc32_calc(test4, strlen(test4), 0);
    EXPECT_EQ(result4, 0x352441C2) << "String 'abc' test failed";
}

TEST(crc32, large_text_crc)
{
    const char* big_data = R"(CHAPTER VIII.
THE POTIONS MASTER There, look!
Where?
Next to the tall kid with the red hair.
Wearing the glasses.
Did you see his face?
Did you see his scar?
Whispers followed Harry from the moment he left his dormitory next day.
People queuing outside classrooms stood on tiptoe to get a look at him, or doubled back to pass him him in the corridors again, staring.
Harry wished they wouldn't, because he was trying to concentrate on finding his way to classes.
There were a hundred and forty-two staircases at Hogwarts—wide, sweeping ones, narrow, rickety ones, some that led somewhere different on a Friday, some with a vanishing step half-way up that you had to remember to jump.
Then there were doors that wouldn't open unless you asked politely, or tickled them in exactly But Peeves the poltergeist was worth two locked doors and a trick staircase, if you met him when you were late for class.
He would drop waste-paper baskets on your head, pull rugs from under your feet, pelt you with bits of chalk, or sneak up behind you, invisible, grab your nose and screech, "'Got ya, conk!' Even worse than Peeves, if that was possible, was the caretaker, Argus Filch.
Harry and Ron managed to get on the wrong side of him on their very first morning.
Filch found them trying to force their way through a door which, unluckily, turned out to be the entrance to the out-of-bounds corridor on the third floor.
He wouldn't believe they were lost, was sure they were trying to break into it on purpose, and was threatening to lock them in the dungeons when they were rescued by Professor Quirrell, who was passing.
Filch owned a cat called Mrs. Norris, a scrawny, dust-coloured creature with bulging, lamp-like eyes, just like Filch's.
She patrolled the corridors alone.
Break a rule in front of her, put just one toe out of line, and she'd whisk off for Filch, who'd appear wheezing two seconds later.
Filch knew the secret passageways of the school better than anyone, except perhaps the Weasley twins, and could pop up as suddenly as any of the ghosts.
The students all hated him, and it was the dearest ambition of many to give Mrs. Norris a good kick.
And then, once you had managed to find them, there were the lessons themselves.
There was a lot more to magic, as Harry quickly found out, than waving your wand and saying a few funny words.
They had to study the night skies through their telescopes every Wednesday at midnight, and learn the names of different stars and the movements of the planets.
Three times a week they went out to the greenhouses behind the castle to study herbology, with a dumpy little witch called Professor Sprout, where they learnt how to take care of all the strange plants and fungi, and found out what they were used for.
Easily the most boring lesson was History of Magic, which was the only class taught by a ghost.
Professor Binns had been very old indeed when he had fallen asleep in front of the staffroom fire, and got up next morning to teach, leaving his body behind him.
Binns droned on and on while they scribbled down names and dates, and got Emmerich the Evil and Urich the Oddball mixed up.
Professor Flitwick, the charms-teacher, was a tiny little wizard who had to stand on a pile of books to see over his desk.
At the start of their first lesson he took the register, and when he reached Harry's name he gave an excited squeak and toppled out of sight.
Professor McGonagall was again different.
Harry had been quite right to think she wasn't a teacher to cross.
Strict and clever, she gave them a talking-to the moment they had sat down in her first class.
"'Transfiguration is some of the most complex and dangerous magic you will learn at Hogwarts, ' she said.
Anyone messing around in my class will leave and not come back.
You have been warned.' Then she changed her desk into a pig, and back again.
They were all very impressed, and couldn't wait to get started, but soon realised they weren't going to be changing the furniture into animals for a long time.
After making a lot of complicated notes, they were each given a match, and started trying to turn it into a needle.
By the end of the lesson only Hermione Granger had made any difference to her match.
Professor McGonagall showed the class how it had gone all silver and pointy, and gave Hermione a rare smile.
The class everyone had really been looking forward to was Defence Against the Dark Arts, but Quirrell's lessons turned out to be a bit of a joke.
His classroom smelled strongly of garlic, which everyone said was to ward off a vampire he'd met in Romania, and was afraid would be coming back to get him one of these days.
His turban, he told them, had been given to him by an African prince as a thank-you for getting rid of a troublesome zombie, but they weren't sure they believed this story.
For one thing, when Seamus Finnegan asked eagerly to hear how Quirrell had fought off the zombie, Quirrell went pink and sta)";
    EXPECT_EQ(crc32_calc(big_data, strlen(big_data), 0), 0x7e3f32f7);
}

TEST(crc32, incremental_calculation)
{
    const char* part1 = "Hello, ";
    const char* part2 = "World!";
    
    std::string combined = std::string(part1) + part2;
    uint32_t full_crc = crc32_calc(combined.c_str(), combined.length(), 0);
    
    uint32_t crc1 = crc32_calc(part1, strlen(part1), 0);
    uint32_t crc2 = crc32_calc(part2, strlen(part2), crc1);
    
    EXPECT_EQ(full_crc, crc2) << "Incremental CRC calculation should match full calculation";
}

TEST(crc32, edge_cases)
{
    EXPECT_EQ(crc32_calc(nullptr, 0, 0), 0);
    // EXPECT_EQ(crc32_calc(nullptr, 10, 123), 123);
    
    const char* data = "test";
    EXPECT_EQ(crc32_calc(data, 0, 0), 0);
}