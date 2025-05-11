#ifndef CYRUS_ART_METHOD_H
#define CYRUS_ART_METHOD_H

#include <stdint.h>
#include <string>

namespace cyurs {

    // android9+
    namespace V28 {
        class ArtMethod {
        public:
            // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
            // The class we are a part of.
            uint8_t declaring_class_;

            // Access flags; low 16 bits are defined by spec.
            // Getting and setting this flag needs to be atomic when concurrency is
            // possible, e.g. after this method's class is linked. Such as when setting
            // verifier flags and single-implementation flag.
            uint32_t access_flags_;

            /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

            // Offset to the CodeItem.
            uint32_t dex_code_item_offset_;

            // Index into method_ids of the dex file associated with this method.
            uint32_t dex_method_index_;

            /* End of dex file fields. */

            // Entry within a dispatch table for this method. For static/direct methods the index is into
            // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
            // ifTable.
            uint16_t method_index_;

            union {
                // Non-abstract methods: The hotness we measure for this method. Not atomic,
                // as we allow missing increments: if the method is hot, we will see it eventually.
                uint16_t hotness_count_;
                // Abstract methods: IMT index (bitwise negated) or zero if it was not cached.
                // The negation is needed to distinguish zero index and missing cached entry.
                uint16_t imt_index_;
            };
        };

    } //namespace V28


    // android 12 开始去掉了 dex_code_item_offset_ 字段
    namespace V31 {

        class ArtMethod {
        public:
            // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
            // The class we are a part of.
            uint8_t declaring_class_;

            // Access flags; low 16 bits are defined by spec.
            // Getting and setting this flag needs to be atomic when concurrency is
            // possible, e.g. after this method's class is linked. Such as when setting
            // verifier flags and single-implementation flag.
            uint32_t access_flags_;

            /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

            // Index into method_ids of the dex file associated with this method.
            uint32_t dex_method_index_;

            /* End of dex file fields. */

            // Entry within a dispatch table for this method. For static/direct methods the index is into
            // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
            // ifTable.
            uint16_t method_index_;
        };

    } //namespace V35

};//namespace cyrus

#endif //CYRUS_ART_METHOD_H
