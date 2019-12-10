#pragma once
#include <stdint.h>

#include <pal.h>

/* Structs needed by e-lib */
typedef enum {
    E_NULL         = 0,
    E_EPI_PLATFORM = 1,
    E_EPI_CHIP     = 2,
    E_EPI_GROUP    = 3,
    E_EPI_CORE     = 4,
    E_EXT_MEM      = 5,
    E_MAPPING      = 6,
    E_SHARED_MEM   = 7
} e_objtype_t;

typedef enum {
    E_E16G301 = 0,
    E_E64G401 = 1,
} e_chiptype_t;

typedef struct {
    uint32_t objtype;           // 0x28
    uint32_t chiptype;          // 0x2c
    uint32_t group_id;          // 0x30
    uint32_t group_row;         // 0x34
    uint32_t group_col;         // 0x38
    uint32_t group_rows;        // 0x3c
    uint32_t group_cols;        // 0x40
    uint32_t core_row;          // 0x44
    uint32_t core_col;          // 0x48
    uint32_t alignment_padding; // 0x4c
} __attribute__((packed)) __attribute__((aligned(4))) e_group_config_t;

typedef struct {
    uint32_t objtype;           // 0x50
    uint32_t base;              // 0x54
} __attribute__((packed)) __attribute__((aligned(4))) e_emem_config_t;

/* Loader arguments */
struct loader_args {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t function_ptr;
    uint32_t function_ptr_hi32;    // upper 32 bits
    uint32_t stack_spill_size;     // (8-byte aligned)
    uint32_t __pad;                // Reserved, must be 0
    uint32_t stack_spill_ptr;      // (8-byte aligned)
    uint32_t stack_spill_ptr_hi32; // upper 32 bits
} __attribute__((packed)) __attribute__((aligned(4)));

struct loader_cfg {
    uint32_t flags;
    uint32_t __pad1;
    uint32_t args_ptr;
    uint32_t __pad2;
} __attribute__((packed))  __attribute__((aligned(4)));

// Loader flags for crt0
#define LOADER_BSS_CLEARED_FLAG 1
#define LOADER_CUSTOM_ARGS_FLAG 2

// For __pal_epiphany_coords (see boilerplate.c / loader.c:set_core_config())
struct pal_epiphany_coords {
    p_coords_t default_dev_start;
    p_coords_t default_dev_size;
    int default_team_topology;
    p_coords_t default_team_start;
    p_coords_t default_team_size;
    p_coords_t default_team_rank;
    int device_rank; /* My device rank */
    uint32_t eram_base;
    uint32_t eram_size;
} __attribute__((packed)) __attribute__((aligned(4)));
