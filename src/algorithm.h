#pragma once

#include <vector>
#include <type_traits>
#include <iterator>

namespace kab_advent {
	template<typename RangeT, typename PredicateT, 
		typename T = std::remove_cv_t<
			std::remove_reference_t< decltype( std::declval<RangeT>()[0] ) >
		>
	>
	std::vector<T> filter( const RangeT & range, PredicateT predicate ) {
		using std::begin; using std::end;
		std::vector<T> out;
		std::copy_if( begin( range ), end( range ), std::back_inserter( out ), predicate );
		return out;
	}

#   define KAB_ITERATOR_CATEGORY_REQUIRES(required_category, current_category) \
    typename = std::enable_if_t<std::is_base_of<required_category, iterator_category>::value>

    class default_sentinel {

    };

    template<typename RangeT>
    class circular_view {
        using iterator = decltype(std::begin(std::declval<RangeT>()));
    public:
        circular_view(RangeT range)
            : m_begin(std::begin(range)) 
            , m_end(std::end(range)) {

        }

        class circular_iterator {
        public:
            using value_type = typename std::iterator_traits<iterator>::value_type;
            using difference_type = typename std::iterator_traits<iterator>::difference_type;
            using reference = typename std::iterator_traits<iterator>::reference;
            using pointer = typename std::iterator_traits<iterator>::pointer;
            using iterator_category = typename std::iterator_traits<iterator>::iterator_category;

            static_assert(std::is_base_of<std::forward_iterator_tag, iterator_category>::value, "Requires forward iterators");

            circular_iterator() = default;
            circular_iterator(iterator begin, iterator end)
                : m_begin(begin)
                , m_end(end)
                , m_current(m_begin)
                , m_count(0) {

            }

            auto operator*() const -> reference {
                return *m_current;
            }

            auto operator++() -> circular_iterator & {
                ++m_current;
                ++m_count;
                if(m_current == m_end) {
                    m_current = m_begin;
                }
                return *this;
            }

            auto operator++(int) -> circular_iterator {
                auto copy = circular_iterator(*this);
                ++(*this);
                return copy;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::bidirectional_iterator_tag, IteratorCategory)>
            auto operator--() -> circular_iterator & {
                if(m_current == m_begin) {
                    m_current = m_end - 1;
                } else {
                    --m_current;
                }
                --m_count;
                return *this;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::bidirectional_iterator_tag, IteratorCategory)>
            auto operator--(int) -> circular_iterator {
                auto copy = circular_iterator(*this);
                --(*this);
                return copy;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator+(difference_type n) const -> circular_iterator {
                auto ret = circular_iterator(*this);
                return ret += n;;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator+=(difference_type n) -> circular_iterator & {
                if(n >= 0) while(n--) ++(*this);
                else while(n++) --(*this);
                return *this;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator-(difference_type n) const -> circular_iterator {
                return *this + -n;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator-(circular_iterator rhs) const -> difference_type {
                auto copy = *this;
                auto n = 0;
                while(copy++ != rhs) { ++n; }
                return n;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator-=(difference_type n) -> circular_iterator & {
                return *this += -n;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator[](difference_type n) -> reference {
                return *(*this + n);
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator<(circular_iterator other) const -> bool {
                return m_count < other.m_count;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator>(circular_iterator other) const -> bool {
                return other < *this;
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator>=(circular_iterator other) const -> bool {
                return !(*this < other);
            }

            template<typename IteratorCategory = iterator_category, KAB_ITERATOR_CATEGORY_REQUIRES(std::random_access_iterator_tag, IteratorCategory)>
            auto operator<=(circular_iterator other) const -> bool {
                return !(*this > other);
            }

            auto operator!=(default_sentinel) const -> bool {
                return false;
            }

            auto operator!=(circular_iterator rhs) const -> bool {
                return !(*this == rhs);
            }

            auto operator==(circular_iterator rhs) const -> bool {
                return m_count == rhs.m_count;
            }

        private:
            iterator m_begin;
            iterator m_end;
            iterator m_current;
            difference_type m_count;
        };
        
        auto begin() const noexcept -> circular_iterator {
            return {m_begin, m_end};
        }

        auto end() const noexcept -> default_sentinel {
            return {};
        }
    
    private:
        iterator m_begin;
        iterator m_end;
    };

    template<typename RangeT>
    auto make_circular_view(RangeT const& range) -> circular_view<RangeT const&> {
        return circular_view<RangeT const&>(range);
    }

    template<typename RangeT>
    auto make_circular_view(RangeT & range) -> circular_view<RangeT &> {
        return circular_view<RangeT &>(range);
    }

#undef KAB_ITERATOR_CATEGORY_REQUIRES
}