// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file basic_inner_types.hpp
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen (version: 4.1.0).
 */

#ifndef FAST_DDS_GENERATED__BASIC_INNER_TYPES_HPP
#define FAST_DDS_GENERATED__BASIC_INNER_TYPES_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <fastcdr/cdr/fixed_size_string.hpp>
#include <fastcdr/exceptions/BadParamException.h>


#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(BASIC_INNER_TYPES_SOURCE)
#define BASIC_INNER_TYPES_DllAPI __declspec( dllexport )
#else
#define BASIC_INNER_TYPES_DllAPI __declspec( dllimport )
#endif // BASIC_INNER_TYPES_SOURCE
#else
#define BASIC_INNER_TYPES_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define BASIC_INNER_TYPES_DllAPI
#endif // _WIN32

/*!
 * @brief This class represents the enumeration InnerEnumHelper defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
enum class InnerEnumHelper : int32_t
{
    ENUM_VALUE_1,
    ENUM_VALUE_2,
    ENUM_VALUE_3
};
/*!
 * @brief This enumeration represents the InnerBitMaskHelper bitflags defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
enum InnerBitMaskHelperBits : uint32_t
{
    flag0 = 0x01ull << 0,
    flag1 = 0x01ull << 1,
    flag4 = 0x01ull << 4,
    flag6 = 0x01ull << 6
};
typedef uint32_t InnerBitMaskHelper;
/*!
 * @brief This enumeration represents the InnerBoundedBitMaskHelper bitflags defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
enum InnerBoundedBitMaskHelperBits : uint8_t
{
    bflag0 = 0x01ull << 0,
    bflag1 = 0x01ull << 1,
    bflag4 = 0x01ull << 4,
    bflag6 = 0x01ull << 6
};
typedef uint8_t InnerBoundedBitMaskHelper;
typedef int32_t InnerAliasHelper;

/*!
 * @brief This class represents the structure InnerStructureHelper defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
class InnerStructureHelper
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport InnerStructureHelper()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~InnerStructureHelper()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object InnerStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerStructureHelper(
            const InnerStructureHelper& x)
    {
                    m_field1 = x.m_field1;

                    m_field2 = x.m_field2;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object InnerStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerStructureHelper(
            InnerStructureHelper&& x) noexcept
    {
        m_field1 = x.m_field1;
        m_field2 = x.m_field2;
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object InnerStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerStructureHelper& operator =(
            const InnerStructureHelper& x)
    {

                    m_field1 = x.m_field1;

                    m_field2 = x.m_field2;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object InnerStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerStructureHelper& operator =(
            InnerStructureHelper&& x) noexcept
    {

        m_field1 = x.m_field1;
        m_field2 = x.m_field2;
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerStructureHelper object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const InnerStructureHelper& x) const
    {
        return (m_field1 == x.m_field1 &&
           m_field2 == x.m_field2);
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerStructureHelper object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const InnerStructureHelper& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function sets a value in member field1
     * @param _field1 New value for member field1
     */
    eProsima_user_DllExport void field1(
            int32_t _field1)
    {
        m_field1 = _field1;
    }

    /*!
     * @brief This function returns the value of member field1
     * @return Value of member field1
     */
    eProsima_user_DllExport int32_t field1() const
    {
        return m_field1;
    }

    /*!
     * @brief This function returns a reference to member field1
     * @return Reference to member field1
     */
    eProsima_user_DllExport int32_t& field1()
    {
        return m_field1;
    }


    /*!
     * @brief This function sets a value in member field2
     * @param _field2 New value for member field2
     */
    eProsima_user_DllExport void field2(
            float _field2)
    {
        m_field2 = _field2;
    }

    /*!
     * @brief This function returns the value of member field2
     * @return Value of member field2
     */
    eProsima_user_DllExport float field2() const
    {
        return m_field2;
    }

    /*!
     * @brief This function returns a reference to member field2
     * @return Reference to member field2
     */
    eProsima_user_DllExport float& field2()
    {
        return m_field2;
    }



private:

    int32_t m_field1{0};
    float m_field2{0.0};

};
/*!
 * @brief This class represents the structure InnerEmptyStructureHelper defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
class InnerEmptyStructureHelper
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport InnerEmptyStructureHelper()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~InnerEmptyStructureHelper()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object InnerEmptyStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerEmptyStructureHelper(
            const InnerEmptyStructureHelper& x)
    {
        static_cast<void>(x);
    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object InnerEmptyStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerEmptyStructureHelper(
            InnerEmptyStructureHelper&& x) noexcept
    {
        static_cast<void>(x);
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object InnerEmptyStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerEmptyStructureHelper& operator =(
            const InnerEmptyStructureHelper& x)
    {

        static_cast<void>(x);

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object InnerEmptyStructureHelper that will be copied.
     */
    eProsima_user_DllExport InnerEmptyStructureHelper& operator =(
            InnerEmptyStructureHelper&& x) noexcept
    {

        static_cast<void>(x);

        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerEmptyStructureHelper object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const InnerEmptyStructureHelper& x) const
    {
        static_cast<void>(x);
        return true;
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerEmptyStructureHelper object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const InnerEmptyStructureHelper& x) const
    {
        return !(*this == x);
    }



