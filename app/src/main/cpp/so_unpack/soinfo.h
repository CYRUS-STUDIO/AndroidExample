#pragma once

#include <link.h>
#include <memory>
#include <string>
#include <vector>
#include <jni.h>

typedef void (*linker_dtor_function_t)();
typedef void (*linker_ctor_function_t)(int, char**, char**);


struct soinfo {
public:
    const ElfW(Phdr)* phdr;
    size_t phnum;
    ElfW(Addr) base;
    size_t size;
    ElfW(Dyn)* dynamic;
    soinfo* next;

    uint32_t flags_;
    const char* strtab_;
    ElfW(Sym)* symtab_;
    size_t nbucket_;
    size_t nchain_;
    uint32_t* bucket_;
    uint32_t* chain_;


    ElfW(Rela)* plt_rela_;
    size_t plt_rela_count_;

    ElfW(Rela)* rela_;
    size_t rela_count_;


    uint8_t* preinit_array_;
    size_t preinit_array_count_;

    uint8_t* init_array_;
    size_t init_array_count_;
    uint8_t* fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;

#if defined(__arm__)
    public:
  // ARM EABI section used for stack unwinding.
  uint32_t* ARM_exidx;
  size_t ARM_exidx_count;
#endif
    size_t ref_count_;

    link_map link_map_head;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    ElfW(Addr) load_bias;

    bool has_DT_SYMBOLIC;

    // This part of the structure is only available
    // when FLAG_NEW_SOINFO is set in this->flags.
    uint32_t version_;

    // version >= 0
    dev_t st_dev_;
    ino_t st_ino_;

    // dependency graph
    uint8_t* children_;
    uint8_t* parents_;

    // version >= 1
    off64_t file_offset_;
    uint32_t rtld_flags_;
    uint32_t dt_flags_1_;
    size_t strtab_size_;

    // version >= 2

    size_t gnu_nbucket_;
    uint32_t* gnu_bucket_;
    uint32_t* gnu_chain_;
    uint32_t gnu_maskwords_;
    uint32_t gnu_shift2_;
    ElfW(Addr)* gnu_bloom_filter_;

    soinfo* local_group_root_;

    uint8_t* android_relocs_;
    size_t android_relocs_size_;

    const char* soname_;
    std::string realpath_;
};
