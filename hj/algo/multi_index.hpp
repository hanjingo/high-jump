/*
 *  This file is part of hj.
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

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <type_traits>

namespace hj
{

namespace detail
{
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

template <typename IndexConfig, typename Class> struct make_index;

template <typename MemberType, auto MemberPtr, typename Tag, typename Class>
struct make_index<unique_index_impl<MemberType, MemberPtr, Tag>, Class>
{
    using type = boost::multi_index::ordered_unique<
      boost::multi_index::tag<Tag>,
      boost::multi_index::member<Class, MemberType, MemberPtr> >;
};

template <typename MemberType, auto MemberPtr, typename Tag, typename Class>
struct make_index<nonunique_index_impl<MemberType, MemberPtr, Tag>, Class>
{
    using type = boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Tag>,
      boost::multi_index::member<Class, MemberType, MemberPtr> >;
};

template <typename Class, auto MemberPtr>
using member_type_t = std::remove_cv_t<
  std::remove_reference_t<decltype (std::declval<Class> ().*MemberPtr)> >;

} // namespace detail

template <typename MemberType, auto MemberPtr, typename Tag>
using unique_index = detail::unique_index_impl<MemberType, MemberPtr, Tag>;

template <typename MemberType, auto MemberPtr, typename Tag>
using nonunique_index =
  detail::nonunique_index_impl<MemberType, MemberPtr, Tag>;

template <typename Class, typename... IndexConfigs>
using multi_index = boost::multi_index::multi_index_container<
  Class,
  boost::multi_index::indexed_by<
    typename detail::make_index<IndexConfigs, Class>::type...> >;

#define HJ_UNIQUE_INDEX(member_type, member_ptr, tag)                      \
    hj::unique_index<member_type, member_ptr, tag>,

#define HJ_NON_UNIQUE_INDEX(member_type, member_ptr, tag)                  \
    hj::nonunique_index<member_type, member_ptr, tag>

#define HJ_INDEX_TAG(tag)                                                  \
    struct tag                                                                 \
    {                                                                          \
    };

#define HJ_INDEX_TAGS_1(_1) HJ_INDEX_TAG (_1)

#define HJ_INDEX_TAGS_2(_1, _2) HJ_INDEX_TAG (_1) HJ_INDEX_TAG (_2)

#define HJ_INDEX_TAGS_3(_1, _2, _3)                                        \
    HJ_INDEX_TAG (_1) HJ_INDEX_TAG (_2) HJ_INDEX_TAG (_3)

#define HJ_INDEX_TAGS_4(_1, _2, _3, _4)                                    \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)

#define HJ_INDEX_TAGS_5(_1, _2, _3, _4, _5)                                \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)                                                      \
    HJ_INDEX_TAG (_5)

#define HJ_INDEX_TAGS_6(_1, _2, _3, _4, _5, _6)                            \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)                                                      \
    HJ_INDEX_TAG (_5)                                                      \
    HJ_INDEX_TAG (_6)

#define HJ_INDEX_TAGS_7(_1, _2, _3, _4, _5, _6, _7)                        \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)                                                      \
    HJ_INDEX_TAG (_5)                                                      \
    HJ_INDEX_TAG (_6) HJ_INDEX_TAG (_7)

#define HJ_INDEX_TAGS_8(_1, _2, _3, _4, _5, _6, _7, _8)                    \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)                                                      \
    HJ_INDEX_TAG (_5)                                                      \
    HJ_INDEX_TAG (_6)                                                      \
    HJ_INDEX_TAG (_7)                                                      \
    HJ_INDEX_TAG (_8)

#define HJ_INDEX_TAGS_9(_1, _2, _3, _4, _5, _6, _7, _8, _9)                \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)                                                      \
    HJ_INDEX_TAG (_5)                                                      \
    HJ_INDEX_TAG (_6)                                                      \
    HJ_INDEX_TAG (_7)                                                      \
    HJ_INDEX_TAG (_8)                                                      \
    HJ_INDEX_TAG (_9)

#define HJ_INDEX_TAGS_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10)          \
    HJ_INDEX_TAG (_1)                                                      \
    HJ_INDEX_TAG (_2)                                                      \
    HJ_INDEX_TAG (_3)                                                      \
    HJ_INDEX_TAG (_4)                                                      \
    HJ_INDEX_TAG (_5)                                                      \
    HJ_INDEX_TAG (_6)                                                      \
    HJ_INDEX_TAG (_7)                                                      \
    HJ_INDEX_TAG (_8)                                                      \
    HJ_INDEX_TAG (_9)                                                      \
    HJ_INDEX_TAG (_10)

#define HJ_GET_MAKE_TAGS_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,    \
                                   NAME, ...)                                  \
    NAME
#define HJ_INDEX_TAGS(...)                                                 \
    HJ_GET_MAKE_TAGS_MACRO (                                               \
      __VA_ARGS__, HJ_INDEX_TAGS_10, HJ_INDEX_TAGS_9,                  \
      HJ_INDEX_TAGS_8, HJ_INDEX_TAGS_7, HJ_INDEX_TAGS_6,           \
      HJ_INDEX_TAGS_5, HJ_INDEX_TAGS_4, HJ_INDEX_TAGS_3,           \
      HJ_INDEX_TAGS_2, HJ_INDEX_TAGS_1)                                \
    (__VA_ARGS__)

} // namespace hj

#endif // MULTI_INDEX_HPP