#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

using namespace std;

/*
    Branch and Bound for 0/1 Knapsack
    ---------------------------------
    目標：
        在容量限制 W 下，從多個物品中選出若干個，
        使得總價值最大，且總重量不超過 W。

    核心概念：
    1. Branch（分支）
       對第 level 個物品做決策：
       - 選它
       - 不選它

    2. Bound（界限）
       對某個節點，估計「從這裡往下，理論上最多還能拿到多少價值」
       如果這個理論上限都比目前最佳解差，
       就沒必要繼續探索這個分支，直接剪掉。

    3. Upper Bound 的估法
       使用 fractional knapsack（可分割背包）的 greedy 想法：
       - 先依照 value/weight 比值排序
       - 能整個放進去的就整個放
       - 放不下的最後一個物品，只放部分
       這樣算出的值，是 0/1 knapsack 的 upper bound。
*/


// ------------------------------
// 物品結構
// ------------------------------
struct Item {
    int id;             // 原始編號，方便最後回推是哪個物品
    double weight;      // 重量
    double value;       // 價值
    double ratio;       // value / weight
};


// ------------------------------
// 搜尋樹中的節點
// ------------------------------
struct Node {
    int level;                  // 目前考慮到第幾個物品（索引）
    double profit;              // 目前已選物品的總價值
    double weight;              // 目前已選物品的總重量
    double bound;               // 從此節點出發的 upper bound
    vector<int> taken;          // taken[i] = 1 表示排序後第 i 個物品被選

    /*
        priority_queue 預設是大根堆，但它需要比較規則。
        我們希望「bound 越大」的節點優先被展開，
        所以這裡要反向定義。
    */
    bool operator<(const Node& other) const {
        return bound < other.bound;
    }
};


// ------------------------------
// 計算某個節點的 upper bound
// ------------------------------
double computeBound(const Node& u, int n, double capacity, const vector<Item>& items) {
    /*
        如果當前節點的重量已經超出容量，
        代表這個節點本身 infeasible，bound 直接設成 0。
    */
    if (u.weight > capacity) {
        return 0.0;
    }

    double boundValue = u.profit;
    double totalWeight = u.weight;

    // 從下一個尚未決策的物品開始，盡量往背包塞
    int j = u.level + 1;

    // 先盡量整個放入
    while (j < n && totalWeight + items[j].weight <= capacity) {
        totalWeight += items[j].weight;
        boundValue += items[j].value;
        j++;
    }

    /*
        如果還有剩餘容量，但下一個物品放不完整，
        那就用 fractional knapsack 的方式只放一部分。
        注意：這只是拿來估 upper bound，不是真的允許 0/1 問題拆物品。
    */
    if (j < n) {
        double remain = capacity - totalWeight;
        boundValue += remain * items[j].ratio;
    }

    return boundValue;
}


