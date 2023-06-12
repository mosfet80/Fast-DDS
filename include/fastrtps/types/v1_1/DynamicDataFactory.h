// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_1_1_DYNAMIC_DATA_FACTORY_H
#define TYPES_1_1_DYNAMIC_DATA_FACTORY_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_1/DynamicTypePtr.h>
#include <fastrtps/types/v1_1/DynamicTypeBuilder.h>
#include <fastrtps/types/v1_1/DynamicType.h>
#include <fastrtps/types/v1_1/DynamicData.h>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace types {
// v1_1 is the default implementation but deprecated
inline namespace v1_1 {

class DynamicDataFactory
{
protected:

    DynamicDataFactory();

    ReturnCode_t create_members(
            DynamicData* pData,
            DynamicType_ptr pType);

#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::vector<DynamicData*> dynamic_datas_;
    mutable std::recursive_mutex mutex_;
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK

public:

    ~DynamicDataFactory();

    RTPS_DllAPI static DynamicDataFactory* get_instance();

    RTPS_DllAPI static ReturnCode_t delete_instance();

    RTPS_DllAPI DynamicData* create_data(
            DynamicTypeBuilder* pBuilder);

    RTPS_DllAPI DynamicData* create_data(
            DynamicType_ptr pType);

    RTPS_DllAPI DynamicData* create_copy(
            const DynamicData* pData);

    RTPS_DllAPI ReturnCode_t delete_data(
            DynamicData* pData);

    RTPS_DllAPI bool is_empty() const;
};

} // namespace v1_1
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_1_DYNAMIC_DATA_FACTORY_H