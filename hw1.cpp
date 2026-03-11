class DeleteLargeHalfDS {
public:
    vector<int> S;   // 用未排序動態陣列儲存集合元素

    void insert_value(int x) {
        S.push_back(x);   // 直接插入尾端，時間 O(1) amortized
    }

    void delete_large_half() {
        int n = S.size();
        if (n <= 1) return;   // 0 或 1 個元素時，不需刪除

        int k = (n + 1) / 2;   // 刪除最大的一半後，保留 ceil(n/2) 個元素

        vector<int> tmp = S;   // 複製一份，供 selection 使用
        int t = select_kth_deterministic(tmp, k - 1);   // 找第 k 小元素（0-based）

        vector<int> L, E;   // L: 小於 t，E: 等於 t

        for (int x : S) {
            if (x < t) L.push_back(x);        // 小於 t 的一定保留
            else if (x == t) E.push_back(x);  // 等於 t 的先暫存
        }

        vector<int> B = L;   // 先保留所有小於 t 的元素

        for (int x : E) {
            if ((int)B.size() < k) B.push_back(x);   // 若不足 k 個，從 E 補上
            else break;
        }

        S = B;   // 更新集合，完成 DELETE-LARGE-HALF
    }
};