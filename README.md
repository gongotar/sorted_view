# sorted_view
A sorted view on the given std::range type.

## Sample Usage:

```cpp
template <typename L1, typename L2>
void print_arrays (const std::string &out, L1 &&l1, L2 &&l2) {
    std::cout << out << '\n';
    for (const auto &v: l1) std::cout << v << ' ';
    std::cout << '\n';
    for (const auto &v: l2) std::cout << v << ' ';
    std::cout << '\n';
}

int main () {

    std::vector <int> arr {5, 3, 45, 6, 4, 1, 3, 8, 9, 24, 4};   // also std::list and other containers are possible
    sorted_view sv (arr);

    print_arrays ("output 1", arr, sv);

    arr.push_back (53);
    arr.push_back (22);
    arr.push_back (54);
    arr.push_back (12);
    arr.push_back (31);
    arr.push_back (53);
    arr.push_back (75);

    sv.merge_from_back ();        // if this is not called, the whole view will be sorted again. 
                                  // By merging the new items we avoid sorting the whole array and improve the performance

    print_arrays ("output 2", arr, sv);

    std::erase_if (arr, [] (const auto n) {return n % 2 == 1;});
    arr.push_back (42);
    arr.push_back (21);
    arr.push_back (85);

    print_arrays ("output 3", arr, sv);

    auto lres = std::ranges::lower_bound (sv, 20);
    auto ures = std::ranges::upper_bound (sv, 20);
    
    std::cout << "lower bound: " << *lres << " upper bound: " << *ures << std::endl;
}
```
Output:
```
output 1
5 3 45 6 4 1 3 8 9 24 4 
1 3 3 4 4 5 6 8 9 24 45 
output 2
5 3 45 6 4 1 3 8 9 24 4 53 22 54 12 31 53 75 
1 3 3 4 4 5 6 8 9 12 22 24 31 45 53 53 54 75 
output 3
6 4 8 24 4 22 54 12 42 21 85 
4 4 6 8 12 21 22 24 42 54 85 
lower bound: 21 upper bound: 21
```
