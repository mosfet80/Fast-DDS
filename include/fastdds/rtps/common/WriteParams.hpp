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

/**
 * @file WriteParams.hpp
 */
#ifndef FASTDDS_RTPS_COMMON__WRITEPARAMS_HPP
#define FASTDDS_RTPS_COMMON__WRITEPARAMS_HPP

#include <memory>

#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/*!
 * @brief This class contains additional information of a CacheChange.
 *
 * @ingroup COMMON_MODULE
 */
class FASTDDS_EXPORTED_API WriteParams
{
public:

    /**
     * @brief Base class storing custom information in the
     * WriteParams structure for later filtering.
     *
     * This struct serves as a base class that allows derived
     * classes to be deleted safely through a pointer to this base type.
     * It is intended to be user-extensible.
     */
    struct FASTDDS_EXPORTED_API UserWriteData
    {
        UserWriteData() = default;

        virtual ~UserWriteData() = default;
    };

    /**
     * Set the value of the sample_identity member.
     *
     * @param sample_id  New value for the sample_identity member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& sample_identity(
            const SampleIdentity& sample_id)
    {
        sample_identity_ = sample_id;
        return *this;
    }

    /**
     * Set the value of the sample_identity member.
     *
     * @param sample_id  New value for the sample_identity member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& sample_identity(
            SampleIdentity&& sample_id)
    {
        sample_identity_ = std::move(sample_id);
        return *this;
    }

    /**
     * Get the value of the sample_identity member.
     *
     * @return Constant reference to the sample_identity member.
     */
    const SampleIdentity& sample_identity() const
    {
        return sample_identity_;
    }

    /**
     * Set the value of the sample_identity member.
     *
     * @return Reference to the sample_identity member.
     */
    SampleIdentity& sample_identity()
    {
        return sample_identity_;
    }

    /**
     * Set the value of the related_sample_identity member of this class.
     *
     * @param sample_id  New value for the related_sample_identity member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& related_sample_identity(
            const SampleIdentity& sample_id)
    {
        related_sample_identity_ = sample_id;
        return *this;
    }

    /**
     * Set the related_sample_identity member of this class.
     *
     * @param sample_id  New value for the related_sample_identity member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& related_sample_identity(
            SampleIdentity&& sample_id)
    {
        related_sample_identity_ = std::move(sample_id);
        return *this;
    }

    /**
     * Get the value of the related_sample_identity member.
     *
     * @return Constant reference to the related_sample_identity member.
     */
    const SampleIdentity& related_sample_identity() const
    {
        return related_sample_identity_;
    }

    /**
     * Set the value of the related_sample_identity member.
     *
     * @return Reference to the related_sample_identity member.
     */
    SampleIdentity& related_sample_identity()
    {
        return related_sample_identity_;
    }

    /**
     * Get the value of the source_timestamp member.
     *
     * @return Current value of the source_timestamp member.
     */
    Time_t source_timestamp() const
    {
        return source_timestamp_;
    }

    /**
     * Set the value of the source_timestamp member.
     *
     * @return Reference to the source_timestamp member.
     */
    Time_t& source_timestamp()
    {
        return source_timestamp_;
    }

    /**
     * Set the source_timestamp member of this class.
     *
     * @param timestamp  New value for the source_timestamp member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& source_timestamp(
            const Time_t& timestamp)
    {
        source_timestamp_ = timestamp;
        return *this;
    }

    /**
     * Set the source_timestamp member of this class.
     *
     * @param timestamp  New value for the source_timestamp member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& source_timestamp(
            Time_t&& timestamp)
    {
        source_timestamp_ = std::move(timestamp);
        return *this;
    }

    /**
     * @brief Retrieves the user write data.
     *
     * @return Shared pointer to the user write data.
     */
    std::shared_ptr<UserWriteData> user_write_data() const
    {
        return user_write_data_;
    }

    /**
     * Set the user write data.
     *
     * @param write_data  New value for the user_write_data member.
     *
     * @return Reference to the modified object in order to allow daisy chaining.
     */
    WriteParams& user_write_data(
            std::shared_ptr<UserWriteData> write_data)
    {
        user_write_data_ = write_data;
        return *this;
    }

    bool has_more_replies() const
    {
        return has_more_replies_;
    }

    WriteParams& has_more_replies(
            bool more_replies)
    {
        has_more_replies_ = more_replies;
        return *this;
    }

    static WriteParams WRITE_PARAM_DEFAULT;

    /**
     * Default value for methods receiving a WriteParams.
     *
     * Will contain the following values on its members:
     * - sample_identity: Invalid SampleIdentity
     * - related_sample_identity: Invalid SampleIdentity
     * - source_timestamp: Invalid Time_t
     *
     * @note This should not return a reference to the static value if this value is meant to be
     * read and written from different threads.
     */
    static WriteParams write_params_default() noexcept
    {
        return WriteParams();
    }

private:

    class FASTDDS_EXPORTED_API UserWriteDataPtr : public std::shared_ptr<UserWriteData>
    {
    public:

        UserWriteDataPtr(
                std::shared_ptr<UserWriteData> ptr)
            : std::shared_ptr<UserWriteData>(ptr)
        {
        }

    };

    /// Attribute that holds sample_identity member value
    SampleIdentity sample_identity_;
    /// Attribute that holds related_sample_identity member value
    SampleIdentity related_sample_identity_;
    /// Attribute that holds source_timestamp member value
    Time_t source_timestamp_{ -1, TIME_T_INFINITE_NANOSECONDS };
    /// User write data
    UserWriteDataPtr user_write_data_{nullptr};
    /// Flag to indicate if there are more replies
    bool has_more_replies_ = false;
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif //FASTDDS_RTPS_COMMON__WRITEPARAMS_HPP
