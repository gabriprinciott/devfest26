#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

using Event = std::pair<int, long long>;

std::pair<int, int> longest_window(const std::vector<Event>& events, int limit, long long budget) {
    if (limit <= 0 || budget < 0) {
        return {0, 0};
    }

    int n = static_cast<int>(events.size());
    std::unordered_map<int, int> counts;
    int distinct = 0;
    long long total = 0;
    int left = 0;

    int best_len = 0;
    long long best_cost = 0;
    std::pair<int, int> best{0, 0};

    for (int right = 0; right < n; ++right) {
        const auto [category, cost] = events[right];
        if (counts[category] == 0) {
            ++distinct;
        }
        ++counts[category];
        total += cost;

        while (left <= right && (distinct > limit || total > budget)) {
            const auto [old_category, old_cost] = events[left];
            --counts[old_category];
            if (counts[old_category] == 0) {
                --distinct;
            }
            total -= old_cost;
            ++left;
        }

        if (left <= right) {
            const int length = right - left + 1;
            if (length > best_len) {
                best_len = length;
                best_cost = total;
                best = {left, right + 1};
            } else if (length == best_len) {
                if (total < best_cost) {
                    best_cost = total;
                    best = {left, right + 1};
                }
            }
        }
    }
    return best;
}

int main() {
    int count, limit;
    long long budget;
    if (!(std::cin >> count >> limit >> budget)) return 0;
    std::vector<Event> events(count);
    for (auto& [category, cost] : events) {
        std::cin >> category >> cost;
    }

    auto [start, end] = longest_window(events, limit, budget);
    std::cout << start << " " << end << "\n";
}
