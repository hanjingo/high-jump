/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MULTI_INDEX_HPP
#define MULTI_INDEX_HPP

// -----------------------------------------------------------------------
// hj::multi_index — a thin, declaration-time convenience layer over
// boost::multi_index_container.
//
// Boost.MultiIndex is powerful but its `indexed_by<...>` declaration is
// verbose. This header lets you write:
//
//   HJ_INDEX_TAG(id_tag);
//   HJ_INDEX_TAG(name_tag);
//   HJ_INDEX_TAG(dept_name_tag);
//
//   struct employee { int id; std::string dept; std::string name; };
//
//   using employee_table = hj::multi_index<
//       employee,
//       HJ_UNIQUE_INDEX(id_tag, &employee::id),               // single field
//       HJ_NON_UNIQUE_INDEX(name_tag, &employee::name),       // single field
//       HJ_UNIQUE_INDEX(dept_name_tag, &employee::dept,
//                       &employee::name)>;                    // composite
//
// Supported index kinds: ordered_unique, ordered_non_unique, hashed_unique,
// hashed_non_unique. Each may be built from one member pointer (plain
// `member<>` key extractor, queryable with a bare value, e.g.
// `view.find(1)`) or several member pointers (a `composite_key<>` key
// extractor, queryable with `boost::make_tuple(v1, v2, ...)`).
//
// NOT supported (deliberately out of scope for now):
//   - member-function-based key extractors (const_mem_fun)
//   - sequenced_index / random_access_index / ranked_index
//   - custom comparators / hashers
// If you need any of the above, use boost::multi_index directly.
//
// Thread safety: identical to boost::multi_index_container itself — NOT
// thread-safe for concurrent mutation; concurrent reads are fine, readers
// concurrent with writers are not, external synchronization is required.
// -----------------------------------------------------------------------

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <cstddef>
#include <type_traits>

namespace hj
{

namespace detail
{

// ---- member pointer -> (class, member) type traits ----
template <typename T>
struct member_pointer_traits;

template <typename Class, typename Member>
struct member_pointer_traits<Member Class::*>
{
    using class_type  = Class;
    using member_type = Member;
};

enum class index_kind
{
    ordered_unique,
    ordered_non_unique,
    hashed_unique,
    hashed_non_unique
};

// ---- key extractor selection: single member -> member<>,
//      two or more members -> composite_key<member<>, member<>, ...> ----
template <typename Class, auto First, auto... Rest>
struct key_extractor
{
    static constexpr std::size_t count = 1 + sizeof...(Rest);

    using single_type = boost::multi_index::member<
        Class,
        typename member_pointer_traits<decltype(First)>::member_type,
        First>;

    using composite_type = boost::multi_index::composite_key<
        Class,
        boost::multi_index::member<
            Class,
            typename member_pointer_traits<decltype(First)>::member_type,
            First>,
        boost::multi_index::member<
            Class,
            typename member_pointer_traits<decltype(Rest)>::member_type,
            Rest>...>;

    // NOTE: both branches are always valid types to *form* (composite_key
    // with a single element is legal), so std::conditional_t here is safe
    // even though only one branch is ever actually used.
    using type = std::conditional_t<count == 1, single_type, composite_type>;
};

// ---- index_config: tag + kind + one-or-more member pointers ----
template <typename Tag, index_kind Kind, auto First, auto... Rest>
struct index_config
{
    using tag_type                   = Tag;
    static constexpr index_kind kind = Kind;

    // Kept for introspection / backward-compat convenience: the first
    // (or only) member pointer in the index.
    static constexpr auto member_ptr = First;

