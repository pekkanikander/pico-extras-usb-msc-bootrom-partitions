#pragma once
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
  // C++11 and later: char16_t is a built-in type
#else
  typedef uint16_t char16_t;
#endif

#ifndef __INTELLISENSE__
  // in real builds, expand to the C11 keyword
  #define STATIC_ASSERT_PACKED(cond, msg) _Static_assert(cond, msg)
#else
  // under IntelliSense, drop all these checks
  #define STATIC_ASSERT_PACKED(cond, msg)
#endif

// exFAT directory‐entry type codes (entry_type field)
typedef enum {
    exfat_entry_type_end_of_directory  = 0x00, ///< End-of-directory marker
    exfat_entry_type_allocation_bitmap = 0x81, ///< Allocation Bitmap
    exfat_entry_type_upcase_table      = 0x82, ///< Up‐case Table
    exfat_entry_type_volume_label      = 0x83, ///< Volume Label
    exfat_entry_type_file_directory    = 0x85, ///< File Directory Entry
    exfat_entry_type_volume_guid       = 0xA0, ///< Volume GUID entry
    exfat_entry_type_stream_extension  = 0xC0, ///< Stream Extension Entry
    exfat_entry_type_file_name         = 0xC1, ///< File Name Entry
} exfat_entry_type_t;

// exFAT Generic Directory Entry (32 bytes)
/// See Microsoft spec § 6.2 “Generic DirectoryEntry template” (Table 14)
typedef struct __packed {
    exfat_entry_type_t entry_type;        ///< Directory entry type code
    uint8_t            entry_specific[19];///< Reserved or structure-specific data
    uint32_t           first_cluster;     ///< FirstCluster: starting cluster of this entry’s data (if any)
    uint64_t           data_length;       ///< DataLength: size in bytes of this entry’s data (if any)
} exfat_generic_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_generic_dir_entry_t) == 32, 
   "Generic exFAT directory entry must be 32 bytes");

/// exFAT Allocation Bitmap Directory Entry (entry_type = 0x81)
/// See Microsoft spec §7.1 “Allocation Bitmap Directory Entry” (Table 20)
typedef struct __packed {
    exfat_entry_type_t entry_type;   ///< Entry type (0x81)
    uint8_t            bitmap_flags; ///< BitmapFlags (bitmask of valid clusters)
    uint8_t            reserved1[18]; ///< Reserved; must be zero
    uint32_t           first_cluster;///< Cluster number of the allocation bitmap file
    uint64_t           data_length;  ///< Size in bytes of the allocation bitmap
} exfat_allocation_bitmap_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_allocation_bitmap_dir_entry_t) == 32,
    "Allocation Bitmap exFAT directory entry must be 32 bytes");

/// exFAT Up-case Table Directory Entry (entry_type = 0x82)
/// See Microsoft spec §7.2 “Up-case Table Directory Entry” (Table 23)
typedef struct __packed {
    exfat_entry_type_t entry_type;    ///< Entry type (0x82)
    uint8_t            reserved1[3];  ///< Reserved; must be zero
    uint32_t           table_checksum;///< Checksum of the up-case table data
    uint8_t            reserved2[12]; ///< Reserved; must be zero
    uint32_t           first_cluster; ///< Cluster number of the up-case table file
    uint64_t           data_length;   ///< Size in bytes of the up-case table
} exfat_upcase_table_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_upcase_table_dir_entry_t) == 32,
    "Up-case Table exFAT directory entry must be 32 bytes");

/// exFAT Volume Label Directory Entry (entry_type = 0x83)
/// See Microsoft spec §7.3 “Volume Label Directory Entry” (Table 26)
typedef struct __packed {
    exfat_entry_type_t entry_type;    ///< Entry type (0x83)
    uint8_t            char_count;    ///< Character count of the volume label (0–11)
    char16_t           volume_label[11]; ///< Volume label in UTF-16 (padded with 0)
    uint8_t            reserved[8];   ///< Reserved; must be zero
} exfat_volume_label_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_volume_label_dir_entry_t) == 32,
    "Volume Label exFAT directory entry must be 32 bytes");


/// exFAT File Directory Entry attribute bits (Table 28)
typedef enum  {
    EXFAT_FILE_ATTR_READ_ONLY  = 0x0001,  ///< read-only file
    EXFAT_FILE_ATTR_HIDDEN     = 0x0002,  ///< hidden file
    EXFAT_FILE_ATTR_SYSTEM     = 0x0004,  ///< system file
    EXFAT_FILE_ATTR_ARCHIVE    = 0x0020,  ///< archive bit
} exfat_file_attr_t;

/// exFAT timestamp field (32 bits; see Table 29 §7.4.8)
typedef uint32_t exfat_timestamp_t;

/// §7.4.9 “10msIncrement Fields"
/// These fields are not used in this read-only volume (always zero).

/// §7.4.10 “UtcOffset Fields" (signed 15-minute increments, Table 31)  
/// We only support UTC (offset = 0 minutes).
typedef enum {
    exfat_utc_offset_UTC = 0x80  ///< OffsetValid | Coordinated Universal Time
} exfat_utc_offset_t;

