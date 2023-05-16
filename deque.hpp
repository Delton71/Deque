#ifndef MYDEQUE_H
#define MYDEQUE_H

// #define MY_DEQUE_DEBUG

#include <new>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <math.h>
#include <utility>

#ifdef MY_DEQUE_DEBUG
#include <iostream>
#include <deque>
#endif // MY_DEQUE_DEBUG

template<typename T>
class Deque;

template<typename T, typename Ptr, typename Ref>
class DequeIterator
        : public std::iterator<std::random_access_iterator_tag, T> {
    friend class Deque<T>;
  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;

    typedef ptrdiff_t difference_type;
    typedef pointer* map_pointer;
    typedef DequeIterator<T, Ptr, Ref> self;

  public:
    DequeIterator() = default;
    ~DequeIterator() = default;

    void Clear() {
        curr_ = nullptr;
        first_ = nullptr;
        last_ = nullptr;
        owner_node_ = nullptr;
    }

    DequeIterator(const DequeIterator& it) noexcept
            : curr_(it.curr_)
            , first_(it.first_)
            , last_(it.last_)
            , owner_node_(it.owner_node_) {}
    DequeIterator(DequeIterator&& it) noexcept
            : curr_(it.curr_)
            , first_(it.first_)
            , last_(it.last_)
            , owner_node_(it.owner_node_) {
        it.Clear();
    }
    self& operator=(const self& it) noexcept {
        curr_ = it.curr_;
        first_ = it.first_;
        last_ = it.last_;
        owner_node_ = it.owner_node_;
        return *this;
    }
	self& operator=(self&& it) noexcept {
        curr_ = std::exchange(it.curr_, nullptr);
        first_ = std::exchange(it.first_, nullptr);
        last_ = std::exchange(it.last_, nullptr);
        owner_node_ = std::exchange(it.owner_node_, nullptr);
        return *this;
    }

    // NON-CONST -> CONST
    DequeIterator(T* ptr, T** owner) noexcept
            : curr_(ptr)
            , first_(*owner)
            , last_(*owner + Deque<T>::kInitBuffSize)
            , owner_node_(nullptr) {
        owner_node_ = const_cast<map_pointer>(owner);
    }
    constexpr operator DequeIterator<T, const T*, const T&>() const {
        return DequeIterator<T, const T*, const T&>(curr_, owner_node_);
    }

    self& operator++() noexcept {
        ++curr_;
        if (curr_ == last_) {
            SetOwnerNode(owner_node_ + 1);
            curr_ = first_;
        }
        return *this;
    }
    self& operator--() noexcept {
        if (curr_ == first_) {
            SetOwnerNode(owner_node_ - 1);
            curr_ = last_;
        }
        --curr_;
        return *this;
    }
    self& operator+=(const difference_type val) noexcept {
        difference_type offset = val + (curr_ - first_);
        // Same node
        if (offset >= 0 && offset < difference_type(Deque<T>::kInitBuffSize)) {
            curr_ += val;
            return *this;
        }

        // Other node
        difference_type node_offset = (offset > 0)
                    ? offset / difference_type(Deque<T>::kInitBuffSize)
                    : -difference_type((-offset - 1) / Deque<T>::kInitBuffSize) - 1;
        SetOwnerNode(owner_node_ + node_offset);
        curr_ = first_ + (offset - node_offset * difference_type(Deque<T>::kInitBuffSize));
        return *this;
    }
    self& operator-=(const difference_type val) noexcept {
        return *this += -val;
    }
    self operator+(const difference_type val) const noexcept {
        self res = *this;
        return res += val;
    }
    self operator-(const difference_type val) const noexcept {
        self res = *this;
        return res -= val;
    }
    
    difference_type operator-(const DequeIterator& it) const noexcept {
        // Also works if nodes (!) are the same
        return ((owner_node_ - it.owner_node_ - 1) * 
                                difference_type(Deque<T>::kInitBuffSize)) +
               (curr_ - first_) + (it.last_ - it.curr_);
    }
    
    value_type operator*() const {
        if (curr_ == nullptr) {
            throw std::out_of_range("Iterator index out of range.");
        }
        return *curr_;
    }
    reference operator*() {
        if (curr_ == nullptr) {
            throw std::out_of_range("Iterator index out of range.");
        }
        return *curr_;
    }
    pointer operator->() {
        if (curr_ == nullptr) {
            throw std::out_of_range("Iterator index out of range.");
        }
        return curr_;
    }

    bool operator==(const self& it) const noexcept {
        return owner_node_ == it.owner_node_ && curr_ == it.curr_;
    }
    bool operator!=(const self& it) const noexcept {
        return !(*this == it);
    }
    bool operator<(const self& it) const noexcept {
        return (owner_node_ == it.owner_node_)? (curr_ < it.curr_)
                                              : (owner_node_ < it.owner_node_);
    }
    bool operator>(const self& it) const noexcept {
        return (owner_node_ == it.owner_node_)? (curr_ > it.curr_)
                                              : (owner_node_ > it.owner_node_);
    }
    bool operator<=(const self& it) const noexcept {
        return !(*this > it);
        // return ((*this < it) || (*this == it));
    }
    bool operator>=(const self& it) const noexcept {
        return !(*this < it);
        // return ((*this > it) || (*this == it));
    }

  private:
    pointer curr_{nullptr};
    pointer first_{nullptr};
    pointer last_{nullptr};
    map_pointer owner_node_{nullptr};

    void SetOwnerNode(map_pointer new_node) {
        owner_node_ = new_node;
        first_ = *new_node;
        last_ = first_ + Deque<T>::kInitBuffSize;
    }
    
    reference operator[](difference_type ind) const {
        return *(*this + ind);
    }
};

