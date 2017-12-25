#pragma once

template<typename T, typename Alloc_>
class linked_list
{
private:
    friend class iterator;

    struct node {
        T value;
        node* next;

        node(const T &value_, node* next_) : value(value_), next(next_) {}
    };
    typename Alloc_::template rebind<node>::other allocator;

    node* root;

    void destroy_node(node* n)
    {
        allocator.destroy(n);
        allocator.deallocate(n, 1);
    }

public:

    class iterator
    {
    private:
        friend class linked_list;

        node* n;

        iterator(node* n_) : n(n_) {}

    public:
        iterator(const iterator &it) : n(it.n) {}
        ~iterator() { }

        T& operator*()
        {
            return n->value;
        }
        void operator++()
        {
            n = n->next;
        }
        bool operator!=(iterator it)
        {
            return n != it.n;
        }
        bool operator==(iterator it)
        {
            return n == it.n;
        }
    };

    linked_list() : root(nullptr) {}
    ~linked_list()
    {
        clear();
    };

    iterator begin()
    {
        return iterator(root);
    }
    iterator end()
    {
        return iterator(nullptr);
    }
    size_t size()
    {
        size_t count = 0;
        for(node* n = root; n != nullptr; n = n->next)
            ++count;
        return count;
    }

    void push_front(const T &v)
    {
        node* n = allocator.allocate(1);
        allocator.construct(n, v, root);
        root = n;
    }
    void push_back(const T &v)
    {
        if(begin() == end())
            push_front(v);
        else {
            node* p = root;
            while(p->next != nullptr)
                p = p->next;
            node* n = allocator.allocate(1);
            allocator.construct(n, v, nullptr);
            p->next = n;
        }
    }
    void erase(iterator &it)
    {
        if(it == begin()) {
            node* n = it.n;
            ++it;
            root = it.n;
            destroy_node(n);
        } else if(it != end()) {
            node* p = root;
            while(p != nullptr && p->next != it.n)
                p = p->next;
            if(p != nullptr) {
                node* n = it.n;
                ++it;
                p->next = it.n;
                destroy_node(n);
            }
        }
    }
    void clear()
    {
        while(root != nullptr) {
            node* n = root->next;
            destroy_node(root);
            root = n;
        }
    }
};