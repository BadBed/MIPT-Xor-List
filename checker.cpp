#include "checker.h"

std::vector<CheckerEvent> Checker::events;

Checker::Checker() {
    Checker::events.emplace_back(CONSTRUCT_DEFAULT);
}

Checker::Checker(const Checker& other) {
    Checker::events.emplace_back(CONSTRUCT_COPY);
}

Checker::Checker(Checker&& other) noexcept {
    Checker::events.emplace_back(CONSTRUCT_MOVE);
}

Checker::~Checker() {
    Checker::events.emplace_back(DESTRUCT);
}

Checker& Checker::operator=(const Checker& other) {
    Checker::events.emplace_back(COPY);
    return *this;
}

Checker& Checker::operator=(Checker&& other) noexcept {
    Checker::events.emplace_back(MOVE);
    return *this;
}
