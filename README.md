# sorted_view

Produces a sorted view of a given array (`std::vector`, `std::list`, `std::array`, etc.) of objects based on the given comparator (or the default `std::less`), without changing the original array and moving the actual objects. This enables having multiple sorted views of an array based on different comparators at the same time.

# How to use
Insert the header file `sorted_view.hpp` in your project tree and include it in your code. To compile `sorted_view.hpp` you need at least `c++20`.

## Manual

You can construct a sorted view of an array `arr` with a custom comparator, as follows:
```
sorted_view sv (arr, [] (const auto &a, const auto &b) {return a.attr < b.attr;});
```

The sorted view has `begin ()` and `end ()` that return an iterator of the view. The sorted view is automatically computed when its iterator is called (e.g., in a `for` loop):
```
for (const auto &item: sv) {    // here the sorted view is computed, if it is not already sorted.
    ...
}
```
You can also manually resort (recompute) the sorted view, if the array is modified (i.e., some items are deleted and/or inserted):
```
sv.resort ();
```

Or, perform the sort operation if the sorted view is not already sorted:
```
sv.check_resort ();
```

Alternativly, if you know how many items are added to the back of the array, you can merge them in the sorted list to prevent sorting the view from scratch, and improve the performance:
```
arr.push_back (item1);
arr.push_back (item2);
arr.push_back (item3);
sv.merge_from_back ();      // the recently added items are merged into the sorted view
```




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
