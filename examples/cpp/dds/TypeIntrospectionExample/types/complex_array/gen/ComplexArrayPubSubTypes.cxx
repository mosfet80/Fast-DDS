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
 * @file ComplexArrayPubSubTypes.cpp
 * This header file contains the implementation of the serialization functions.
 *
 * This file was generated by the tool fastddsgen.
 */


#include <fastdds/rtps/common/CdrSerialization.hpp>

#include "ComplexArrayPubSubTypes.h"
#include "ComplexArrayCdrAux.hpp"

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;
using DataRepresentationId_t = eprosima::fastdds::dds::DataRepresentationId_t;



InternalArrayPoints_TypeIntrospectionExamplePubSubType::InternalArrayPoints_TypeIntrospectionExamplePubSubType()
{
    setName("InternalArrayPoints_TypeIntrospectionExample");
    uint32_t type_size =
#if FASTCDR_VERSION_MAJOR == 1
        static_cast<uint32_t>(InternalArrayPoints_TypeIntrospectionExample::getMaxCdrSerializedSize());
#else
        InternalArrayPoints_TypeIntrospectionExample_max_cdr_typesize;
#endif
    type_size += static_cast<uint32_t>(eprosima::fastcdr::Cdr::alignment(type_size, 4)); /* possible submessage alignment */
    m_typeSize = type_size + 4; /*encapsulation*/
    m_isGetKeyDefined = false;
    uint32_t keyLength = InternalArrayPoints_TypeIntrospectionExample_max_key_cdr_typesize > 16 ? InternalArrayPoints_TypeIntrospectionExample_max_key_cdr_typesize : 16;
    m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
    memset(m_keyBuffer, 0, keyLength);
}

InternalArrayPoints_TypeIntrospectionExamplePubSubType::~InternalArrayPoints_TypeIntrospectionExamplePubSubType()
{
    if (m_keyBuffer != nullptr)
    {
        free(m_keyBuffer);
    }
}

bool InternalArrayPoints_TypeIntrospectionExamplePubSubType::serialize(
        void* data,
        SerializedPayload_t* payload,
        DataRepresentationId_t data_representation)
{
    InternalArrayPoints_TypeIntrospectionExample* p_type = static_cast<InternalArrayPoints_TypeIntrospectionExample*>(data);

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
            eprosima::fastcdr::CdrVersion::XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
#if FASTCDR_VERSION_MAJOR > 1
    ser.set_encoding_flag(
        data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR  :
        eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2);
#endif // FASTCDR_VERSION_MAJOR > 1

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();
        // Serialize the object.
        ser << *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    // Get the serialized length
#if FASTCDR_VERSION_MAJOR == 1
    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
#else
    payload->length = static_cast<uint32_t>(ser.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
    return true;
}

bool InternalArrayPoints_TypeIntrospectionExamplePubSubType::deserialize(
        SerializedPayload_t* payload,
        void* data)
{
    try
    {
        // Convert DATA to pointer of your type
        InternalArrayPoints_TypeIntrospectionExample* p_type = static_cast<InternalArrayPoints_TypeIntrospectionExample*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
#if FASTCDR_VERSION_MAJOR == 1
                , eprosima::fastcdr::Cdr::CdrType::DDS_CDR
#endif // FASTCDR_VERSION_MAJOR == 1
                );

        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        // Deserialize the object.
        deser >> *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    return true;
}

std::function<uint32_t()> InternalArrayPoints_TypeIntrospectionExamplePubSubType::getSerializedSizeProvider(
        void* data,
        DataRepresentationId_t data_representation)
{
    return [data, data_representation]() -> uint32_t
           {
#if FASTCDR_VERSION_MAJOR == 1
               static_cast<void>(data_representation);
               return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<InternalArrayPoints_TypeIntrospectionExample*>(data))) +
                      4u /*encapsulation*/;
#else
               try
               {
                   eprosima::fastcdr::CdrSizeCalculator calculator(
                       data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                       eprosima::fastcdr::CdrVersion::XCDRv1 :eprosima::fastcdr::CdrVersion::XCDRv2);
                   size_t current_alignment {0};
                   return static_cast<uint32_t>(calculator.calculate_serialized_size(
                               *static_cast<InternalArrayPoints_TypeIntrospectionExample*>(data), current_alignment)) +
                           4u /*encapsulation*/;
               }
               catch (eprosima::fastcdr::exception::Exception& /*exception*/)
               {
                   return 0;
               }
#endif // FASTCDR_VERSION_MAJOR == 1
           };
}