template<typename T>
class Deque {
    friend class DequeIterator<T, T*, T&>;
    friend class DequeIterator<T, const T*, const T&>;
  public:
    typedef T* pointer;
    typedef T& reference;

    typedef DequeIterator<T, T*, T&> iterator;
    typedef DequeIterator<T, const T*, const T&> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef ptrdiff_t difference_type;
    typedef pointer* map_pointer;
    std::allocator<T> data_allocator;
    std::allocator<pointer> map_allocator;

  public:
    #ifdef MY_DEQUE_DEBUG
    void PrintDeque() {
        for (iterator it = start_; it != finish_; ++it) {
            std::cout << *it << ' ';
        }
        std::cout << "\n---------------\n";
    }
    #endif // MY_DEQUE_DEBUG

    Deque() noexcept
            : start_()
            , finish_()
            , map_(nullptr) {
        CreateMapAndNodes(0);
    }
    Deque(int64_t size) noexcept
            : start_()
            , finish_()
            , map_(nullptr) {
        CreateMapAndNodes(size);
    }
    Deque(const Deque& deq) noexcept
            : start_()
            , finish_()
            , map_(nullptr) {
        map_ = map_allocator.allocate(deq.map_size_);
        map_size_ = deq.map_size_;

        map_pointer start_ptr = map_ + (deq.start_.owner_node_ - deq.map_);
        map_pointer finish_ptr = map_ + (deq.finish_.owner_node_ - deq.map_);

        for (map_pointer this_ptr = start_ptr, other_ptr = deq.start_.owner_node_;
                        other_ptr <= deq.finish_.owner_node_; ++this_ptr, ++other_ptr) {
            *this_ptr = AllocateNode();
            std::copy(*other_ptr, *other_ptr + kInitBuffSize, *this_ptr);
        }

        start_.SetOwnerNode(start_ptr);
        start_.curr_ = start_.first_ + (deq.start_.curr_ - deq.start_.first_);
        finish_.SetOwnerNode(finish_ptr);
        finish_.curr_ = finish_.first_ + (deq.finish_.curr_ - deq.finish_.first_);
    }
    Deque(Deque&& deq) noexcept
            : start_(std::move(deq.start_))
            , finish_(std::move(deq.finish_))
            , map_(deq.map_)
            , map_size_(deq.map_size_) {
        deq.map_ = nullptr;
        deq.map_size_ = 0;
    }
    Deque(int64_t size, const T& val) noexcept
            : start_()
            , finish_()
            , map_(nullptr) {
        CreateMapAndNodes(size);
        for (map_pointer curr_node = start_.owner_node_; 
                        curr_node < finish_.owner_node_; ++curr_node) {
            std::fill(*curr_node, *curr_node + kInitBuffSize, val);
        }
        std::fill(*(finish_.owner_node_), 
                *(finish_.owner_node_) + (size % kInitBuffSize), val);
    }
    Deque(std::initializer_list<T> val_list) noexcept
            : start_()
            , finish_()
            , map_(nullptr) {
        CreateMapAndNodes(val_list.size());
        auto val_list_it = val_list.begin();
        for (map_pointer curr_node = start_.owner_node_; 
                        curr_node < finish_.owner_node_; ++curr_node) {
            std::copy(val_list_it, val_list_it + kInitBuffSize, *curr_node);
            val_list_it += kInitBuffSize;
        }
        std::copy(val_list_it, val_list.end(), *(finish_.owner_node_));
    }