private:


};
/*!
 * @brief This class represents the union InnerUnionHelper defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
class InnerUnionHelper
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport InnerUnionHelper()
    {
        selected_member_ = 0x00000003;
        member_destructor_ = nullptr;
        m_shortValue = {0};

    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~InnerUnionHelper()
    {
        if (member_destructor_)
        {
            member_destructor_();
        }
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object InnerUnionHelper that will be copied.
     */
    eProsima_user_DllExport InnerUnionHelper(
            const InnerUnionHelper& x)
    {
        m__d = x.m__d;

        switch (x.selected_member_)
        {
                        case 0x00000001:
                            longValue_() = x.m_longValue;
                            break;

                        case 0x00000002:
                            floatValue_() = x.m_floatValue;
                            break;

                        case 0x00000003:
                            shortValue_() = x.m_shortValue;
                            break;

        }
    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object InnerUnionHelper that will be copied.
     */
    eProsima_user_DllExport InnerUnionHelper(
            InnerUnionHelper&& x) noexcept
    {
        m__d = x.m__d;

        switch (x.selected_member_)
        {
                        case 0x00000001:
                            longValue_() = std::move(x.m_longValue);
                            break;

                        case 0x00000002:
                            floatValue_() = std::move(x.m_floatValue);
                            break;

                        case 0x00000003:
                            shortValue_() = std::move(x.m_shortValue);
                            break;

        }
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object InnerUnionHelper that will be copied.
     */
    eProsima_user_DllExport InnerUnionHelper& operator =(
            const InnerUnionHelper& x)
    {
        m__d = x.m__d;

        switch (x.selected_member_)
        {
                        case 0x00000001:
                            longValue_() = x.m_longValue;
                            break;

                        case 0x00000002:
                            floatValue_() = x.m_floatValue;
                            break;

                        case 0x00000003:
                            shortValue_() = x.m_shortValue;
                            break;

        }

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object InnerUnionHelper that will be copied.
     */
    eProsima_user_DllExport InnerUnionHelper& operator =(
            InnerUnionHelper&& x) noexcept
    {
        m__d = x.m__d;

        switch (x.selected_member_)
        {
                        case 0x00000001:
                            longValue_() = std::move(x.m_longValue);
                            break;

                        case 0x00000002:
                            floatValue_() = std::move(x.m_floatValue);
                            break;

                        case 0x00000003:
                            shortValue_() = std::move(x.m_shortValue);
                            break;

        }

        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerUnionHelper object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const InnerUnionHelper& x) const
    {
        bool ret_value {false};

        if (x.selected_member_ == selected_member_)
        {
            if (0x0FFFFFFFu != selected_member_)
            {
                if (x.m__d == m__d)
                {
                    switch (selected_member_)
                    {
                                                    case 0x00000001:
                                                        ret_value = (x.m_longValue == m_longValue);
                                                        break;

                                                    case 0x00000002:
                                                        ret_value = (x.m_floatValue == m_floatValue);
                                                        break;

                                                    case 0x00000003:
                                                        ret_value = (x.m_shortValue == m_shortValue);
                                                        break;

                    }
                }
            }
            else
            {
                ret_value = true;
            }
        }

        return ret_value;
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerUnionHelper object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const InnerUnionHelper& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function sets the discriminator value.
     * @param __d New value for the discriminator.
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
     */
    eProsima_user_DllExport void _d(
            int32_t __d)
    {
        bool valid_discriminator = false;

        switch (__d)
        {
                        case 0:
                            if (0x00000001 == selected_member_)
                            {
                                valid_discriminator = true;
                            }
                            break;

                        case 1:
                            if (0x00000002 == selected_member_)
                            {
                                valid_discriminator = true;
                            }
                            break;

                        default:
                            if (0x00000003 == selected_member_)
                            {
                                valid_discriminator = true;
                            }
                            break;

        }

        if (!valid_discriminator)
        {
            throw eprosima::fastcdr::exception::BadParamException("Discriminator doesn't correspond with the selected union member");
        }

        m__d = __d;
    }

    /*!
     * @brief This function returns the value of the discriminator.
     * @return Value of the discriminator
     */
    eProsima_user_DllExport int32_t _d() const
    {
        return m__d;
    }

    /*!
     * @brief This function sets a value in member longValue
     * @param _longValue New value for member longValue
     */
    eProsima_user_DllExport void longValue(
            int32_t _longValue)
    {
        longValue_() = _longValue;
        m__d = 0;
    }

    /*!
     * @brief This function returns the value of member longValue
     * @return Value of member longValue
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    eProsima_user_DllExport int32_t longValue() const
    {
        if (0x00000001 != selected_member_)
        {
            throw eprosima::fastcdr::exception::BadParamException("This member has not been selected");
        }

        return m_longValue;
    }

    /*!
     * @brief This function returns a reference to member longValue
     * @return Reference to member longValue
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    eProsima_user_DllExport int32_t& longValue()
    {
        if (0x00000001 != selected_member_)
        {
            throw eprosima::fastcdr::exception::BadParamException("This member has not been selected");
        }

        return m_longValue;
    }


    /*!
     * @brief This function sets a value in member floatValue
     * @param _floatValue New value for member floatValue
     */
    eProsima_user_DllExport void floatValue(
            float _floatValue)
    {
        floatValue_() = _floatValue;
        m__d = 1;
    }

    /*!
     * @brief This function returns the value of member floatValue
     * @return Value of member floatValue
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    eProsima_user_DllExport float floatValue() const
    {
        if (0x00000002 != selected_member_)
        {
            throw eprosima::fastcdr::exception::BadParamException("This member has not been selected");
        }

        return m_floatValue;
    }

    /*!
     * @brief This function returns a reference to member floatValue
     * @return Reference to member floatValue
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    eProsima_user_DllExport float& floatValue()
    {
        if (0x00000002 != selected_member_)
        {
            throw eprosima::fastcdr::exception::BadParamException("This member has not been selected");
        }

        return m_floatValue;
    }


    /*!
     * @brief This function sets a value in member shortValue
     * @param _shortValue New value for member shortValue
     */
    eProsima_user_DllExport void shortValue(
            int16_t _shortValue)
    {
        shortValue_() = _shortValue;
        m__d = 2147483647;
    }

    /*!
     * @brief This function returns the value of member shortValue
     * @return Value of member shortValue
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    eProsima_user_DllExport int16_t shortValue() const
    {
        if (0x00000003 != selected_member_)
        {
            throw eprosima::fastcdr::exception::BadParamException("This member has not been selected");
        }

        return m_shortValue;
    }

    /*!
     * @brief This function returns a reference to member shortValue
     * @return Reference to member shortValue
     * @exception eprosima::fastcdr::exception::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    eProsima_user_DllExport int16_t& shortValue()
    {
        if (0x00000003 != selected_member_)
        {
            throw eprosima::fastcdr::exception::BadParamException("This member has not been selected");
        }

        return m_shortValue;
    }



private:

            int32_t& longValue_()
            {
                if (0x00000001 != selected_member_)
                {
                    if (member_destructor_)
                    {
                        member_destructor_();
                    }

                    selected_member_ = 0x00000001;
                    member_destructor_ = nullptr;
                    m_longValue = {0};

                }

                return m_longValue;
            }

            float& floatValue_()
            {
                if (0x00000002 != selected_member_)
                {
                    if (member_destructor_)
                    {
                        member_destructor_();
                    }

                    selected_member_ = 0x00000002;
                    member_destructor_ = nullptr;
                    m_floatValue = {0.0};

                }

                return m_floatValue;
            }

            int16_t& shortValue_()
            {
                if (0x00000003 != selected_member_)
                {
                    if (member_destructor_)
                    {
                        member_destructor_();
                    }

                    selected_member_ = 0x00000003;
                    member_destructor_ = nullptr;
                    m_shortValue = {0};

                }

                return m_shortValue;
            }


    int32_t m__d {2147483647};

    union
    {
        int32_t m_longValue;
        float m_floatValue;
        int16_t m_shortValue;
    };

    uint32_t selected_member_ {0x0FFFFFFFu};

    std::function<void()> member_destructor_;
};
/*!
 * @brief This structure represents the bitset InnerBitsetHelper defined by the user in the IDL file.
 * @ingroup basic_inner_types
 */
struct InnerBitsetHelper
{
        uint8_t a : 3;

        bool b : 1;

        uint8_t  : 4;

        uint16_t c : 10;

        uint8_t  : 3;

        int16_t d : 12;


    /*!
     * @brief Comparison operator.
     * @param x InnerBitsetHelper object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const InnerBitsetHelper& x) const
    {
        return (a == x.a &&
           b == x.b &&
           c == x.c &&
           d == x.d);
    }

    /*!
     * @brief Comparison operator.
     * @param x InnerBitsetHelper object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const InnerBitsetHelper& x) const
    {
        return !(*this == x);
    }
};
const int16_t inner_const_helper = 1;
typedef eprosima::fastcdr::fixed_string<10> Inner_alias_bounded_string_helper;

typedef std::wstring Inner_alias_bounded_wstring_helper;

typedef std::array<int16_t, 2> Inner_alias_array_helper;

typedef std::vector<int16_t> Inner_alias_sequence_helper;

typedef std::map<int32_t, int32_t> Inner_alias_map_helper;

typedef InnerStructureHelper inner_structure_helper_alias;

typedef InnerBitsetHelper inner_bitset_helper_alias;

const std::string const_string = "AAA";
const std::wstring const_wstring = L"BBB";

#endif // _FAST_DDS_GENERATED_BASIC_INNER_TYPES_HPP_


