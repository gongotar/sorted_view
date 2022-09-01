/**
Copyright 2022 Masoud Gholami

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**/

#ifndef SORTED_VIEW_HPP
#define SORTED_VIEW_HPP

#include <ranges>
#include <algorithm>
#include <numeric>


template <std::ranges::range Range, typename Comp = std::less <>>
class sorted_view: public std::ranges::view_interface <sorted_view <Range, Comp>> {

    template <typename T, typename OP = std::less <>>
    struct pointer_op {
        T *base_;
        const OP pop;
        
        constexpr explicit pointer_op (T *base, OP op = {}): base_ {base}, pop {op} {
        }

        template <class V, class U>
            requires std::totally_ordered_with <V, U>
        constexpr bool operator () (V&& v, U&& u) const {
            return pop (*(base_ + v), *(base_ + u));
        }
    };

    using Item = std::ranges::range_value_t <Range>;
    using Iterator_Type = typename Range::iterator;

    Range * const range;
    //std::vector <typename Range::value_type *> pointers;
    const Comp comp_;
    pointer_op <Item, Comp> op;
    std::vector <long> pointers;

    constexpr void update_pointers (bool merge = false) {
        op.base_ = &(*range->begin ());
        if constexpr (std::contiguous_iterator <Iterator_Type>) {
            const auto pointers_size = pointers.size ();
            pointers.resize (range->size ());
            if (range->size () > pointers_size) {
                std::iota (pointers.begin () + pointers_size, pointers.end (), pointers_size);
            }
            else if (range->size () < pointers_size) {
                std::iota (pointers.begin (), pointers.end (), 0);
            }
        }
        else {
            if (merge) {
                auto it = range->crbegin (); 
                const auto new_items = range->size () - pointers.size ();
                pointers.reserve (range->size ());
                for (auto i = new_items; i > 0; i--) {
                    pointers.push_back (&(*it) - op.base_);
                    it ++;
                }
            }
            else {
                pointers.clear ();
                pointers.reserve (range->size ());
                const auto range_start_pointer = &(*range->begin ());
                for (auto &item: *range) {
                    pointers.push_back (&item - range_start_pointer);
                }
            }
        }
    }

public:
    constexpr explicit sorted_view (Range &arr, Comp comp = {}): 
        range {&arr}, 
        comp_ {comp},
        op (&(*range->begin ()), comp_) {
    }

    constexpr sorted_view (const sorted_view &sv): 
        range {sv.range}, comp_ {sv.comp_}, op {sv.op}, pointers {sv.pointers} {}


    constexpr sorted_view (sorted_view &&sv): 
        range {sv.range}, comp_ {sv.comp_}, op {sv.op} {
            pointers = std::move (sv.pointers);
            sv.range = nullptr;
            sv.op.base_ = nullptr;
        }

    constexpr auto begin () {
        check_resort ();
        return view_iterator (&(*range->begin ()), &(*pointers.begin ()));
    }

    constexpr auto end () {
        return view_iterator (&(*range->begin ()), &(*pointers.end ()));
    }

    constexpr void merge_from_back () {
        auto old_pointers_size = pointers.size ();
        update_pointers (true);
        const auto sorted_offset = pointers.begin () + old_pointers_size;
        std::sort (sorted_offset, pointers.end (), op);
        std::inplace_merge (pointers.begin (), sorted_offset, pointers.end (), op);
    }

    constexpr void resort () {
        update_pointers ();
        std::ranges::sort (pointers, op);
    }

    constexpr void check_resort () {
        if ((pointers.size () != range->size () || op.base_ != &(*range->begin ())) || !std::ranges::is_sorted (pointers, op)) {
            update_pointers ();
            std::ranges::sort (pointers, op);
        }
    }
    
    
    constexpr Item &operator [] (long i) const {
        return *(&(*range->begin ()) + pointers [i]);
    } 

    constexpr Item &at (long i) {
        check_resort ();
        return *(&(*range->begin ()) + pointers.at (i));
    }

    struct view_iterator {

        using difference_type   = std::ptrdiff_t;
        using value_type        = Item;
        using pointer           = Item*;
        using reference         = Item&;

        constexpr view_iterator() = default;
        constexpr view_iterator(pointer base, long *offsets_pointer): 
            base_ {base}, offsets_ptr {offsets_pointer} {}

        constexpr reference operator* () const {return *(base_ + *offsets_ptr);}
        constexpr pointer operator-> () const {return base_ + *offsets_ptr;}

        constexpr view_iterator& operator++ () {offsets_ptr ++; return *this;}  
        constexpr view_iterator operator++ (int) {const view_iterator tmp = *this; offsets_ptr ++; return tmp;}

        constexpr view_iterator operator+ (long n) const {return {base_, offsets_ptr + n};}  
        constexpr view_iterator operator- (long n) const {return {base_, offsets_ptr - n};}  

        constexpr view_iterator &operator+= (long n) {offsets_ptr += n; return *this;}  
        constexpr view_iterator &operator-= (long n) {offsets_ptr -= n; return *this;}  

        constexpr view_iterator& operator-- () {offsets_ptr --; return *this;}  
        constexpr view_iterator operator-- (int) {const view_iterator tmp = *this; offsets_ptr --; return tmp;}

        constexpr reference operator[] (unsigned long n) const {return *(base_ + offsets_ptr + n);}

        friend constexpr bool operator > (const view_iterator& a, const view_iterator& b) {return a.offsets_ptr > b.offsets_ptr;};
        friend constexpr bool operator < (const view_iterator& a, const view_iterator& b) {return a.offsets_ptr < b.offsets_ptr;};
        friend constexpr bool operator >= (const view_iterator& a, const view_iterator& b) {return a.offsets_ptr >= b.offsets_ptr;};
        friend constexpr bool operator <= (const view_iterator& a, const view_iterator& b) {return a.offsets_ptr <= b.offsets_ptr;};
        friend constexpr bool operator== (const view_iterator& a, const view_iterator& b) {return a.offsets_ptr == b.offsets_ptr;};
        friend constexpr bool operator!= (const view_iterator& a, const view_iterator& b) {return a.offsets_ptr != b.offsets_ptr;};     
        friend constexpr difference_type operator- (const view_iterator &a, const view_iterator& b) { return a.offsets_ptr - b.offsets_ptr;};   
        friend constexpr view_iterator operator+ (long n, const view_iterator& a) {return a + n;};     


    private:
        pointer base_;
        long *offsets_ptr;

    };  

};

#endif
