#include <ranges>
#include <tuple>

int main() {
    int a[4]{}, b[4]{};
    std::ignore = std::views::zip(a, b);
}