// ------------------------------
// Branch and Bound 主程式
// 回傳最大價值與對應選取情況
// ------------------------------
pair<double, vector<int>> knapsackBranchAndBound(vector<Item> items, double capacity) {
    int n = (int)items.size();

    /*
        先依照 value/weight 由大到小排序
        這樣 upper bound 才會算得比較緊，
        剪枝效果通常也會比較好。
    */
    sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        return a.ratio > b.ratio;
    });

    // 優先佇列：每次先展開 bound 最大的節點（best-first）
    priority_queue<Node> pq;

    // 初始化 root 節點：還沒考慮任何物品
    Node root;
    root.level = -1;
    root.profit = 0.0;
    root.weight = 0.0;
    root.taken = vector<int>(n, 0);
    root.bound = computeBound(root, n, capacity, items);

    pq.push(root);

    // 目前找到的最佳 feasible solution
    double bestProfit = 0.0;
    vector<int> bestTaken(n, 0);

    while (!pq.empty()) {
        // 取出目前 upper bound 最大的節點
        Node current = pq.top();
        pq.pop();

        /*
            如果這個節點的 bound 都不可能超過目前最佳解，
            那它的整棵子樹都可以剪掉。
        */
        if (current.bound <= bestProfit) {
            continue;
        }

        /*
            若 current.level == n - 1，
            代表已經處理完最後一個物品，
            沒有子節點可再展開。
        */
        if (current.level == n - 1) {
            continue;
        }

        // 下一個要決策的物品索引
        int nextLevel = current.level + 1;

        // =========================================================
        // 子節點 1：選取 nextLevel 這個物品
        // =========================================================
        Node withItem;
        withItem.level = nextLevel;
        withItem.weight = current.weight + items[nextLevel].weight;
        withItem.profit = current.profit + items[nextLevel].value;
        withItem.taken = current.taken;
        withItem.taken[nextLevel] = 1;

        /*
            先檢查 feasibility：
            只有在重量不超過 capacity 時，
            這個節點本身才是 feasible。
        */
        if (withItem.weight <= capacity) {
            /*
                如果這個 feasible 節點的 profit 比目前最佳解更好，
                就更新 best solution。
            */
            if (withItem.profit > bestProfit) {
                bestProfit = withItem.profit;
                bestTaken = withItem.taken;
            }

            // 即使目前 feasible，也還是可以往下擴展，所以要算 bound
            withItem.bound = computeBound(withItem, n, capacity, items);

            // 只有在有希望超越目前最佳解時才丟回 pq
            if (withItem.bound > bestProfit) {
                pq.push(withItem);
            }
        }

        // =========================================================
        // 子節點 2：不選取 nextLevel 這個物品
        // =========================================================
        Node withoutItem;
        withoutItem.level = nextLevel;
        withoutItem.weight = current.weight;
        withoutItem.profit = current.profit;
        withoutItem.taken = current.taken;
        withoutItem.taken[nextLevel] = 0;
        withoutItem.bound = computeBound(withoutItem, n, capacity, items);

        // 只有有希望超越最佳解時才保留
        if (withoutItem.bound > bestProfit) {
            pq.push(withoutItem);
        }
    }

    /*
        bestTaken 目前是對「排序後」的物品紀錄，
        但我們通常想知道原始輸入順序下哪些物品被選。
        所以這裡要轉回原始編號。
    */
    vector<int> answerByOriginalId(n, 0);
    for (int i = 0; i < n; i++) {
        if (bestTaken[i] == 1) {
            answerByOriginalId[items[i].id] = 1;
        }
    }

    return {bestProfit, answerByOriginalId};
}


// ------------------------------
// 測試主程式
// ------------------------------
int main() {
    /*
        這裡直接放一組簡單測試資料
        你也可以自己改成老師上課的例子
    */
    vector<double> weights = {2, 5, 10, 5};
    vector<double> values  = {40, 30, 50, 10};
    double capacity = 16;

    vector<Item> items;
    for (int i = 0; i < (int)weights.size(); i++) {
        Item item;
        item.id = i;                         // 記錄原始編號
        item.weight = weights[i];
        item.value = values[i];
        item.ratio = values[i] / weights[i];
        items.push_back(item);
    }

    auto result = knapsackBranchAndBound(items, capacity);

    double maxProfit = result.first;
    vector<int> chosen = result.second;

    cout << fixed << setprecision(2);
    cout << "Maximum Profit = " << maxProfit << "\n";
    cout << "Chosen items (original order):\n";

    double totalWeight = 0.0;
    double totalValue = 0.0;

    for (int i = 0; i < (int)chosen.size(); i++) {
        cout << "Item " << i
             << " -> " << (chosen[i] ? "Take" : "Skip") << "\n";

        if (chosen[i]) {
            totalWeight += weights[i];
            totalValue += values[i];
        }
    }

    cout << "Total Weight = " << totalWeight << "\n";
    cout << "Total Value  = " << totalValue << "\n";

    return 0;
}
