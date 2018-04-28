#pragma once
#include <vector>

enum CheckerEvent {CONSTRUCT_DEFAULT, CONSTRUCT_COPY,
    CONSTRUCT_MOVE, COPY, MOVE, DESTRUCT};

class Checker {
public:
    static std::vector<CheckerEvent> events;

    Checker();
    Checker(const Checker&);
    Checker(Checker&&) noexcept;

    ~Checker();

    Checker& operator=(const Checker&);
    Checker& operator=(Checker&&) noexcept;
};