#include <bits/stdc++.h>
using namespace std;
using ll = long long;

const int N = 1e5 + 5;
ll a[N], prefix[N];

int main(){
    ios::sync_with_stdio(0),cin.tie(0),cout.tie(0);
    int n;cin >> n;

    for(int i = 1; i <= n; ++i)
        cin >> a[i];
    for(int i = 1; i <= n; ++i)
        prefix[i] = prefix[i - 1] + a[i];
    int q; cin >> q;
    while(q--){
        int l, r; cin >> l >> r;
        cout << prefix[r] - prefix[l - 1] << '\n';
    }

}