void* InternalArrayPoints_TypeIntrospectionExamplePubSubType::createData()
{
    return reinterpret_cast<void*>(new InternalArrayPoints_TypeIntrospectionExample());
}

void InternalArrayPoints_TypeIntrospectionExamplePubSubType::deleteData(
        void* data)
{
    delete(reinterpret_cast<InternalArrayPoints_TypeIntrospectionExample*>(data));
}

bool InternalArrayPoints_TypeIntrospectionExamplePubSubType::getKey(
        void* data,
        InstanceHandle_t* handle,
        bool force_md5)
{
    if (!m_isGetKeyDefined)
    {
        return false;
    }

    InternalArrayPoints_TypeIntrospectionExample* p_type = static_cast<InternalArrayPoints_TypeIntrospectionExample*>(data);

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
            InternalArrayPoints_TypeIntrospectionExample_max_key_cdr_typesize);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS, eprosima::fastcdr::CdrVersion::XCDRv1);
#if FASTCDR_VERSION_MAJOR == 1
    p_type->serializeKey(ser);
#else
    eprosima::fastcdr::serialize_key(ser, *p_type);
#endif // FASTCDR_VERSION_MAJOR == 1
    if (force_md5 || InternalArrayPoints_TypeIntrospectionExample_max_key_cdr_typesize > 16)
    {
        m_md5.init();
#if FASTCDR_VERSION_MAJOR == 1
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
#else
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.get_serialized_data_length()));
#endif // FASTCDR_VERSION_MAJOR == 1
        m_md5.finalize();
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_md5.digest[i];
        }
    }
    else
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}



InternalArrayMessage_TypeIntrospectionExamplePubSubType::InternalArrayMessage_TypeIntrospectionExamplePubSubType()
{
    setName("InternalArrayMessage_TypeIntrospectionExample");
    uint32_t type_size =
#if FASTCDR_VERSION_MAJOR == 1
        static_cast<uint32_t>(InternalArrayMessage_TypeIntrospectionExample::getMaxCdrSerializedSize());
#else
        InternalArrayMessage_TypeIntrospectionExample_max_cdr_typesize;
#endif
    type_size += static_cast<uint32_t>(eprosima::fastcdr::Cdr::alignment(type_size, 4)); /* possible submessage alignment */
    m_typeSize = type_size + 4; /*encapsulation*/
    m_isGetKeyDefined = false;
    uint32_t keyLength = InternalArrayMessage_TypeIntrospectionExample_max_key_cdr_typesize > 16 ? InternalArrayMessage_TypeIntrospectionExample_max_key_cdr_typesize : 16;
    m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
    memset(m_keyBuffer, 0, keyLength);
}

InternalArrayMessage_TypeIntrospectionExamplePubSubType::~InternalArrayMessage_TypeIntrospectionExamplePubSubType()
{
    if (m_keyBuffer != nullptr)
    {
        free(m_keyBuffer);
    }
}