/// exFAT File Directory Entry (entry_type = 0x85)
/// See Microsoft spec §7.4 “File Directory Entry” (Table 27)
typedef struct __packed {
    exfat_entry_type_t   entry_type;        ///< Entry type (0x85)
    uint8_t              secondary_count;   ///< Number of secondary entries
    uint16_t             set_checksum;      ///< Checksum of the set of entries
    exfat_file_attr_t    file_attributes;   ///< File attributes 
    uint8_t              reserved1[2];      ///< Reserved; must be zero 
    exfat_timestamp_t    creat_time;        ///< Creation time
    exfat_timestamp_t    last_mod_time;     ///< Last modification time
    exfat_timestamp_t    last_acc_time;     ///< Last access time
    uint8_t              creat_time_ms;     ///< Creation time 10ms increments
    uint8_t              last_mod_time_ms;  ///< Last modification time 10ms increments
    uint8_t              last_acc_time_ms;  ///< Last access 10ms increments
    exfat_utc_offset_t   creat_time_off;    ///< Creation time UTC offset
    exfat_utc_offset_t   last_mod_time_off; ///< Last modification time UTC offset
    exfat_utc_offset_t   last_acc_time_off; ///< Last access time UTC offset
    uint8_t              reserved2[7];      ///< Reserved; must be zero 
} exfat_file_directory_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_file_directory_dir_entry_t) == 32,
    "File Directory exFAT directory entry must be 32 bytes");

/// exFAT Volume GUID Directory Entry (entry_type = 0xA0)
/// See Microsoft spec §7.5 “Volume GUID Directory Entry” (Table 32)
typedef struct __packed {
    exfat_entry_type_t entry_type;           ///< Entry type (0xA0)
    uint8_t            secondary_count;      ///< SecondaryCount; must be 0
    uint16_t           set_checksum;         ///< SetChecksum
    uint16_t           general_primary_flags;///< GeneralPrimaryFlags; must be 0
    uint8_t            volume_guid[16];      ///< Volume GUID (all values valid except null GUID)
    uint8_t            reserved[10];         ///< Reserved; must be zero
} exfat_volume_guid_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_volume_guid_dir_entry_t) == 32,
    "Volume GUID exFAT directory entry must be 32 bytes");

/// exFAT Stream Extension Directory Entry (entry_type = 0xC0)
/// See Microsoft spec §7.6 “Stream Extension Directory Entry” (Table 33)
typedef struct __packed {
    exfat_entry_type_t entry_type;             ///< Entry type (0xC0)
    uint8_t            general_secondary_flags;///< GeneralSecondaryFlags; AllocationPossible must be 1
    uint8_t            reserved1;              ///< Reserved; must be zero
    uint8_t            name_length;            ///< NameLength: length of file name in bytes (1–255)
    uint16_t           name_hash;              ///< NameHash: hash of the up-cased file name
    uint16_t           reserved2;              ///< Reserved; must be zero
    uint64_t           valid_data_length;      ///< ValidDataLength: count of valid bytes written
    uint32_t           reserved3;              ///< Reserved; must be zero
    uint32_t           first_cluster;          ///< FirstCluster: starting cluster of the data stream
    uint64_t           data_length;            ///< DataLength: total size in bytes of the data stream
} exfat_stream_extension_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_stream_extension_dir_entry_t) == 32,
    "Stream Extension exFAT directory entry must be 32 bytes");

/// exFAT File Name Directory Entry (entry_type = 0xC1)
/// See Microsoft spec §7.7 “File Name Directory Entry” (Table 34)
typedef struct __packed {
    exfat_entry_type_t entry_type;            ///< Entry type (0xC1)
    uint8_t            general_secondary_flags;///< GeneralSecondaryFlags; AllocationPossible must be 0
    uint16_t           file_name[15];         ///< FileName: array of 15 UTF-16 characters
} exfat_file_name_dir_entry_t;
STATIC_ASSERT_PACKED(sizeof(exfat_file_name_dir_entry_t) == 32,
    "File Name exFAT directory entry must be 32 bytes");

/// exFAT first root directory entries
typedef struct __packed exfat_first_root_dir_entries {
    exfat_volume_label_dir_entry_t      volume_label;      // 32 bytes
    exfat_allocation_bitmap_dir_entry_t allocation_bitmap; // 32 bytes
    exfat_upcase_table_dir_entry_t      upcase_table;      // 32 bytes
    exfat_generic_dir_entry_t           end_marker;        // 32 bytes
} exfat_first_root_dir_entries_t;

STATIC_ASSERT_PACKED(sizeof(exfat_first_root_dir_entries_t) == 4 * 32, "must be 128 bytes");

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------
// Boot sector image exported for MSC interface.
// Defined in the C++ source as a constexpr instance.
// ---------------------------------------------------------------
extern const uint8_t exfat_boot_sector_data[];
extern const size_t exfat_boot_sector_data_length;

/**
 * Compile-time VBR checksum for the exFAT boot sector.
 * This is the checksum for sectors 0-10, as per Microsoft spec §3.1.2,
 * ignoring VolumeSerialNumber, which is added at runtime.
 * For the math involved, see the C++ source file.
 */
// Net rotate amount for suffix (modulo 32)
extern const int      EXFAT_VBR_SUFFIX_ROT;
extern const uint32_t EXFAT_VBR_CHECKSUM_PREFIX;
extern const uint32_t EXFAT_VBR_CHECKSUM_SUFFIX;

// ---------------------------------------------------------------
// First FAT sector partial data (initial cluster chains)
// ---------------------------------------------------------------
extern const uint8_t exfat_fat0_sector_data[]; ///< First FAT sector data
extern const size_t  exfat_fat0_sector_data_length; // (128 bytes)

// ---------------------------------------------------------------
// Minimal up-case table
// ---------------------------------------------------------------
// Up-case table for exFAT, used for case-insensitive file name matching.
extern const uint16_t exfat_upcase_table[]; ///< Up-case table data
extern const size_t   exfat_upcase_table_len; /// Length of the up-case table in bytes (30 entries × 2 bytes each = 60 bytes)
extern const uint32_t exfat_upcase_table_checksum; ///< Checksum of the up-case table


// First root directory entries, pre-constructed
extern const exfat_first_root_dir_entries_t exfat_root_dir_first_entries_data;

#ifdef __cplusplus
}
#endif
