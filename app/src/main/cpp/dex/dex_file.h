#ifndef CYURS_DEX_FILE_H
#define CYURS_DEX_FILE_H

#include <stdint.h>
#include <string>

namespace cyurs {
    namespace dex {
        struct Header {
        public:
            uint8_t magic_[8];
            uint32_t checksum_;  // See also location_checksum_
            uint8_t signature_[20];
            uint32_t file_size_;  // size of entire file
            uint32_t header_size_;  // offset to start of next section
            uint32_t endian_tag_;
            uint32_t link_size_;  // unused
            uint32_t link_off_;  // unused
            uint32_t map_off_;  // unused
            uint32_t string_ids_size_;  // number of StringIds
            uint32_t string_ids_off_;  // file offset of StringIds array
            uint32_t type_ids_size_;  // number of TypeIds, we don't support more than 65535
            uint32_t type_ids_off_;  // file offset of TypeIds array
            uint32_t proto_ids_size_;  // number of ProtoIds, we don't support more than 65535
            uint32_t proto_ids_off_;  // file offset of ProtoIds array
            uint32_t field_ids_size_;  // number of FieldIds
            uint32_t field_ids_off_;  // file offset of FieldIds array
            uint32_t method_ids_size_;  // number of MethodIds
            uint32_t method_ids_off_;  // file offset of MethodIds array
            uint32_t class_defs_size_;  // number of ClassDefs
            uint32_t class_defs_off_;  // file offset of ClassDef array
            uint32_t data_size_;  // unused
            uint32_t data_off_;  // unused
        };

        struct MapItem {
            uint16_t type_;
            uint16_t unused_;
            uint32_t size_;
            uint32_t offset_;
        };

        struct MapList {
            uint32_t size_;
            MapItem list_[1];
        };

        // Raw string_id_item.
        struct StringId {
            uint32_t string_data_off_;  // offset in bytes from the base address
        };
        // Raw type_id_item.
        struct TypeId {
            uint32_t descriptor_idx_;  // index into string_ids
        };
        // Raw field_id_item.
        struct FieldId {
            uint16_t class_idx_;  // index into type_ids_ array for defining class
            uint16_t type_idx_;  // index into type_ids_ array for field type
            uint32_t name_idx_;  // index into string_ids_ array for field name
        };
        // Raw method_id_item.
        struct MethodId {
            uint16_t class_idx_;  // index into type_ids_ array for defining class
            uint16_t proto_idx_;  // index into proto_ids_ array for method prototype
            uint32_t name_idx_;  // index into string_ids_ array for method name

        };
        // Raw proto_id_item.
        struct ProtoId {
            uint32_t shorty_idx_;  // index into string_ids array for shorty descriptor
            uint16_t return_type_idx_;  // index into type_ids array for return type
            uint16_t pad_;             // padding = 0
            uint32_t parameters_off_;  // file offset to type_list for parameter types
        };
        // Raw class_def_item.
        struct ClassDef {
        public:
            uint32_t class_idx_;  // index into type_ids_ array for this class
            uint32_t access_flags_;
            uint32_t superclass_idx_;  // index into type_ids_ array for superclass
            uint32_t interfaces_off_;  // file offset to TypeList
            uint32_t source_file_idx_;  // index into string_ids_ for source file name
            uint32_t annotations_off_;  // file offset to annotations_directory_item
            uint32_t class_data_off_;  // file offset to class_data_item
            uint32_t static_values_off_;  // file offset to EncodedArray
        };
        // Raw code_item.
        struct CodeItem {
        public:
            uint16_t registers_size_;            // the number of registers used by this code
            //   (locals + parameters)
            uint16_t ins_size_;                  // the number of words of incoming arguments to the method
            //   that this code is for
            uint16_t outs_size_;                 // the number of words of outgoing argument space required
            //   by this code for method invocation
            uint16_t tries_size_;                // the number of try_items for this instance. If non-zero,
            //   then these appear as the tries array just after the
            //   insns in this instance.
            uint32_t debug_info_off_;            // file offset to debug info stream
            uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
            uint16_t insns_[1];                  // actual array of bytecode.
        };
        // Raw try_item.
        struct TryItem {
            uint32_t start_addr_;
            uint16_t insn_count_;
            uint16_t handler_off_;
        };

        struct ClassDataHeader {
            uint32_t static_fields_size_;  // the number of static fields
            uint32_t instance_fields_size_;  // the number of instance fields
            uint32_t direct_methods_size_;  // the number of direct methods
            uint32_t virtual_methods_size_;  // the number of virtual methods
        };

        struct ClassDataField {
        public:
            uint32_t field_idx_delta_;  // delta of index into the field_ids array for FieldId
            uint32_t access_flags_;  // access flags for the field

            ClassDataField(uint32_t field_idx_delta_, uint32_t access_flags_) :
                    field_idx_delta_(field_idx_delta_), access_flags_(access_flags_) {
            }

            ClassDataField() : field_idx_delta_(0), access_flags_(0) {
            }
        };