bool InternalArrayMessage_TypeIntrospectionExamplePubSubType::serialize(
        void* data,
        SerializedPayload_t* payload,
        DataRepresentationId_t data_representation)
{
    InternalArrayMessage_TypeIntrospectionExample* p_type = static_cast<InternalArrayMessage_TypeIntrospectionExample*>(data);

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
            eprosima::fastcdr::CdrVersion::XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
#if FASTCDR_VERSION_MAJOR > 1
    ser.set_encoding_flag(
        data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR  :
        eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2);
#endif // FASTCDR_VERSION_MAJOR > 1

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();
        // Serialize the object.
        ser << *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    // Get the serialized length
#if FASTCDR_VERSION_MAJOR == 1
    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
#else
    payload->length = static_cast<uint32_t>(ser.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
    return true;
}

bool InternalArrayMessage_TypeIntrospectionExamplePubSubType::deserialize(
        SerializedPayload_t* payload,
        void* data)
{
    try
    {
        // Convert DATA to pointer of your type
        InternalArrayMessage_TypeIntrospectionExample* p_type = static_cast<InternalArrayMessage_TypeIntrospectionExample*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
#if FASTCDR_VERSION_MAJOR == 1
                , eprosima::fastcdr::Cdr::CdrType::DDS_CDR
#endif // FASTCDR_VERSION_MAJOR == 1
                );

        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        // Deserialize the object.
        deser >> *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    return true;
}

std::function<uint32_t()> InternalArrayMessage_TypeIntrospectionExamplePubSubType::getSerializedSizeProvider(
        void* data,
        DataRepresentationId_t data_representation)
{
    return [data, data_representation]() -> uint32_t
           {
#if FASTCDR_VERSION_MAJOR == 1
               static_cast<void>(data_representation);
               return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<InternalArrayMessage_TypeIntrospectionExample*>(data))) +
                      4u /*encapsulation*/;
#else
               try
               {
                   eprosima::fastcdr::CdrSizeCalculator calculator(
                       data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                       eprosima::fastcdr::CdrVersion::XCDRv1 :eprosima::fastcdr::CdrVersion::XCDRv2);
                   size_t current_alignment {0};
                   return static_cast<uint32_t>(calculator.calculate_serialized_size(
                               *static_cast<InternalArrayMessage_TypeIntrospectionExample*>(data), current_alignment)) +
                           4u /*encapsulation*/;
               }
               catch (eprosima::fastcdr::exception::Exception& /*exception*/)
               {
                   return 0;
               }
#endif // FASTCDR_VERSION_MAJOR == 1
           };
}

void* InternalArrayMessage_TypeIntrospectionExamplePubSubType::createData()
{
    return reinterpret_cast<void*>(new InternalArrayMessage_TypeIntrospectionExample());
}

void InternalArrayMessage_TypeIntrospectionExamplePubSubType::deleteData(
        void* data)
{
    delete(reinterpret_cast<InternalArrayMessage_TypeIntrospectionExample*>(data));
}

bool InternalArrayMessage_TypeIntrospectionExamplePubSubType::getKey(
        void* data,
        InstanceHandle_t* handle,
        bool force_md5)
{
    if (!m_isGetKeyDefined)
    {
        return false;
    }

    InternalArrayMessage_TypeIntrospectionExample* p_type = static_cast<InternalArrayMessage_TypeIntrospectionExample*>(data);

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
            InternalArrayMessage_TypeIntrospectionExample_max_key_cdr_typesize);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS, eprosima::fastcdr::CdrVersion::XCDRv1);
#if FASTCDR_VERSION_MAJOR == 1
    p_type->serializeKey(ser);
#else
    eprosima::fastcdr::serialize_key(ser, *p_type);
#endif // FASTCDR_VERSION_MAJOR == 1
    if (force_md5 || InternalArrayMessage_TypeIntrospectionExample_max_key_cdr_typesize > 16)
    {
        m_md5.init();
#if FASTCDR_VERSION_MAJOR == 1
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
#else
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.get_serialized_data_length()));
#endif // FASTCDR_VERSION_MAJOR == 1
        m_md5.finalize();
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_md5.digest[i];
        }
    }
    else
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}



ComplexArray_TypeIntrospectionExamplePubSubType::ComplexArray_TypeIntrospectionExamplePubSubType()
{
    setName("ComplexArray_TypeIntrospectionExample");
    uint32_t type_size =
#if FASTCDR_VERSION_MAJOR == 1
        static_cast<uint32_t>(ComplexArray_TypeIntrospectionExample::getMaxCdrSerializedSize());
#else
        ComplexArray_TypeIntrospectionExample_max_cdr_typesize;
#endif
    type_size += static_cast<uint32_t>(eprosima::fastcdr::Cdr::alignment(type_size, 4)); /* possible submessage alignment */
    m_typeSize = type_size + 4; /*encapsulation*/
    m_isGetKeyDefined = false;
    uint32_t keyLength = ComplexArray_TypeIntrospectionExample_max_key_cdr_typesize > 16 ? ComplexArray_TypeIntrospectionExample_max_key_cdr_typesize : 16;
    m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
    memset(m_keyBuffer, 0, keyLength);
}

