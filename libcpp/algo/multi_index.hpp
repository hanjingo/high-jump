/*
 *  This file is part of libcpp.
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

#include <type_traits>

#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace libcpp {

namespace detail {
template <typename MemberType, auto MemberPtr, typename Tag>
struct unique_index_impl
{
    using member_type = MemberType;
    static constexpr auto member_ptr = MemberPtr;
    using tag_type = Tag;
    static constexpr bool is_unique = true;
};

template <typename MemberType, auto MemberPtr, typename Tag>
struct nonunique_index_impl
{
    using member_type = MemberType;
    static constexpr auto member_ptr = MemberPtr;
    using tag_type = Tag;
    static constexpr bool is_unique = false;
};

template <typename IndexConfig, typename Class>
struct make_index;

template <typename MemberType, auto MemberPtr, typename Tag, typename Class>
struct make_index<unique_index_impl<MemberType, MemberPtr, Tag>, Class>
{
    using type = boost::multi_index::ordered_unique<
        boost::multi_index::tag<Tag>,
        boost::multi_index::member<Class, MemberType, MemberPtr>>;
};

template <typename MemberType, auto MemberPtr, typename Tag, typename Class>
struct make_index<nonunique_index_impl<MemberType, MemberPtr, Tag>, Class>
{
    using type = boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<Tag>,
        boost::multi_index::member<Class, MemberType, MemberPtr>>;
};

template <typename Class, auto MemberPtr>
using member_type_t = std::remove_cv_t<
    std::remove_reference_t<decltype(std::declval<Class>().*MemberPtr)>>;

}  // namespace detail


template <typename MemberType, auto MemberPtr, typename Tag>
using unique_index = detail::unique_index_impl<MemberType, MemberPtr, Tag>;

template <typename MemberType, auto MemberPtr, typename Tag>
using nonunique_index =
    detail::nonunique_index_impl<MemberType, MemberPtr, Tag>;

template <typename Class, typename... IndexConfigs>
using multi_index = boost::multi_index::multi_index_container<
    Class,
    boost::multi_index::indexed_by<
        typename detail::make_index<IndexConfigs, Class>::type...>>;


#define LIBCPP_UNIQUE_INDEX(member_type, member_ptr, tag) \
    libcpp::unique_index<member_type, member_ptr, tag>,

#define LIBCPP_NON_UNIQUE_INDEX(member_type, member_ptr, tag) \
    libcpp::nonunique_index<member_type, member_ptr, tag>

#define LIBCPP_INDEX_TAG(tag) \
    struct tag                \
    {                         \
    };

#define LIBCPP_INDEX_TAGS_1(_1) LIBCPP_INDEX_TAG(_1)

#define LIBCPP_INDEX_TAGS_2(_1, _2) LIBCPP_INDEX_TAG(_1) LIBCPP_INDEX_TAG(_2)

#define LIBCPP_INDEX_TAGS_3(_1, _2, _3) \
    LIBCPP_INDEX_TAG(_1) LIBCPP_INDEX_TAG(_2) LIBCPP_INDEX_TAG(_3)

#define LIBCPP_INDEX_TAGS_4(_1, _2, _3, _4) \
    LIBCPP_INDEX_TAG(_1)                    \
    LIBCPP_INDEX_TAG(_2) LIBCPP_INDEX_TAG(_3) LIBCPP_INDEX_TAG(_4)

#define LIBCPP_INDEX_TAGS_5(_1, _2, _3, _4, _5) \
    LIBCPP_INDEX_TAG(_1)                        \
    LIBCPP_INDEX_TAG(_2)                        \
    LIBCPP_INDEX_TAG(_3) LIBCPP_INDEX_TAG(_4) LIBCPP_INDEX_TAG(_5)

#define LIBCPP_INDEX_TAGS_6(_1, _2, _3, _4, _5, _6) \
    LIBCPP_INDEX_TAG(_1)                            \
    LIBCPP_INDEX_TAG(_2)                            \
    LIBCPP_INDEX_TAG(_3)                            \
    LIBCPP_INDEX_TAG(_4) LIBCPP_INDEX_TAG(_5) LIBCPP_INDEX_TAG(_6)

#define LIBCPP_INDEX_TAGS_7(_1, _2, _3, _4, _5, _6, _7) \
    LIBCPP_INDEX_TAG(_1)                                \
    LIBCPP_INDEX_TAG(_2)                                \
    LIBCPP_INDEX_TAG(_3)                                \
    LIBCPP_INDEX_TAG(_4)                                \
    LIBCPP_INDEX_TAG(_5) LIBCPP_INDEX_TAG(_6) LIBCPP_INDEX_TAG(_7)

#define LIBCPP_INDEX_TAGS_8(_1, _2, _3, _4, _5, _6, _7, _8) \
    LIBCPP_INDEX_TAG(_1)                                    \
    LIBCPP_INDEX_TAG(_2)                                    \
    LIBCPP_INDEX_TAG(_3)                                    \
    LIBCPP_INDEX_TAG(_4)                                    \
    LIBCPP_INDEX_TAG(_5)                                    \
    LIBCPP_INDEX_TAG(_6) LIBCPP_INDEX_TAG(_7) LIBCPP_INDEX_TAG(_8)

#define LIBCPP_INDEX_TAGS_9(_1, _2, _3, _4, _5, _6, _7, _8, _9) \
    LIBCPP_INDEX_TAG(_1)                                        \
    LIBCPP_INDEX_TAG(_2)                                        \
    LIBCPP_INDEX_TAG(_3)                                        \
    LIBCPP_INDEX_TAG(_4)                                        \
    LIBCPP_INDEX_TAG(_5)                                        \
    LIBCPP_INDEX_TAG(_6)                                        \
    LIBCPP_INDEX_TAG(_7) LIBCPP_INDEX_TAG(_8) LIBCPP_INDEX_TAG(_9)

#define LIBCPP_INDEX_TAGS_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    LIBCPP_INDEX_TAG(_1)                                              \
    LIBCPP_INDEX_TAG(_2)                                              \
    LIBCPP_INDEX_TAG(_3)                                              \
    LIBCPP_INDEX_TAG(_4)                                              \
    LIBCPP_INDEX_TAG(_5)                                              \
    LIBCPP_INDEX_TAG(_6)                                              \
    LIBCPP_INDEX_TAG(_7)                                              \
    LIBCPP_INDEX_TAG(_8) LIBCPP_INDEX_TAG(_9) LIBCPP_INDEX_TAG(_10)

#define LIBCPP_GET_MAKE_TAGS_MACRO(_1,   \
                                   _2,   \
                                   _3,   \
                                   _4,   \
                                   _5,   \
                                   _6,   \
                                   _7,   \
                                   _8,   \
                                   _9,   \
                                   _10,  \
                                   NAME, \
                                   ...)  \
    NAME
#define LIBCPP_INDEX_TAGS(...)                       \
    LIBCPP_GET_MAKE_TAGS_MACRO(__VA_ARGS__,          \
                               LIBCPP_INDEX_TAGS_10, \
                               LIBCPP_INDEX_TAGS_9,  \
                               LIBCPP_INDEX_TAGS_8,  \
                               LIBCPP_INDEX_TAGS_7,  \
                               LIBCPP_INDEX_TAGS_6,  \
                               LIBCPP_INDEX_TAGS_5,  \
                               LIBCPP_INDEX_TAGS_4,  \
                               LIBCPP_INDEX_TAGS_3,  \
                               LIBCPP_INDEX_TAGS_2,  \
                               LIBCPP_INDEX_TAGS_1)(__VA_ARGS__)

}  // namespace libcpp

#endif  // MULTI_INDEX_HPP