    ~Deque() {
        Clear();
        map_allocator.deallocate(map_, map_size_);
    }

    Deque& operator=(const Deque& deq) {
		Deque temp(deq);
		std::swap(*this, temp);

        return *this;
    }
    Deque& operator=(Deque&& deq) noexcept {
		Clear();
        map_allocator.deallocate(map_, map_size_);

        start_ = std::move(deq.start_);
        finish_ = std::move(deq.finish_);
        map_ = deq.map_;
        map_size_ = deq.map_size_;        
        
		deq.map_ = nullptr;
        deq.map_size_ = 0;

        return *this;
    }
    Deque& operator=(std::initializer_list<T> val_list) noexcept {
        CreateMapAndNodes(val_list.size());
        auto val_list_it = val_list.begin();
        for (map_pointer curr_node = start_.owner_node_; 
                        curr_node < finish_.owner_node_; ++curr_node) {
            std::copy(val_list_it, val_list_it + kInitBuffSize, *curr_node);
            val_list_it += kInitBuffSize;
        }
        std::copy(val_list_it, val_list.end(), *(finish_.owner_node_));

        return *this;
    }

    // Actually always shrinked :/
    void shrink_to_fit() noexcept {}

    int64_t size() const noexcept {
        return int64_t(finish_ - start_);
    }
    bool empty() const noexcept {
        return start_ == finish_;
    }

    reference operator[](int64_t ind) noexcept {
        return start_[difference_type(ind)];
    }
    const T& operator[](int64_t ind) const noexcept {
        return start_[difference_type(ind)];
    }
    reference at(int64_t ind) {
        if (ind < 0 || ind >= size()) {
            throw std::out_of_range("Deque::at out of range!");
        }
        return start_[difference_type(ind)];
    }

    bool operator==(const Deque& deq) const {
        if (size() != deq.size()) {
            return false;
        }
        for (int64_t i = 0; i < size(); ++i) {
            if ((*this)[i] != deq[i]) {
                return false;
            }
        }
        return true;
    }
    #ifdef MY_DEQUE_DEBUG
    bool operator==(const std::deque<T>& deq) const {
        if (static_cast<size_t>(size()) != deq.size()) {
            return false;
        }
        for (int64_t i = 0; i < size(); ++i) {
            if ((*this)[i] != deq[i]) {
                return false;
            }
        }
        return true;
    }
    #endif // MY_DEQUE_DEBUG

    reference front() {
        if (empty()) {
            throw std::runtime_error("Deque::front error: deque is empty!");
        }
        return *start_;
    }
    const T& front() const {
        if (empty()) {
            throw std::runtime_error("Deque::front error: deque is empty!");
        }
        return *start_;
    }
    reference back() {
        if (empty()) {
            throw std::runtime_error("Deque::back error: deque is empty!");
        }
        iterator temp = finish_;
        return *(--temp);
    }
    const T& back() const {
        if (empty()) {
            throw std::runtime_error("Deque::back error: deque is empty!");
        }
        iterator temp = finish_;
        return *(--temp);
    }

