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
 * @file key_struct.hpp
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef FAST_DDS_GENERATED__KEY_STRUCT_HPP
#define FAST_DDS_GENERATED__KEY_STRUCT_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <fastcdr/cdr/fixed_size_string.hpp>

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
#if defined(KEY_STRUCT_SOURCE)
#define KEY_STRUCT_DllAPI __declspec( dllexport )
#else
#define KEY_STRUCT_DllAPI __declspec( dllimport )
#endif // KEY_STRUCT_SOURCE
#else
#define KEY_STRUCT_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define KEY_STRUCT_DllAPI
#endif // _WIN32

/*!
 * @brief This class represents the structure ImportantStruct defined by the user in the IDL file.
 * @ingroup key_struct
 */
class ImportantStruct
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport ImportantStruct()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~ImportantStruct()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object ImportantStruct that will be copied.
     */
    eProsima_user_DllExport ImportantStruct(
            const ImportantStruct& x)
    {
                    m_my_first_value = x.m_my_first_value;

                    m_my_second_value = x.m_my_second_value;

                    m_my_third_value = x.m_my_third_value;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object ImportantStruct that will be copied.
     */
    eProsima_user_DllExport ImportantStruct(
            ImportantStruct&& x) noexcept
    {
        m_my_first_value = x.m_my_first_value;
        m_my_second_value = x.m_my_second_value;
        m_my_third_value = x.m_my_third_value;
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object ImportantStruct that will be copied.
     */
    eProsima_user_DllExport ImportantStruct& operator =(
            const ImportantStruct& x)
    {

                    m_my_first_value = x.m_my_first_value;

                    m_my_second_value = x.m_my_second_value;

                    m_my_third_value = x.m_my_third_value;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object ImportantStruct that will be copied.
     */
    eProsima_user_DllExport ImportantStruct& operator =(
            ImportantStruct&& x) noexcept
    {

        m_my_first_value = x.m_my_first_value;
        m_my_second_value = x.m_my_second_value;
        m_my_third_value = x.m_my_third_value;
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x ImportantStruct object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const ImportantStruct& x) const
    {
        return (m_my_first_value == x.m_my_first_value &&
           m_my_second_value == x.m_my_second_value &&
           m_my_third_value == x.m_my_third_value);
    }

    /*!
     * @brief Comparison operator.
     * @param x ImportantStruct object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const ImportantStruct& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function sets a value in member my_first_value
     * @param _my_first_value New value for member my_first_value
     */
    eProsima_user_DllExport void my_first_value(
            int32_t _my_first_value)
    {
        m_my_first_value = _my_first_value;
    }

    /*!
     * @brief This function returns the value of member my_first_value
     * @return Value of member my_first_value
     */
    eProsima_user_DllExport int32_t my_first_value() const
    {
        return m_my_first_value;
    }

    /*!
     * @brief This function returns a reference to member my_first_value
     * @return Reference to member my_first_value
     */
    eProsima_user_DllExport int32_t& my_first_value()
    {
        return m_my_first_value;
    }


    /*!
     * @brief This function sets a value in member my_second_value
     * @param _my_second_value New value for member my_second_value
     */
    eProsima_user_DllExport void my_second_value(
            int32_t _my_second_value)
    {
        m_my_second_value = _my_second_value;
    }

    /*!
     * @brief This function returns the value of member my_second_value
     * @return Value of member my_second_value
     */
    eProsima_user_DllExport int32_t my_second_value() const
    {
        return m_my_second_value;
    }

    /*!
     * @brief This function returns a reference to member my_second_value
     * @return Reference to member my_second_value
     */
    eProsima_user_DllExport int32_t& my_second_value()
    {
        return m_my_second_value;
    }


    /*!
     * @brief This function sets a value in member my_third_value
     * @param _my_third_value New value for member my_third_value
     */
    eProsima_user_DllExport void my_third_value(
            int32_t _my_third_value)
    {
        m_my_third_value = _my_third_value;
    }

    /*!
     * @brief This function returns the value of member my_third_value
     * @return Value of member my_third_value
     */
    eProsima_user_DllExport int32_t my_third_value() const
    {
        return m_my_third_value;
    }

    /*!
     * @brief This function returns a reference to member my_third_value
     * @return Reference to member my_third_value
     */
    eProsima_user_DllExport int32_t& my_third_value()
    {
        return m_my_third_value;
    }



private:

    int32_t m_my_first_value{0};
    int32_t m_my_second_value{0};
    int32_t m_my_third_value{0};

};
/*!
 * @brief This class represents the structure KeyStruct defined by the user in the IDL file.
 * @ingroup key_struct
 */
class KeyStruct
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport KeyStruct()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~KeyStruct()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object KeyStruct that will be copied.
     */
    eProsima_user_DllExport KeyStruct(
            const KeyStruct& x)
    {
                    m_my_short = x.m_my_short;

                    m_my_long = x.m_my_long;

                    m_my_string = x.m_my_string;

                    m_my_important_struct = x.m_my_important_struct;

    }

    /*!
     * @brief Move constructor.
     * @param x Reference to the object KeyStruct that will be copied.
     */
    eProsima_user_DllExport KeyStruct(
            KeyStruct&& x) noexcept
    {
        m_my_short = x.m_my_short;
        m_my_long = x.m_my_long;
        m_my_string = std::move(x.m_my_string);
        m_my_important_struct = std::move(x.m_my_important_struct);
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object KeyStruct that will be copied.
     */
    eProsima_user_DllExport KeyStruct& operator =(
            const KeyStruct& x)
    {

                    m_my_short = x.m_my_short;

                    m_my_long = x.m_my_long;

                    m_my_string = x.m_my_string;

                    m_my_important_struct = x.m_my_important_struct;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object KeyStruct that will be copied.
     */
    eProsima_user_DllExport KeyStruct& operator =(
            KeyStruct&& x) noexcept
    {

        m_my_short = x.m_my_short;
        m_my_long = x.m_my_long;
        m_my_string = std::move(x.m_my_string);
        m_my_important_struct = std::move(x.m_my_important_struct);
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x KeyStruct object to compare.
     */
    eProsima_user_DllExport bool operator ==(
            const KeyStruct& x) const
    {
        return (m_my_short == x.m_my_short &&
           m_my_long == x.m_my_long &&
           m_my_string == x.m_my_string &&
           m_my_important_struct == x.m_my_important_struct);
    }

    /*!
     * @brief Comparison operator.
     * @param x KeyStruct object to compare.
     */
    eProsima_user_DllExport bool operator !=(
            const KeyStruct& x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function sets a value in member my_short
     * @param _my_short New value for member my_short
     */
    eProsima_user_DllExport void my_short(
            int16_t _my_short)
    {
        m_my_short = _my_short;
    }

    /*!
     * @brief This function returns the value of member my_short
     * @return Value of member my_short
     */
    eProsima_user_DllExport int16_t my_short() const
    {
        return m_my_short;
    }

    /*!
     * @brief This function returns a reference to member my_short
     * @return Reference to member my_short
     */
    eProsima_user_DllExport int16_t& my_short()
    {
        return m_my_short;
    }


    /*!
     * @brief This function sets a value in member my_long
     * @param _my_long New value for member my_long
     */
    eProsima_user_DllExport void my_long(
            int32_t _my_long)
    {
        m_my_long = _my_long;
    }

    /*!
     * @brief This function returns the value of member my_long
     * @return Value of member my_long
     */
    eProsima_user_DllExport int32_t my_long() const
    {
        return m_my_long;
    }

    /*!
     * @brief This function returns a reference to member my_long
     * @return Reference to member my_long
     */
    eProsima_user_DllExport int32_t& my_long()
    {
        return m_my_long;
    }


    /*!
     * @brief This function copies the value in member my_string
     * @param _my_string New value to be copied in member my_string
     */
    eProsima_user_DllExport void my_string(
            const std::string& _my_string)
    {
        m_my_string = _my_string;
    }

    /*!
     * @brief This function moves the value in member my_string
     * @param _my_string New value to be moved in member my_string
     */
    eProsima_user_DllExport void my_string(
            std::string&& _my_string)
    {
        m_my_string = std::move(_my_string);
    }

    /*!
     * @brief This function returns a constant reference to member my_string
     * @return Constant reference to member my_string
     */
    eProsima_user_DllExport const std::string& my_string() const
    {
        return m_my_string;
    }

    /*!
     * @brief This function returns a reference to member my_string
     * @return Reference to member my_string
     */
    eProsima_user_DllExport std::string& my_string()
    {
        return m_my_string;
    }


    /*!
     * @brief This function copies the value in member my_important_struct
     * @param _my_important_struct New value to be copied in member my_important_struct
     */
    eProsima_user_DllExport void my_important_struct(
            const ImportantStruct& _my_important_struct)
    {
        m_my_important_struct = _my_important_struct;
    }

    /*!
     * @brief This function moves the value in member my_important_struct
     * @param _my_important_struct New value to be moved in member my_important_struct
     */
    eProsima_user_DllExport void my_important_struct(
            ImportantStruct&& _my_important_struct)
    {
        m_my_important_struct = std::move(_my_important_struct);
    }

    /*!
     * @brief This function returns a constant reference to member my_important_struct
     * @return Constant reference to member my_important_struct
     */
    eProsima_user_DllExport const ImportantStruct& my_important_struct() const
    {
        return m_my_important_struct;
    }

    /*!
     * @brief This function returns a reference to member my_important_struct
     * @return Reference to member my_important_struct
     */
    eProsima_user_DllExport ImportantStruct& my_important_struct()
    {
        return m_my_important_struct;
    }



private:

    int16_t m_my_short{0};
    int32_t m_my_long{0};
    std::string m_my_string;
    ImportantStruct m_my_important_struct;

};

#endif // _FAST_DDS_GENERATED_KEY_STRUCT_HPP_