ComplexArray_TypeIntrospectionExamplePubSubType::~ComplexArray_TypeIntrospectionExamplePubSubType()
{
    if (m_keyBuffer != nullptr)
    {
        free(m_keyBuffer);
    }
}

bool ComplexArray_TypeIntrospectionExamplePubSubType::serialize(
        void* data,
        SerializedPayload_t* payload,
        DataRepresentationId_t data_representation)
{
    ComplexArray_TypeIntrospectionExample* p_type = static_cast<ComplexArray_TypeIntrospectionExample*>(data);

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
            eprosima::fastcdr::CdrVersion::XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
#if FASTCDR_VERSION_MAJOR > 1
    ser.set_encoding_flag(
        data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR  :
        eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2);
#endif // FASTCDR_VERSION_MAJOR > 1

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();
        // Serialize the object.
        ser << *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    // Get the serialized length
#if FASTCDR_VERSION_MAJOR == 1
    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
#else
    payload->length = static_cast<uint32_t>(ser.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
    return true;
}

bool ComplexArray_TypeIntrospectionExamplePubSubType::deserialize(
        SerializedPayload_t* payload,
        void* data)
{
    try
    {
        // Convert DATA to pointer of your type
        ComplexArray_TypeIntrospectionExample* p_type = static_cast<ComplexArray_TypeIntrospectionExample*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
#if FASTCDR_VERSION_MAJOR == 1
                , eprosima::fastcdr::Cdr::CdrType::DDS_CDR
#endif // FASTCDR_VERSION_MAJOR == 1
                );

        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        // Deserialize the object.
        deser >> *p_type;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    return true;
}

std::function<uint32_t()> ComplexArray_TypeIntrospectionExamplePubSubType::getSerializedSizeProvider(
        void* data,
        DataRepresentationId_t data_representation)
{
    return [data, data_representation]() -> uint32_t
           {
#if FASTCDR_VERSION_MAJOR == 1
               static_cast<void>(data_representation);
               return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<ComplexArray_TypeIntrospectionExample*>(data))) +
                      4u /*encapsulation*/;
#else
               try
               {
                   eprosima::fastcdr::CdrSizeCalculator calculator(
                       data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                       eprosima::fastcdr::CdrVersion::XCDRv1 :eprosima::fastcdr::CdrVersion::XCDRv2);
                   size_t current_alignment {0};
                   return static_cast<uint32_t>(calculator.calculate_serialized_size(
                               *static_cast<ComplexArray_TypeIntrospectionExample*>(data), current_alignment)) +
                           4u /*encapsulation*/;
               }
               catch (eprosima::fastcdr::exception::Exception& /*exception*/)
               {
                   return 0;
               }
#endif // FASTCDR_VERSION_MAJOR == 1
           };
}

void* ComplexArray_TypeIntrospectionExamplePubSubType::createData()
{
    return reinterpret_cast<void*>(new ComplexArray_TypeIntrospectionExample());
}

void ComplexArray_TypeIntrospectionExamplePubSubType::deleteData(
        void* data)
{
    delete(reinterpret_cast<ComplexArray_TypeIntrospectionExample*>(data));
}

bool ComplexArray_TypeIntrospectionExamplePubSubType::getKey(
        void* data,
        InstanceHandle_t* handle,
        bool force_md5)
{
    if (!m_isGetKeyDefined)
    {
        return false;
    }

    ComplexArray_TypeIntrospectionExample* p_type = static_cast<ComplexArray_TypeIntrospectionExample*>(data);

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
            ComplexArray_TypeIntrospectionExample_max_key_cdr_typesize);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS, eprosima::fastcdr::CdrVersion::XCDRv1);
#if FASTCDR_VERSION_MAJOR == 1
    p_type->serializeKey(ser);
#else
    eprosima::fastcdr::serialize_key(ser, *p_type);
#endif // FASTCDR_VERSION_MAJOR == 1
    if (force_md5 || ComplexArray_TypeIntrospectionExample_max_key_cdr_typesize > 16)
    {
        m_md5.init();
#if FASTCDR_VERSION_MAJOR == 1
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
#else
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.get_serialized_data_length()));
#endif // FASTCDR_VERSION_MAJOR == 1
        m_md5.finalize();
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_md5.digest[i];
        }
    }
    else
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}