    iterator begin() noexcept {
        return start_;
    }
    const_iterator begin() const noexcept {
        return const_iterator(start_.curr_, start_.owner_node_);
    }
    const_iterator cbegin() const noexcept {
        return const_iterator(start_.curr_, start_.owner_node_);
    }
    iterator end() noexcept {
        return finish_;
    }
    const_iterator end() const noexcept {
        return const_iterator(finish_.curr_, finish_.owner_node_);
    }
    const_iterator cend() const noexcept {
        return const_iterator(finish_.curr_, finish_.owner_node_);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(cend());
    }
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(cbegin());
    }
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    void push_front(const T& val) noexcept {
        if (start_.curr_ != start_.first_) {
            data_allocator.construct(--start_.curr_, val);
        } else {
            ReserveMapInFront();
            *(start_.owner_node_ - 1) = AllocateNode();
            start_.SetOwnerNode(start_.owner_node_ - 1);
            start_.curr_ = start_.last_ - 1;
            data_allocator.construct(start_.curr_, val);
        }
    }
    void push_front(T&& val) noexcept {
        if (start_.curr_ != start_.first_) {
            data_allocator.construct(--start_.curr_, std::move(val));
        } else {
            ReserveMapInFront();
            *(start_.owner_node_ - 1) = AllocateNode();
            start_.SetOwnerNode(start_.owner_node_ - 1);
            start_.curr_ = start_.last_ - 1;
            data_allocator.construct(start_.curr_, std::move(val));
        }
    }
    void push_back(const T& val) noexcept {
        if (finish_.curr_ != finish_.last_ - 1) {
            data_allocator.construct(finish_.curr_, val);
            ++finish_.curr_;
        } else {
            ReserveMapInBack();
            *(finish_.owner_node_ + 1) = AllocateNode();
            map_allocator.construct(finish_.curr_, val);
            finish_.SetOwnerNode(finish_.owner_node_ + 1);
            finish_.curr_ = finish_.first_;
        }
    }
    void push_back(T&& val) noexcept {
        if (finish_.curr_ != finish_.last_ - 1) {
            data_allocator.construct(finish_.curr_, std::move(val));
            ++finish_.curr_;
        } else {
            ReserveMapInBack();
            *(finish_.owner_node_ + 1) = AllocateNode();
            map_allocator.construct(finish_.curr_, std::move(val));
            finish_.SetOwnerNode(finish_.owner_node_ + 1);
            finish_.curr_ = finish_.first_;
        }
    }

    void pop_front() {
        if (empty()) {
            throw std::runtime_error("Deque::pop_front error: deque is empty!");
        }
        if (start_.curr_ != start_.last_ - 1) {
            data_allocator.destroy(start_.curr_);
            ++start_.curr_;
        } else {
            data_allocator.destroy(start_.curr_);
            DeallocateNode(start_.first_);
            start_.SetOwnerNode(start_.owner_node_ + 1);
            start_.curr_ = start_.first_;
        }
    }
    void pop_back() {
        if (empty()) {
            throw std::runtime_error("Deque::pop_back error: deque is empty!");
        }
        if (finish_.curr_ != finish_.first_) {
            data_allocator.destroy(finish_.curr_);
            --finish_.curr_;
        } else {
            DeallocateNode(finish_.first_);
            finish_.SetOwnerNode(finish_.owner_node_ - 1);
            finish_.curr_ = finish_.last_ - 1;
            data_allocator.destroy(finish_.curr_);
        }
    }

    void insert(iterator pos, const T& val) noexcept {
        if (pos == start_) {
            push_front(val);
            return;
        }
        if (pos == finish_) {
            push_back(val);
            return;
        }

        difference_type ind = pos - start_;
        if (ind < (size() >> 1)) {
            // First half
            push_front(front());
            iterator front_1 = start_;
            ++front_1;
            iterator front_2 = front_1;
            ++front_2;

            pos = start_ + ind;
            iterator pos_1 = pos;
            ++pos_1;
            std::copy(front_2, pos_1, front_1);
        } else {
            // Second half
            push_back(back());
            iterator back_1 = finish_;
            --back_1;
            iterator back_2 = back_1;
            --back_2;

            pos = start_ + ind;
            std::copy_backward(pos, back_2, back_1);
        }

        *pos = val;
    }
    void insert(iterator pos, T&& val) noexcept {
        if (pos == start_) {
            push_front(std::move(val));
            return;
        }
        if (pos == finish_) {
            push_back(std::move(val));
            return;
        }

        difference_type ind = pos - start_;
        if (ind < (size() >> 1)) {
            // First half
            push_front(front());
            iterator front_1 = start_;
            ++front_1;
            iterator front_2 = front_1;
            ++front_2;

            pos = start_ + ind;
            iterator pos_1 = pos;
            ++pos_1;
            std::copy(front_2, pos_1, front_1);
        } else {
            // Second half
            push_back(back());
            iterator back_1 = finish_;
            --back_1;
            iterator back_2 = back_1;
            --back_2;

            pos = start_ + ind;
            std::copy_backward(pos, back_2, back_1);
        }
        
        *pos = std::move(val);
    }
    void insert(iterator pos, std::initializer_list<T> val_list) noexcept {
        difference_type val_list_size = difference_type(val_list.size());
        if (pos == start_) {
            for (size_t i = 0; i < val_list.size(); ++i) {
                push_front(0);
            }
            std::copy(val_list.begin(), val_list.end(), start_);
            return;
        }
        if (pos == finish_) {
            for (size_t i = 0; i < val_list.size(); ++i) {
                push_back(0);
            }
            std::copy(val_list.begin(), val_list.end(), finish_ - (val_list_size - 1));
            return;
        }

        difference_type mid_ind = pos + (val_list_size >> 1) - start_;
        difference_type ind = pos - start_;
        if (mid_ind < (size() >> 1)) {
            for (size_t i = 0; i < val_list.size(); ++i) {
                push_front(*(start_ + (val_list_size - 1)));
            }

            iterator front_len = start_ + val_list_size;
            iterator front_len_2 = front_len + val_list_size;
            pos = start_ + ind;
            iterator pos_len = pos + val_list_size;
            std::copy(front_len_2, pos_len, front_len);
        } else {
            for (size_t i = 0; i < val_list.size(); ++i) {
                push_back(*(finish_ - val_list_size));
            }

            iterator back_len = finish_ - val_list_size;
            iterator back_len_2 = back_len - val_list_size;
            pos = start_ + ind;
            std::copy_backward(pos, back_len_2, back_len);
        }

        std::copy(val_list.begin(), val_list.end(), pos);
    }
    void insert(const_iterator c_pos, const T& val) noexcept {
        iterator pos = c_pos;
        insert(pos, val);
    }
    void insert(const_iterator c_pos, T&& val) noexcept {
        iterator pos = c_pos;
        insert(pos, std::move(val));
    }
    void insert(const_iterator c_pos, std::initializer_list<T> val_list) noexcept {
        iterator pos = c_pos;
        insert(pos, val_list);
    }

    void erase(iterator pos) {
        iterator next = pos;
        ++next;
        difference_type ind = pos - start_;
        if (ind < difference_type(size() >> 1)) {
            // First half
            std::copy_backward(start_, pos, next);
            pop_front();
        } else {
            // Second half
            std::copy(next, finish_, pos);
            pop_back();
        }
    }
    void erase(const_iterator c_pos) {
        iterator pos = c_pos;
        erase(pos);
    }

    void erase(iterator from, iterator to) {
        if (from == start_ && to == finish_) {
            Clear();
            return;
        }
        difference_type erase_size = to - from;
        difference_type elems_before = from - start_;
        if (elems_before < difference_type((size() - erase_size) >> 1)) {
            std::copy_backward(start_, from, to);
            iterator new_start = start_ + erase_size;
            for (map_pointer curr_node = start_.owner_node_; 
                            curr_node < new_start.owner_node_; ++curr_node) {
                for (int64_t i = 0; i <= kInitBuffSize; ++i) {
                    data_allocator.destroy(*curr_node + i);
                }
                data_allocator.deallocate(*curr_node, kInitBuffSize);
            }
            start_ = new_start;
        } else {
            std::copy(to, finish_, from);
            iterator new_finish = finish_ - erase_size;
            for (map_pointer curr_node = new_finish.owner_node_ + 1;
                            curr_node <= finish_.owner_node_; ++curr_node) {
                for (int64_t i = 0; i <= kInitBuffSize; ++i) {
                    data_allocator.destroy(*curr_node + i);
                }
                data_allocator.deallocate(*curr_node, kInitBuffSize);
            }
            finish_ = new_finish;
        }
    }

  private:
    iterator start_;
    iterator finish_;
    map_pointer map_{nullptr};
    int64_t map_size_{0};

    static constexpr int64_t kInitMapSize = 16;
    static constexpr int64_t kInitBuffSize = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;

    pointer AllocateNode() {
        return data_allocator.allocate(kInitBuffSize);
    }
    void DeallocateNode(pointer node) {
        data_allocator.deallocate(node, kInitBuffSize);
    }
    void CreateMapAndNodes(int64_t elems_size) {
        int64_t nodes_size = elems_size / kInitBuffSize + 1;

        // (+ 2) Begin and last will be allocate more memory
        // to save time for inserting elements in deque.
        map_size_ = std::max(kInitMapSize, nodes_size + 2);
        map_ = map_allocator.allocate(map_size_);

        // Aligning start_ & finish_
        map_pointer start_ptr = map_ + ((map_size_ - nodes_size) >> 1);
        map_pointer finish_ptr = start_ptr + nodes_size - 1;
        for (map_pointer curr_ptr = start_ptr; curr_ptr <= finish_ptr; ++curr_ptr) {
            *curr_ptr = AllocateNode();
            std::memset(*curr_ptr, 0, sizeof(T) * kInitBuffSize);
        }
        start_.SetOwnerNode(start_ptr);
        finish_.SetOwnerNode(finish_ptr);
        start_.curr_ = start_.first_;
        finish_.curr_ = finish_.first_ + (elems_size % kInitBuffSize);
    }
    void ReallocateMap(int64_t add_nodes_size, bool is_in_front) {
        int64_t old_nodes_size = finish_.owner_node_ - start_.owner_node_ + 1;
        int64_t new_nodes_size = old_nodes_size + add_nodes_size;
        map_pointer start_ptr;
        if (map_size_ >= new_nodes_size + 2) {
            // Balancing map_size_
            start_ptr = map_ + ((map_size_ - new_nodes_size) >> 1) +
                        ((is_in_front)? add_nodes_size : 0);
            if (start_ptr < start_.owner_node_) {
                std::copy(start_.owner_node_, finish_.owner_node_ + 1, start_ptr);
            } else {
                std::copy_backward(start_.owner_node_, finish_.owner_node_ + 1, start_ptr + old_nodes_size);
            }
        } else {
            int64_t new_map_size = map_size_ + std::max(map_size_, add_nodes_size) + 2;
            // New memory allocation
            map_pointer new_map = map_allocator.allocate(new_map_size);
            start_ptr = new_map + ((new_map_size - new_nodes_size) >> 1) +
                        ((is_in_front)? add_nodes_size : 0);
            std::copy(start_.owner_node_, finish_.owner_node_ + 1, start_ptr);
            map_allocator.deallocate(map_, map_size_);
            map_ = new_map;
            map_size_ = new_map_size;
        }

        // Reset start_ & finish_
        start_.SetOwnerNode(start_ptr);
        finish_.SetOwnerNode(start_ptr + old_nodes_size - 1);
    }
    void ReserveMapInFront(int64_t add_nodes_size = 1) {
        if (add_nodes_size > start_.owner_node_ - map_) {
            ReallocateMap(add_nodes_size, true);
        }
    }
    void ReserveMapInBack(int64_t add_nodes_size = 1) {
        if (add_nodes_size > map_size_ - (finish_.owner_node_ - map_) - 1) {
            ReallocateMap(add_nodes_size, false);
        }
    }

    // Only one buffer will be left
    void Clear() {
        for (map_pointer curr_node = start_.owner_node_ + 1; 
                        curr_node < finish_.owner_node_; ++curr_node) {
            for (int64_t i = 0; i < kInitBuffSize; ++i) {
                data_allocator.destroy(*curr_node + i);
            }
            data_allocator.deallocate(*curr_node, kInitBuffSize);
        }

        if (start_.owner_node_ != finish_.owner_node_) {
            for (pointer pt = start_.curr_; pt != start_.last_; ++pt) {
                data_allocator.destroy(pt);
            }
            for (pointer pt = finish_.first_; pt != finish_.curr_; ++pt) {
                data_allocator.destroy(pt);
            }
            data_allocator.deallocate(finish_.first_, kInitBuffSize);
        } else {
            for (pointer pt = start_.curr_; pt != finish_.curr_; ++pt) {
                data_allocator.destroy(pt);
            }
        }

        data_allocator.deallocate(start_.first_, kInitBuffSize);
        start_.Clear();
        finish_.Clear();
    }
};

#endif /* MYDEQUE_H */