        // A decoded version of the method of a class_data_item
        struct ClassDataMethod {
        public:
            uint32_t method_idx_delta_;  // delta of index into the method_ids array for MethodId
            uint32_t access_flags_;
            uint32_t code_off_;

            ClassDataMethod(uint32_t method_idx_delta_, uint32_t access_flags_, uint32_t code_off_)
                    :
                    method_idx_delta_(method_idx_delta_), access_flags_(access_flags_),
                    code_off_(code_off_) {
            }

            ClassDataMethod() :
                    method_idx_delta_(0), access_flags_(0), code_off_(0) {
            }
        };
    };

    namespace V21 {
        class DexFile {
        public:
            //vtable pointer
            void *_;

            // The base address of the memory mapping.
            const uint8_t *const begin_;

            // The size of the underlying memory allocation in bytes.
            const size_t size_;

            // Typically the dex file name when available, alternatively some identifying string.
            //
            // The ClassLinker will use this to match DexFiles the boot class
            // path to DexCache::GetLocation when loading from an image.
            const std::string location_;

            const uint32_t location_checksum_;

            // Manages the underlying memory allocation.
            std::unique_ptr<void *> mem_map_;

            // Points to the header section.
            const dex::Header *const header_;

            // Points to the base of the string identifier list.
            const dex::StringId *const string_ids_;

            // Points to the base of the type identifier list.
            const dex::TypeId *const type_ids_;

            // Points to the base of the field identifier list.
            const dex::FieldId *const field_ids_;

            // Points to the base of the method identifier list.
            const dex::MethodId *const method_ids_;

            // Points to the base of the prototype identifier list.
            const dex::ProtoId *const proto_ids_;

            // Points to the base of the class definition list.
            const dex::ClassDef *const class_defs_;

        };
    } //namespace V21

    namespace V28 {
        class DexFile {
        public:
            //vtable pointer
            void *_;

            // The base address of the memory mapping.
            const uint8_t *const begin_;

            // The size of the underlying memory allocation in bytes.
            const size_t size_;

            // The base address of the data section (same as Begin() for standard dex).
            const uint8_t *const data_begin_;

            // The size of the data section.
            const size_t data_size_;

            // Typically the dex file name when available, alternatively some identifying string.
            //
            // The ClassLinker will use this to match DexFiles the boot class
            // path to DexCache::GetLocation when loading from an image.
            const std::string location_;

            const uint32_t location_checksum_;

            // Points to the header section.
            const dex::Header *const header_;

            // Points to the base of the string identifier list.
            const dex::StringId *const string_ids_;

            // Points to the base of the type identifier list.
            const dex::TypeId *const type_ids_;

            // Points to the base of the field identifier list.
            const dex::FieldId *const field_ids_;

            // Points to the base of the method identifier list.
            const dex::MethodId *const method_ids_;

            // Points to the base of the prototype identifier list.
            const dex::ProtoId *const proto_ids_;

            // Points to the base of the class definition list.
            const dex::ClassDef *const class_defs_;

        };
    } //namespace V28

    namespace V35 {
        template <typename T>
        class ArrayRef {
        private:
            T* array_;
            size_t size_;
        };
        class DexFile {
        public:
            void *_;

            // The base address of the memory mapping.
            const uint8_t* begin_;

            size_t unused_size_ = 0;  // Preserve layout for DRM (b/305203031).

            // Data memory range: Most dex offsets are relative to this memory range.
            // Standard dex: same as (begin_, size_).
            // Dex container: all dex files (starting from the first header).
            // Compact: shared data which is located after all non-shared data.
            //
            // This is different to the "data section" in the standard dex header.
            ArrayRef<const uint8_t> const data_;

            // The full absolute path to the dex file, if it was loaded from disk.
            //
            // Can also be a path to a multidex container (typically apk), followed by
            // DexFileLoader.kMultiDexSeparator (i.e. '!') and the file inside the
            // container.
            //
            // On host this may not be an absolute path.
            //
            // On device libnativeloader uses this to determine the location of the java
            // package or shared library, which decides where to load native libraries
            // from.
            //
            // The ClassLinker will use this to match DexFiles the boot class
            // path to DexCache::GetLocation when loading from an image.
            const std::string location_;

            uint32_t location_checksum_;

            // Points to the header section.
            const dex::Header*  header_;

            // Points to the base of the string identifier list.
            const dex::StringId*  string_ids_;

            // Points to the base of the type identifier list.
            const dex::TypeId* type_ids_;

            // Points to the base of the field identifier list.
            const dex::FieldId* field_ids_;

            // Points to the base of the method identifier list.
            const dex::MethodId* method_ids_;

            // Points to the base of the prototype identifier list.
            const dex::ProtoId* proto_ids_;

            // Points to the base of the class definition list.
            const dex::ClassDef* class_defs_;
        };
    } //namespace V35
};//namespace cyrus

#endif //CYURS_DEX_FILE_H
