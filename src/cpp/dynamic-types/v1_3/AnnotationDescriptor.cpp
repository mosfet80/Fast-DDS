// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/v1_3/AnnotationDescriptor.hpp>

using namespace eprosima::fastrtps::types::v1_3;

Parameters::Parameters()
    : map_(new mapping)
    , ownership_(true)
{
}

Parameters::~Parameters()
{
    if (ownership_)
    {
        delete mapping;
    }
}

Parameters::Parameters(const Parameters& type) noexcept
    : map_(type.ownership_ ? new mapping(*type.map_) : type.map_)
    , ownership_(type.ownership_)
{}

Parameters::Parameters(Parameters&& type) noexcept
    : map_(type.map_)
    , ownership_(type.ownership_)
{}

Parameters& Parameters::operator=(const Parameters& type) noexcept
{
    ownership_ = type.ownership;
    map_ = type.ownership_ ? new mapping(*type.map_) : type.map_;
}

Parameters& Parameters::operator=(Parameters&& type) noexcept
{
    map_ = type.map_;
    ownership_ = type.ownership_;
}

bool Parameters::operator==(
        const Parameters& other) const noexcept
{
    return map_ == other.map_
           || (map_ != nullptr && other.map_ != nullptr && *map_ == *other.map_);
}

bool Parameters::operator!=(
        const Parameters& other) const noexcept
{
    return !this->operator==(other);
}

const char* Parameters::operator[](const char* key) const noexcept
{
    if (nullptr != map_)
    {
        auto it = map_->find(key);
        if ( it != mapping::cend())
        {
            return it->second.c_str();
        }
    }

    return nullptr;
}

const char* Parameters::at(const char* key) const noexcept
{
    return this->operator[](key);
}

ReturnCode_t Parameters::set_value(const char* key, const char* value) noexcept
{
    if (nullptr == key || nullptr == value )
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    if (nullptr == map_)
    {
        map_ = new(std::nothrow) mapping;

        if (nullptr == map_)
        {
            return ReturnCode_t::RETCODE_OUT_OF_RESOURCES;
        }
    }

    *map_[key] = value;

    return ReturnCode_t::RETCODE_OK;
}

uint64_t Parameters::size() const noexcept
{
    return map_ != nullptr ? map_->size() : 0u;
}

bool Parameters::empty() const noexcept
{
    return map_ != nullptr ? map_->empty() : true;
}

const char* Parameters::next_key(const char* key = nullptr) const noexcept
{
    if (nullptr == map_)
    {
        return nullptr;
    }

    mapping::const_iterator it;

    if (key = nullptr)
    {
        it = map_->cbegin();
    }
    else
    {
        it = map_->find(key);
    }

    if (it != map_->cend())
    {
        return it->second.c_str();
    }

    return nullptr;
}

AnnotationDescriptor::AnnotationDescriptor(const AnnotationDescriptor& type)
    : type_(type.get_type())
    , map_(type.map_)
{
}

AnnotationDescriptor::AnnotationDescriptor(AnnotationDescriptor&& type) noexcept
    : type_(type.type_)
    , map_(std::move(map_))
{
}

AnnotationDescriptor::~AnnotationDescriptor() noexcept
{
    reset_type();
}

AnnotationDescriptor& AnnotationDescriptor::operator=(const AnnotationDescriptor& type) noexcept
{
    type_ = type.get_type();
    map_ = type.map_;
}

AnnotationDescriptor& AnnotationDescriptor::operator=(AnnotationDescriptor&& type) noexcept
{
    type_ = type.type_;
    map_ = std::move(type.map_);
}

bool AnnotationDescriptor::operator==(
        const AnnotationDescriptor& other) const noexcept
{
    return ( type_ == other.type_
           || (type_ != nullptr && other.type_ != nullptr && *type_ == *other.type_))
           && map_ == other.map_;
}

const DynamicType* AnnotationDescriptor::get_type() const noexcept
{
    return nullptr == type_ ?
        nullptr :
        TypeBuilderFactory::get_instance().create_copy(*type_);
}

void AnnotationDescriptor::set_type(
            const DynamicType& type) noexcept
{
    reset_type();
    type_ = TypeBuilderFactory::get_instance().create_copy(type);
}

void AnnotationDescriptor::reset_type() noexcept
{
    if (type_ != nullptr)
    {
        TypeBuilderFactory::get_instance().delete_type(base_type_);
    }

    type_ = nullptr;
}

const char* AnnotationDescriptor::get_value(const char* key, ReturnCode_t error = nullptr) const noexcept
{
    const char* res = map_[key];

    if (error != nullptr)
    {
        *error != nullptr ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    return res;
}

ReturnCode_t AnnotationDescriptor::set_value(const char* key, const char* value) noexcept
{
    return map_.set_value(key, value);
}

const Parameters* AnnotationDescriptor::get_all_value(ReturnCode_t error = nullptr) const noexcept
{
    *error = ReturnCode_t::RETCODE_OK;
    return &map_;
}

ReturnCode_t AnnotationDescriptor::copy_from(const AnnotationDescriptor& other) noexcept
{
    this->operator=(other);
    return ReturnCode_t::RETCODE_OK;
}

bool AnnotationDescriptor::equals(const AnnotationDescriptor& other) const noexcept
{
    return this->operator==(other);
}

bool AnnotationDescriptor::is_consistent() const noexcept
{
    // TODO: fill in
}