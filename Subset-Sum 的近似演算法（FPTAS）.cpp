#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

vector<int> trim(const vector<int>& L, double delta) {
    if (L.empty()) return {};

    vector<int> trimmed;
    trimmed.push_back(L[0]);
    int last = L[0];

    for (int i = 1; i < (int)L.size(); i++) {
        // # 若目前值明顯比 last 大，才保留
        if (L[i] > last * (1.0 + delta)) {
            trimmed.push_back(L[i]);
            last = L[i];
        }
    }
    return trimmed;
}

int approx_subset_sum(const vector<int>& S, int t, double epsilon) {
    int n = (int)S.size();
    vector<int> L = {0};

    for (int x : S) {
        vector<int> added;
        for (int y : L) {
            added.push_back(y + x);
        }

        // # 合併 old list 與 added list
        vector<int> merged = L;
        merged.insert(merged.end(), added.begin(), added.end());

        // # 排序 + 去重
        sort(merged.begin(), merged.end());
        merged.erase(unique(merged.begin(), merged.end()), merged.end());

        double delta = epsilon / (2.0 * n);

        // # trimming
        vector<int> trimmed = trim(merged, delta);

        // # 刪掉 > t 的值
        vector<int> newL;
        for (int v : trimmed) {
            if (v <= t) {
                newL.push_back(v);
            }
        }

        L = newL;
    }

    return *max_element(L.begin(), L.end());
}

int main() {
    vector<int> S = {104, 102, 201, 101};
    int t = 308;
    double epsilon = 0.4;

    cout << "Approx subset sum = " << approx_subset_sum(S, t, epsilon) << endl;
    return 0;
}
