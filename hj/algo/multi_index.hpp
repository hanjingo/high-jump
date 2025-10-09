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
    using member_type                = MemberType;
    static constexpr auto member_ptr = MemberPtr;
    using tag_type                   = Tag;
    static constexpr bool is_unique  = true;
    static_assert(std::is_member_pointer_v<decltype(MemberPtr)>,
                  "MemberPtr must be a member pointer");
};

template <typename MemberType, auto MemberPtr, typename Tag>
struct nonunique_index_impl
{
    using member_type                = MemberType;
    static constexpr auto member_ptr = MemberPtr;
    using tag_type                   = Tag;
    static constexpr bool is_unique  = false;
    static_assert(std::is_member_pointer_v<decltype(MemberPtr)>,
                  "MemberPtr must be a member pointer");
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
        typename detail::make_index<IndexConfigs, Class>::type...>>;
} // namespace hj

#define HJ_UNIQUE_INDEX(member_type, member_ptr, tag)                          \
    hj::unique_index<member_type, member_ptr, tag>,
#define HJ_NON_UNIQUE_INDEX(member_type, member_ptr, tag)                      \
    hj::nonunique_index<member_type, member_ptr, tag>

#define HJ_INDEX_TAG(tag)                                                      \
    struct tag                                                                 \
    {                                                                          \
    };


#endif // MULTI_INDEX_HPP