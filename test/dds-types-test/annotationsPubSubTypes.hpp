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
 * @file annotationsPubSubTypes.hpp
 * This header file contains the declaration of the serialization functions.
 *
 * This file was generated by the tool fastddsgen.
 */


#ifndef FAST_DDS_GENERATED__ANNOTATIONS_PUBSUBTYPES_HPP
#define FAST_DDS_GENERATED__ANNOTATIONS_PUBSUBTYPES_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/utils/md5.hpp>

#include "annotations.hpp"

#include "helpers/basic_inner_typesPubSubTypes.hpp"

#if !defined(FASTDDS_GEN_API_VER) || (FASTDDS_GEN_API_VER != 3)
#error \
    Generated annotations is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // FASTDDS_GEN_API_VER


/*!
 * @brief This class represents the TopicDataType of the type AnnotatedStruct defined by the user in the IDL file.
 * @ingroup annotations
 */
class AnnotatedStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef AnnotatedStruct type;

    eProsima_user_DllExport AnnotatedStructPubSubType();

    eProsima_user_DllExport ~AnnotatedStructPubSubType() override;

    eProsima_user_DllExport bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;

    eProsima_user_DllExport uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport bool compute_key(
            const void* const data,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* create_data() override;

    eProsima_user_DllExport void delete_data(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

    eProsima_user_DllExport inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

private:

    eprosima::fastdds::MD5 md5_;
    unsigned char* key_buffer_;

};

/*!
 * @brief This class represents the TopicDataType of the type EmptyAnnotatedStruct defined by the user in the IDL file.
 * @ingroup annotations
 */
class EmptyAnnotatedStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef EmptyAnnotatedStruct type;

    eProsima_user_DllExport EmptyAnnotatedStructPubSubType();

    eProsima_user_DllExport ~EmptyAnnotatedStructPubSubType() override;

    eProsima_user_DllExport bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;

    eProsima_user_DllExport uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport bool compute_key(
            const void* const data,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* create_data() override;

    eProsima_user_DllExport void delete_data(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

    eProsima_user_DllExport inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

private:

    eprosima::fastdds::MD5 md5_;
    unsigned char* key_buffer_;

};


/*!
 * @brief This class represents the TopicDataType of the type BasicAnnotationsStruct defined by the user in the IDL file.
 * @ingroup annotations
 */
class BasicAnnotationsStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef BasicAnnotationsStruct type;

    eProsima_user_DllExport BasicAnnotationsStructPubSubType();

    eProsima_user_DllExport ~BasicAnnotationsStructPubSubType() override;

    eProsima_user_DllExport bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;

    eProsima_user_DllExport uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport bool compute_key(
            const void* const data,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* create_data() override;

    eProsima_user_DllExport void delete_data(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

    eProsima_user_DllExport inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

private:

    eprosima::fastdds::MD5 md5_;
    unsigned char* key_buffer_;

};

#endif // FAST_DDS_GENERATED__ANNOTATIONS_PUBSUBTYPES_HPP
