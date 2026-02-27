#include <bits/stdc++.h>
using namespace std;

/*
  Data Structure for Homework 1:

  Support two operations on a set/collection S of integers:
    1) INSERT(x): insert integer x
    2) DELETE_LARGE_HALF(): delete the largest half elements from S

  Key idea:
    - Keep S as an UNSORTED dynamic array (vector<int>) for O(1) append insert.
    - For DELETE_LARGE_HALF(), we find the global k-th smallest element t
      (k = ceil(n/2)) using deterministic selection (Median of Medians),
      then partition and keep exactly k smallest elements.

  Complexity sketch (amortized over m operations):
    - INSERT: O(1)
    - DELETE_LARGE_HALF: O(n) for current size n
    - But each delete halves the size, so total over many deletes is geometric:
        n + n/2 + n/4 + ... = O(n)
      Hence any sequence of m operations is O(m) (amortized).
*/

/* ---------- Deterministic Selection: Median of Medians (Worst-case O(n)) ---------- */

static int select_kth_deterministic(vector<int> &A, int k); // forward

// Helper: return median of A[l..r] (inclusive) by sorting that small range.
// For group size <= 5, this is constant time in analysis.
static int median_of_small_group(vector<int> &A, int l, int r) {
    sort(A.begin() + l, A.begin() + r + 1);
    int len = r - l + 1;
    return A[l + len / 2];
}

/*
  select_kth_deterministic(A, k):
    Return the k-th smallest element of A (0-indexed), worst-case O(n).
    Note: This function may reorder A internally (due to sorting small groups).
*/
static int select_kth_deterministic(vector<int> &A, int k) {
    int n = (int)A.size();

    // Base case: for small n, sort directly and pick.
    // Any fixed constant works; 50 is common and keeps code simple.
    if (n <= 50) {
        nth_element(A.begin(), A.begin() + k, A.end()); // average linear, but n is constant here
        return A[k];
    }

    // 1) Split into groups of 5, compute each group's median
    vector<int> medians;
    medians.reserve((n + 4) / 5);

    for (int i = 0; i < n; i += 5) {
        int l = i;
        int r = min(i + 4, n - 1);
        // Sorting group of size <=5 is constant work
        int med = median_of_small_group(A, l, r);
        medians.push_back(med);
    }

    // 2) Recursively find pivot = median of medians
    int mid = (int)medians.size() / 2;
    int pivot = select_kth_deterministic(medians, mid);

    // 3) Partition A into three groups: < pivot, == pivot, > pivot
    vector<int> L, E, G;
    L.reserve(n);
    E.reserve(n);
    G.reserve(n);

    for (int x : A) {
        if (x < pivot) L.push_back(x);
        else if (x > pivot) G.push_back(x);
        else E.push_back(x);
    }

    // 4) Recurse only into the side that contains the k-th element
    if (k < (int)L.size()) {
        return select_kth_deterministic(L, k);
    }
    if (k < (int)L.size() + (int)E.size()) {
        return pivot;
    }
    return select_kth_deterministic(G, k - (int)L.size() - (int)E.size());
}

/* ----------------------- Our Data Structure Wrapper ----------------------- */

class DeleteLargeHalfDS {
public:
    // Unsorted container storing all current elements
    vector<int> S;

    void insert_value(int x) {
        // O(1) amortized append
        S.push_back(x);
    }

    void delete_large_half() {
        int n = (int)S.size();
        if (n <= 1) return;

        // We choose to KEEP k = ceil(n/2) smallest elements.
        // Therefore we DELETE floor(n/2) largest elements.
        int k = (n + 1) / 2;            // ceil(n/2)
        int kth_index = k - 1;          // 0-index: the (k-1)-th index is the k-th smallest

        // Find threshold t = k-th smallest element (global)
        vector<int> tmp = S;            // copy because selection can reorder
        int t = select_kth_deterministic(tmp, kth_index);

        // Partition by t, keep exactly k elements <= t
        vector<int> L, E;
        L.reserve(n);
        E.reserve(n);

        for (int x : S) {
            if (x < t) L.push_back(x);
            else if (x == t) E.push_back(x);
            // if x > t, it's in the "largest" side and will be deleted
        }

        // We must keep exactly k elements total:
        // keep all L, then take enough from E to reach k
        int need = k - (int)L.size();
        if (need < 0) {
            // This should not happen if t is truly the k-th smallest.
            // But in case of logic error, we can truncate.
            need = 0;
        }
        if (need > (int)E.size()) {
            // Also should not happen, but guard anyway.
            need = (int)E.size();
        }

        vector<int> newS;
        newS.reserve(k);
        for (int x : L) newS.push_back(x);
        for (int i = 0; i < need; i++) newS.push_back(E[i]);

        S.swap(newS);
    }

    // For debugging / demonstration
    void print_state() const {
        cout << "S(size=" << S.size() << "): ";
        for (int x : S) cout << x << " ";
        cout << "\n";
    }
};

/* ----------------------- Example main (stdio input) -----------------------
   Input format (example):
     q
     op [x]
   where:
     op = 1 means INSERT x
     op = 2 means DELETE_LARGE_HALF

   Example:
     7
     1 10
     1 3
     1 7
     2
     1 5
     2
     2

   You can modify this to match your TA's preferred format.
------------------------------------------------------------------------- */
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int q;
    if (!(cin >> q)) return 0;

    DeleteLargeHalfDS ds;

    while (q--) {
        int op;
        cin >> op;
        if (op == 1) {
            int x;
            cin >> x;
            ds.insert_value(x);
        } else if (op == 2) {
            ds.delete_large_half();
        } else if (op == 3) {
            // optional debug
            ds.print_state();
        }
    }

    // Output final size and elements (for checking)
    cout << ds.S.size() << "\n";
    for (int x : ds.S) cout << x << " ";
    cout << "\n";
    return 0;
}
