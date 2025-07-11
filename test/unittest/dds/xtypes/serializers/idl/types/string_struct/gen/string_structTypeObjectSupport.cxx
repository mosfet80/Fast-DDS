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
 * @file string_structTypeObjectSupport.cxx
 * Source file containing the implementation to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen (version: 4.1.0).
 */

#include "string_structTypeObjectSupport.hpp"

#include <mutex>
#include <string>

#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include "string_struct.hpp"


using namespace eprosima::fastdds::dds::xtypes;

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_StringStruct_type_identifier(
        TypeIdentifierPair& type_ids_StringStruct)
{

    ReturnCode_t return_code_StringStruct {eprosima::fastdds::dds::RETCODE_OK};
    return_code_StringStruct =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
        "StringStruct", type_ids_StringStruct);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_StringStruct)
    {
        StructTypeFlag struct_flags_StringStruct = TypeObjectUtils::build_struct_type_flag(eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
                false, false);
        QualifiedTypeName type_name_StringStruct = "StringStruct";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_StringStruct;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_StringStruct;
        AppliedAnnotationSeq tmp_ann_custom_StringStruct;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_StringStruct;
        if (!tmp_ann_custom_StringStruct.empty())
        {
            ann_custom_StringStruct = tmp_ann_custom_StringStruct;
        }

        CompleteTypeDetail detail_StringStruct = TypeObjectUtils::build_complete_type_detail(type_ann_builtin_StringStruct, ann_custom_StringStruct, type_name_StringStruct.to_string());
        CompleteStructHeader header_StringStruct;
        header_StringStruct = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_StringStruct);
        CompleteStructMemberSeq member_seq_StringStruct;
        {
            TypeIdentifierPair type_ids_my_string;
            ReturnCode_t return_code_my_string {eprosima::fastdds::dds::RETCODE_OK};
            return_code_my_string =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_string_unbounded", type_ids_my_string);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_my_string)
            {
                {
                    SBound bound = 0;
                    StringSTypeDefn string_sdefn = TypeObjectUtils::build_string_s_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_string_type_identifier(string_sdefn,
                            "anonymous_string_unbounded", type_ids_my_string))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_string_unbounded already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_my_string = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_my_string = 0x00000000;
            bool common_my_string_ec {false};
            CommonStructMember common_my_string {TypeObjectUtils::build_common_struct_member(member_id_my_string, member_flags_my_string, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_my_string, common_my_string_ec))};
            if (!common_my_string_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure my_string member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_my_string = "my_string";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_my_string;
            ann_custom_StringStruct.reset();
            CompleteMemberDetail detail_my_string = TypeObjectUtils::build_complete_member_detail(name_my_string, member_ann_builtin_my_string, ann_custom_StringStruct);
            CompleteStructMember member_my_string = TypeObjectUtils::build_complete_struct_member(common_my_string, detail_my_string);
            TypeObjectUtils::add_complete_struct_member(member_seq_StringStruct, member_my_string);
        }
        {
            TypeIdentifierPair type_ids_my_wstring;
            ReturnCode_t return_code_my_wstring {eprosima::fastdds::dds::RETCODE_OK};
            return_code_my_wstring =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_wstring_unbounded", type_ids_my_wstring);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_my_wstring)
            {
                {
                    SBound bound = 0;
                    StringSTypeDefn string_sdefn = TypeObjectUtils::build_string_s_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_string_type_identifier(string_sdefn,
                            "anonymous_wstring_unbounded", type_ids_my_wstring, true))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_wstring_unbounded already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_my_wstring = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_my_wstring = 0x00000001;
            bool common_my_wstring_ec {false};
            CommonStructMember common_my_wstring {TypeObjectUtils::build_common_struct_member(member_id_my_wstring, member_flags_my_wstring, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_my_wstring, common_my_wstring_ec))};
            if (!common_my_wstring_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure my_wstring member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_my_wstring = "my_wstring";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_my_wstring;
            ann_custom_StringStruct.reset();
            CompleteMemberDetail detail_my_wstring = TypeObjectUtils::build_complete_member_detail(name_my_wstring, member_ann_builtin_my_wstring, ann_custom_StringStruct);
            CompleteStructMember member_my_wstring = TypeObjectUtils::build_complete_struct_member(common_my_wstring, detail_my_wstring);
            TypeObjectUtils::add_complete_struct_member(member_seq_StringStruct, member_my_wstring);
        }
        {
            TypeIdentifierPair type_ids_my_bounded_string;
            ReturnCode_t return_code_my_bounded_string {eprosima::fastdds::dds::RETCODE_OK};
            return_code_my_bounded_string =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_string_41925", type_ids_my_bounded_string);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_my_bounded_string)
            {
                {
                    LBound bound = 41925;
                    StringLTypeDefn string_ldefn = TypeObjectUtils::build_string_l_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_l_string_type_identifier(string_ldefn,
                            "anonymous_string_41925", type_ids_my_bounded_string))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_string_41925 already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_my_bounded_string = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_my_bounded_string = 0x00000002;
            bool common_my_bounded_string_ec {false};
            CommonStructMember common_my_bounded_string {TypeObjectUtils::build_common_struct_member(member_id_my_bounded_string, member_flags_my_bounded_string, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_my_bounded_string, common_my_bounded_string_ec))};
            if (!common_my_bounded_string_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure my_bounded_string member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_my_bounded_string = "my_bounded_string";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_my_bounded_string;
            ann_custom_StringStruct.reset();
            CompleteMemberDetail detail_my_bounded_string = TypeObjectUtils::build_complete_member_detail(name_my_bounded_string, member_ann_builtin_my_bounded_string, ann_custom_StringStruct);
            CompleteStructMember member_my_bounded_string = TypeObjectUtils::build_complete_struct_member(common_my_bounded_string, detail_my_bounded_string);
            TypeObjectUtils::add_complete_struct_member(member_seq_StringStruct, member_my_bounded_string);
        }
        {
            TypeIdentifierPair type_ids_my_bounded_wstring;
            ReturnCode_t return_code_my_bounded_wstring {eprosima::fastdds::dds::RETCODE_OK};
            return_code_my_bounded_wstring =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().get_type_identifiers(
                "anonymous_wstring_20925", type_ids_my_bounded_wstring);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_my_bounded_wstring)
            {
                {
                    LBound bound = 20925;
                    StringLTypeDefn string_ldefn = TypeObjectUtils::build_string_l_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_l_string_type_identifier(string_ldefn,
                            "anonymous_wstring_20925", type_ids_my_bounded_wstring, true))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                            "anonymous_wstring_20925 already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_my_bounded_wstring = TypeObjectUtils::build_struct_member_flag(eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                    false, false, false, false);
            MemberId member_id_my_bounded_wstring = 0x00000003;
            bool common_my_bounded_wstring_ec {false};
            CommonStructMember common_my_bounded_wstring {TypeObjectUtils::build_common_struct_member(member_id_my_bounded_wstring, member_flags_my_bounded_wstring, TypeObjectUtils::retrieve_complete_type_identifier(type_ids_my_bounded_wstring, common_my_bounded_wstring_ec))};
            if (!common_my_bounded_wstring_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure my_bounded_wstring member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_my_bounded_wstring = "my_bounded_wstring";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_my_bounded_wstring;
            ann_custom_StringStruct.reset();
            CompleteMemberDetail detail_my_bounded_wstring = TypeObjectUtils::build_complete_member_detail(name_my_bounded_wstring, member_ann_builtin_my_bounded_wstring, ann_custom_StringStruct);
            CompleteStructMember member_my_bounded_wstring = TypeObjectUtils::build_complete_struct_member(common_my_bounded_wstring, detail_my_bounded_wstring);
            TypeObjectUtils::add_complete_struct_member(member_seq_StringStruct, member_my_bounded_wstring);
        }
        CompleteStructType struct_type_StringStruct = TypeObjectUtils::build_complete_struct_type(struct_flags_StringStruct, header_StringStruct, member_seq_StringStruct);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_StringStruct, type_name_StringStruct.to_string(), type_ids_StringStruct))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "StringStruct already registered in TypeObjectRegistry for a different type.");
        }
    }
}