    static_assert(std::is_member_object_pointer_v<decltype(First)>
                      && (std::is_member_object_pointer_v<decltype(Rest)>
                          && ...),
                  "hj::multi_index: every argument must be a pointer to a "
                  "data member (member-function key extractors such as "
                  "const_mem_fun are not supported)");
};

// ---- index_config -> boost index specification ----
template <typename IndexConfig, typename Class>
struct make_index
{
    // Fires only if this primary template is ever instantiated, i.e. no
    // specialization below matched IndexConfig. Gives a readable error
    // instead of an "incomplete type" diagnostic when an index_kind is
    // added to the enum without a matching specialization here.
    static_assert(sizeof(Class) == 0,
                  "hj::multi_index: no make_index specialization for this "
                  "index_kind / index_config combination");
};

template <typename Tag, auto First, auto... Rest, typename Class>
struct make_index<index_config<Tag, index_kind::ordered_unique, First, Rest...>,
                  Class>
{
    using type = boost::multi_index::ordered_unique<
        boost::multi_index::tag<Tag>,
        typename key_extractor<Class, First, Rest...>::type>;
};

template <typename Tag, auto First, auto... Rest, typename Class>
struct make_index<
    index_config<Tag, index_kind::ordered_non_unique, First, Rest...>,
    Class>
{
    using type = boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<Tag>,
        typename key_extractor<Class, First, Rest...>::type>;
};

template <typename Tag, auto First, auto... Rest, typename Class>
struct make_index<index_config<Tag, index_kind::hashed_unique, First, Rest...>,
                  Class>
{
    using type = boost::multi_index::hashed_unique<
        boost::multi_index::tag<Tag>,
        typename key_extractor<Class, First, Rest...>::type>;
};

template <typename Tag, auto First, auto... Rest, typename Class>
struct make_index<
    index_config<Tag, index_kind::hashed_non_unique, First, Rest...>,
    Class>
{
    using type = boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<Tag>,
        typename key_extractor<Class, First, Rest...>::type>;
};

} // namespace detail

// ---- public index-kind aliases ----
// `MemberPtrs`: one member pointer for a plain single-field index, or two
// or more for a composite (multi-field) index.
template <typename Tag, auto... MemberPtrs>
using unique_index = detail::
    index_config<Tag, detail::index_kind::ordered_unique, MemberPtrs...>;

template <typename Tag, auto... MemberPtrs>
using non_unique_index = detail::
    index_config<Tag, detail::index_kind::ordered_non_unique, MemberPtrs...>;

template <typename Tag, auto... MemberPtrs>
using hashed_unique_index =
    detail::index_config<Tag, detail::index_kind::hashed_unique, MemberPtrs...>;

template <typename Tag, auto... MemberPtrs>
using hashed_non_unique_index = detail::
    index_config<Tag, detail::index_kind::hashed_non_unique, MemberPtrs...>;

// ---- the container alias itself ----
template <typename Class, typename... IndexConfigs>
using multi_index = boost::multi_index::multi_index_container<
    Class,
    boost::multi_index::indexed_by<
        typename detail::make_index<IndexConfigs, Class>::type...>>;

} // namespace hj

// -----------------------------------------------------------------------
// Convenience macros.
//
// BREAKING CHANGE vs. the previous version of this header: the argument
// order is now (tag, member_ptr...) instead of (member_ptr, tag). This is
// required so the macros can accept a variable number of member pointers
// for composite-key indices — a C-style variadic macro's `...` must be its
// last parameter, so the tag can no longer come last.
// -----------------------------------------------------------------------
#define HJ_UNIQUE_INDEX(tag, ...) hj::unique_index<tag, __VA_ARGS__>
#define HJ_NON_UNIQUE_INDEX(tag, ...) hj::non_unique_index<tag, __VA_ARGS__>
#define HJ_HASHED_UNIQUE_INDEX(tag, ...)                                       \
    hj::hashed_unique_index<tag, __VA_ARGS__>
#define HJ_HASHED_NON_UNIQUE_INDEX(tag, ...)                                   \
    hj::hashed_non_unique_index<tag, __VA_ARGS__>

#define HJ_INDEX_TAG(tag)                                                      \
    struct tag                                                                 \
    {                                                                          \
    }

#endif // MULTI_INDEX_HPP