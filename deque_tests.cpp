#include <stdexcept>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "deque.hpp"

#include <string>
#include <vector>
#include <deque>

#define DEBUG

TEST_CASE("Trying 3000000 size...") {
    SECTION("Copy & move deque") {
        Deque<int> d(3000000);
        Deque<int> copy1{};
        copy1 = d;
        Deque<int> copy2{};
        copy2 = std::move(d);
        REQUIRE(copy1 == copy2);
    }
}

TEST_CASE("Deque_tests_21") {
    SECTION("Constructors...") {
        Deque<int> d1 = {1, 2, 3, 4, 5};
        Deque<int> d2 = d1;
        const Deque<int> d3 = d1;
        REQUIRE(d1 == d2);
        REQUIRE(d1 == d3);
    }

    SECTION("operator [] && at && push/pop_front") {
        Deque<int> d = {1, 3, 4, 5, 2, 3, 6, 8, 3, 5, 4};
        d[3] = 5;
        d[7] = 8;
        d[9] = 10;
        d[1] = 2;
        d.at(2) = 1;
        d.at(10) = 0;
        REQUIRE_THROWS_AS(d.at(20) = 666, std::out_of_range);

        REQUIRE(d == Deque<int>{1, 2, 1, 5, 2, 3, 6, 8, 3, 10, 0});

        d.push_front(11);
        d.push_front(12);
        d.push_front(13);
        d.push_front(14);
        #ifdef DEBUG
        d.PrintDeque();
        #endif // DEBUG
        REQUIRE(d.at(0) == 14);
        REQUIRE(d.at(14) == 0);
        REQUIRE(d[0] == 14);
        REQUIRE(d[14] == 0);
        REQUIRE(d.front() == 14);
        REQUIRE(d.back() == 0);

        d.pop_front();
        d.pop_front();
        REQUIRE(d.at(0) == 12);
        REQUIRE(d.at(12) == 0);
        REQUIRE(d.at(11) == 10);
        REQUIRE(d[0] == 12);
        REQUIRE(d[12] == 0);
        REQUIRE(d[11] == 10);
        REQUIRE(d.front() == 12);
        REQUIRE(d.back() == 0);
    }

    SECTION("size()") {
        std::string ss = "";
        Deque<int> ddd(10);
        for (int64_t i = 0; i < ddd.size(); ++i) {
            ss += std::to_string(ddd[i]);
        }

        REQUIRE(ss == "0000000000");
    }

    SECTION("pop() && simple iterators...") {
        Deque<int> d(334'000, 5);
        for (int64_t i = 0; i < d.size(); ++i) {
            d[i] = i;
        }

        Deque<int>::iterator left  = d.begin() + 100'000;
        Deque<int>::iterator right = d.end() - 233'990;
        while (d.begin() != left) {
            d.pop_front();
            REQUIRE(*(d.rbegin()) == 333999);
        }
        while (d.end() != right) {
            d.pop_back();
            REQUIRE(*(d.begin()) == 100000);
        }

        REQUIRE(right - left == 10);
    }

    SECTION("at() after some pop() operations") {
        Deque<float> d(64, 1.0);
        for (int64_t i = 0; i < d.size(); ++i) {
            d.at(i) = i;
        }

        for (float i = 1.0; d.size() != 32; ++i) {
            d.pop_front();
            REQUIRE(d.at(0) == i);
            REQUIRE(d.at(d.size() - 1) == 63.0);
        }
        for (float i = 62.0; d.size() != 2; --i) {
            d.pop_back();
            REQUIRE(d.at(d.size() - 1) == i);
            REQUIRE(d.at(0) == 32.0);
        }
    }

    SECTION("for auto iterator ++") {
        Deque<int> d = {1, -3, 5, -6};
        for (auto it = d.begin(); it != d.end(); ++it) {
            ++*it;
        }

        REQUIRE(d == Deque<int>{2, -2, 6, -5});
    }

    SECTION("for auto iterator --") {
        Deque<int> d = {1, 1, 1, 1, 1};
        auto left = d.begin();
        auto right = d.end();

        for (auto it = right - 1; it >= left; --it) {
            --*it;
        }

        REQUIRE(d == Deque<int>{0, 0, 0, 0, 0});
    }

    SECTION("erase") {
        Deque<char> d = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
        d.erase(d.begin() + 1);
        REQUIRE(d == Deque<char>{'a', 'c', 'd', 'e', 'f', 'g'});
        d.erase(d.begin() + 3);
        REQUIRE(d == Deque<char>{'a', 'c', 'd', 'f', 'g'});
        d.erase(d.begin() + 3);
        REQUIRE(d == Deque<char>{'a', 'c', 'd', 'g'});
    }

    SECTION("insert") {
        Deque<float> dd(5, 5.);
        dd.insert(dd.begin() + 1, 7.0);
        REQUIRE(dd == Deque<float>{5.0, 7.0, 5.0, 5.0, 5.0, 5.0});
        dd.insert(dd.begin(), {3, 4.0});
        REQUIRE(dd == Deque<float>{3.0, 4.0, 5.0, 7.0, 5.0, 5.0, 5.0, 5.0});
        dd.insert(dd.begin() + 2, {4, 5.0});
        REQUIRE(dd == Deque<float>{3.0, 4.0, 4.0, 5.0, 5.0, 7.0, 5.0, 5.0, 5.0, 5.0});
    }

    SECTION("auto &x : Deque") {
        Deque<float> dd(5, 5.);
        for (auto &x : dd) {
            x = 1.;
        }

        REQUIRE(dd == Deque<float>{1.0, 1.0, 1.0, 1.0, 1.0});
    }

    SECTION("&*") {
        Deque<int> d = {1, 2, 3};
        auto left_ptr  = &*d.begin();
        auto right_ptr = &*(d.end() - 1);

        REQUIRE(*left_ptr == 1);
        REQUIRE(*right_ptr == 3);
    }

    SECTION("Deque<struct>") {
        struct Coords {
            int x;
            int y;
        };

        Deque<Coords> crds(10);
        REQUIRE(crds[3].x == 0);
        REQUIRE(crds[5].y == 0);

        crds[7].x = 4;
        crds[7].y = -2;

        REQUIRE(crds[7].x == 4);
        REQUIRE(crds[7].y == -2);
    }

    SECTION("hard iterators") {
        Deque<std::pair<int, int>> d = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}};

        int64_t counter = 0;
        for (auto pair : d) {
            REQUIRE(pair == d.at(counter));
            ++counter;
        }
        REQUIRE(counter == d.size());

        for (auto it = d.rbegin(); it != d.rend(); ++it) {
            --counter;
            REQUIRE(*it == d.at(counter));
        }

        REQUIRE(*d.rbegin() == *(d.end() - 1));

        Deque<long double> d_empty{};
        REQUIRE(d_empty.rbegin() == d_empty.rend());

        REQUIRE_THROWS(d_empty.front());
        REQUIRE_THROWS(d_empty.back());
        REQUIRE_THROWS(d_empty.at(0) = 5);
        REQUIRE_THROWS(d_empty.at(1e9) = 5);

        // Actually UB
        // REQUIRE_THROWS(*d_empty.begin());
        // REQUIRE_THROWS(*d_empty.end());
    }

    SECTION("iterators: non-const -> const") {
        Deque<int> d = {1, 2, 3, 4, 5};
        auto it = d.begin();
        Deque<int>::const_iterator cit = d.cend();
        cit = it;
        // it = cit; WRONG!
        REQUIRE(cit == d.begin());

        auto rit = d.rbegin();
        Deque<int>::const_reverse_iterator crit = d.crend();
        crit = rit;
        // rit = crit; WRONG!
        REQUIRE(crit == d.rbegin());
    }

    SECTION("Deque<Union>") {
        union u {
            int64_t     val_int;
            double      val_dbl;
        };

        Deque<union u> d(2);
        d[0].val_int = 1;
        d[1].val_dbl = .75;

        REQUIRE(d.at(0).val_int == 1);
        REQUIRE(d.at(1).val_dbl == .75);
    }

    SECTION("Ustal'...") {
        Deque<int32_t> d(64);
        for (int64_t i = 0; i < d.size(); ++i) {
            d[i] = i;
        }

        for (int64_t i = 0; i < 60; ++i) {
            d.pop_front();
            REQUIRE(d.front() == i + 1);
            REQUIRE(d.at(0) == i + 1);
            REQUIRE(d[0] == i + 1);
        }
        REQUIRE(d == Deque<int32_t>{60, 61, 62, 63});

        for (int32_t i = 0; i < 128; ++i) {
            d.push_front(i);
            REQUIRE(d.front() == i);
            REQUIRE(d.at(0) == i);
            REQUIRE(d[0] == i);
            REQUIRE(d.back() == 63);
        }
    }

    SECTION("Random testing with std::deque...") {
        int low_code_dist  = 1;
        int high_code_dist = 14;
        int low_num_dist  = 1;
        int high_num_dist = 100;
        int random_operation_code = 0;
        int random_number = 0;
        int pos = 0;
        // auto seed = (unsigned int)std::time(nullptr);
        unsigned int seed = 1683102432;
        std::srand(seed);

        std::cout << "SEED: " << seed << std::endl;

        Deque<int> d;
        std::deque<int> true_d;
        for (int64_t counter = 0; counter < 10000; ++counter) {
            do {
                random_operation_code = low_code_dist + std::rand() % ( high_code_dist - low_code_dist );
            } while (random_operation_code > 3 && (d.empty() && true_d.empty()));
            random_number = low_num_dist + std::rand() % ( high_num_dist - low_num_dist );
            
            #ifdef DEBUG
            std::cout << "DEQUE: " << std::endl;
            d.PrintDeque();
            std::cout << "T_DEQ: " << std::endl;
            for (auto e : true_d) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;

            std::cout << "CURR_OPERATION: (" << random_operation_code << "; " << random_number << ")" << std::endl;
            #endif // DEBUG

            // Checking for iterators work
            for (auto& e : d) {e = 1 + e - 1;}

            switch(random_operation_code) {
                case 1:
                    d.push_front(random_number);
                    true_d.push_front(random_number);
                    REQUIRE(d == true_d);
                    break;
                case 2:
                    d.push_back(random_number);
                    true_d.push_back(random_number);
                    REQUIRE(d == true_d);
                    break;
                case 3:
                    if (true_d.empty() && d.empty()) {
                        pos = 0;
                    } else {
                        pos = std::rand() % (true_d.size());
                    }
                    std::cout << "POS: " << pos << std::endl;

                    d.insert(d.begin() + pos, random_number);
                    true_d.insert(true_d.begin() + pos, random_number);
                    REQUIRE(d == true_d);
                    break;
                case 4:
                    d.pop_front();
                    true_d.pop_front();
                    REQUIRE(d == true_d);
                    break;
                case 5:
                    d.pop_back();
                    true_d.pop_back();
                    REQUIRE(d == true_d);
                    break;
                case 6:
                    d.erase(d.begin() + (random_number % d.size()));
                    true_d.erase(true_d.begin() + (random_number % true_d.size()));
                    REQUIRE(d == true_d);
                    break;
                case 7:
                    REQUIRE(d.front() == true_d.front());
                    break;
                case 8:
                    REQUIRE(d.back() == true_d.back());
                    break;
                case 9:
                    REQUIRE(d.at(random_number % d.size()) == true_d.at(random_number % true_d.size()));
                    break;
                case 10:
                    REQUIRE(d[random_number % d.size()] == true_d[random_number % true_d.size()]);
                    break;
                case 11:
                    REQUIRE(d.end() - d.begin() == true_d.end() - true_d.begin());
                    break;
                case 12:
                    REQUIRE(d.size() == static_cast<int64_t>(true_d.size()));
                    break;
                case 13:
                    REQUIRE(*(d.rend() - 1) == *(true_d.rend() - 1));
                    break;
                case 14:
                    REQUIRE(*d.rbegin() == *true_d.rbegin());
                    break;
                default:
                    std::cout << "DEFAULT" << std::endl;
                    break;
            }
        }
    }
}