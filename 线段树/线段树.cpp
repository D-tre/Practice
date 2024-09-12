#include <iostream>
#include <vector>

void solve() {
    int n, m;
    std::cin >> n >> m;
    std::vector<int> b(n + 10);
    std::vector<int> a(n + 10);
    for (int i = 1; i <= n; ++i)
    {
        a[i] = i;
    }

    for (int i = 1; i <= m; i++) 
    {
        int x, y;
        std::cin >> x >> y;

        b[x] ++, b[y + 1]--;
        a[y] = std::min(a[y], x);
    }

    for (int i = 1; i <= n + 5; i++) {
        b[i] += b[i - 1];
    }

    for (int i = n - 1; i > 0; --i)
    {
        a[i] = std::min(a[i + 1], a[i]);
    }

    std::vector<int> g(n + 10);
    for (int i = 1; i <= n; i++) 
    {
        g[i] = std::max(g[i - 1], g[a[i] - 1] + b[i]);
    }
    std::cout << g[n] << "\n";
}

signed main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int T = 1;

    std::cin >> T;

    while (T--) {
        solve();
    }
    return 0;
}