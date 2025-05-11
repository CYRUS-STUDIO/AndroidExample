#ifndef CYRUS_METHOD_H
#define CYRUS_METHOD_H

#include <stdint.h>
#include <string>


namespace cyurs {

    class BaseItem {
    public:
        // Internal data pointer for reading.
        const void* dex_file_;
        const uint8_t* ptr_pos_ = nullptr;
        const uint8_t* hiddenapi_ptr_pos_ = nullptr;
        uint32_t index_ = 0u;
        uint32_t access_flags_ = 0u;
        uint32_t hiddenapi_flags_ = 0u;
    };

    // A decoded version of the method of a class_data_item.
    class Method : public BaseItem {
    public:
        bool is_static_or_direct_ = true;
        uint32_t code_off_ = 0u;
    };

};//namespace cyrus

#endif //CYRUS_METHOD_H
