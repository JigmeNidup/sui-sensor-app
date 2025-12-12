#ifndef BCS_H
#define BCS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// BCS Writer for serialization
typedef struct {
    uint8_t *buffer;
    size_t capacity;
    size_t position;
    size_t max_size;
    size_t allocate_size;
} bcs_writer_t;

// BCS Reader for deserialization
typedef struct {
    const uint8_t *buffer;
    size_t length;
    size_t position;
} bcs_reader_t;

// Error codes
typedef enum {
    BCS_OK = 0,
    BCS_ERROR_OUT_OF_MEMORY = -1,
    BCS_ERROR_BUFFER_TOO_SMALL = -2,
    BCS_ERROR_INVALID_INPUT = -3,
    BCS_ERROR_OVERFLOW = -4,
    BCS_ERROR_BUFFER_UNDERFLOW = -5,
} bcs_error_t;

// ============================================================================
// Writer API - for serializing data
// ============================================================================

/**
 * Initialize a BCS writer with given capacity
 * @param writer Pointer to writer structure
 * @param initial_capacity Initial buffer capacity in bytes
 * @param max_size Maximum allowed buffer size (0 for unlimited)
 * @return BCS_OK on success, error code otherwise
 */
bcs_error_t bcs_writer_init(bcs_writer_t *writer, size_t initial_capacity, size_t max_size);

/**
 * Free resources allocated by the writer
 * @param writer Pointer to writer structure
 */
void bcs_writer_free(bcs_writer_t *writer);

/**
 * Get the serialized bytes from the writer
 * @param writer Pointer to writer structure
 * @param length Output parameter for the length of serialized data
 * @return Pointer to serialized data (valid until writer is freed)
 */
const uint8_t *bcs_writer_get_bytes(const bcs_writer_t *writer, size_t *length);

/**
 * Write a single byte (u8)
 */
bcs_error_t bcs_write_u8(bcs_writer_t *writer, uint8_t value);

/**
 * Write a 16-bit unsigned integer (u16) - little endian
 */
bcs_error_t bcs_write_u16(bcs_writer_t *writer, uint16_t value);

/**
 * Write a 32-bit unsigned integer (u32) - little endian
 */
bcs_error_t bcs_write_u32(bcs_writer_t *writer, uint32_t value);

/**
 * Write a 64-bit unsigned integer (u64) - little endian
 */
bcs_error_t bcs_write_u64(bcs_writer_t *writer, uint64_t value);

/**
 * Write a 128-bit unsigned integer (u128) - little endian
 * @param high High 64 bits
 * @param low Low 64 bits
 */
bcs_error_t bcs_write_u128(bcs_writer_t *writer, uint64_t high, uint64_t low);

/**
 * Write a 256-bit unsigned integer (u256) - little endian
 * @param bytes Pointer to 32 bytes in little endian format
 */
bcs_error_t bcs_write_u256(bcs_writer_t *writer, const uint8_t *bytes);

/**
 * Write a boolean value
 */
bcs_error_t bcs_write_bool(bcs_writer_t *writer, bool value);

/**
 * Write an ULEB128 encoded unsigned integer
 */
bcs_error_t bcs_write_uleb128(bcs_writer_t *writer, uint64_t value);

/**
 * Write raw bytes
 */
bcs_error_t bcs_write_bytes(bcs_writer_t *writer, const uint8_t *data, size_t length);

/**
 * Write a UTF-8 string (length-prefixed with ULEB128)
 */
bcs_error_t bcs_write_string(bcs_writer_t *writer, const char *str);

/**
 * Write fixed-length bytes (no length prefix)
 */
bcs_error_t bcs_write_fixed_bytes(bcs_writer_t *writer, const uint8_t *data, size_t length);

/**
 * Begin writing a vector (writes the length as ULEB128)
 * After calling this, write each element individually
 */
bcs_error_t bcs_write_vec_length(bcs_writer_t *writer, size_t length);

/**
 * Write an Option<T> - some value
 * First writes 1 (some), caller should then write the value
 */
bcs_error_t bcs_write_option_some(bcs_writer_t *writer);

/**
 * Write an Option<T> - none value
 * Writes 0 to indicate none
 */
bcs_error_t bcs_write_option_none(bcs_writer_t *writer);

// ============================================================================
// Reader API - for deserializing data
// ============================================================================

/**
 * Initialize a BCS reader with a buffer
 * @param reader Pointer to reader structure
 * @param buffer Pointer to buffer containing BCS data
 * @param length Length of the buffer
 */
void bcs_reader_init(bcs_reader_t *reader, const uint8_t *buffer, size_t length);

/**
 * Get remaining bytes in the reader
 */
size_t bcs_reader_remaining(const bcs_reader_t *reader);

/**
 * Read a single byte (u8)
 */
bcs_error_t bcs_read_u8(bcs_reader_t *reader, uint8_t *value);

/**
 * Read a 16-bit unsigned integer (u16) - little endian
 */
bcs_error_t bcs_read_u16(bcs_reader_t *reader, uint16_t *value);

/**
 * Read a 32-bit unsigned integer (u32) - little endian
 */
bcs_error_t bcs_read_u32(bcs_reader_t *reader, uint32_t *value);

/**
 * Read a 64-bit unsigned integer (u64) - little endian
 */
bcs_error_t bcs_read_u64(bcs_reader_t *reader, uint64_t *value);

/**
 * Read a 128-bit unsigned integer (u128) - little endian
 */
bcs_error_t bcs_read_u128(bcs_reader_t *reader, uint64_t *high, uint64_t *low);

/**
 * Read a 256-bit unsigned integer (u256) - little endian
 * @param bytes Output buffer for 32 bytes
 */
bcs_error_t bcs_read_u256(bcs_reader_t *reader, uint8_t *bytes);

/**
 * Read a boolean value
 */
bcs_error_t bcs_read_bool(bcs_reader_t *reader, bool *value);

/**
 * Read an ULEB128 encoded unsigned integer
 */
bcs_error_t bcs_read_uleb128(bcs_reader_t *reader, uint64_t *value);

/**
 * Read raw bytes into a buffer
 * @param reader Pointer to reader
 * @param buffer Output buffer
 * @param length Number of bytes to read
 */
bcs_error_t bcs_read_bytes(bcs_reader_t *reader, uint8_t *buffer, size_t length);

/**
 * Read a UTF-8 string (length-prefixed with ULEB128)
 * @param reader Pointer to reader
 * @param buffer Output buffer for string (must be large enough)
 * @param max_length Maximum length of output buffer
 * @param actual_length Output parameter for actual string length
 */
bcs_error_t bcs_read_string(bcs_reader_t *reader, char *buffer, size_t max_length, size_t *actual_length);

/**
 * Read a vector length (ULEB128)
 */
bcs_error_t bcs_read_vec_length(bcs_reader_t *reader, size_t *length);

/**
 * Read an Option<T> tag
 * @param has_value Output: true if Some, false if None
 */
bcs_error_t bcs_read_option_tag(bcs_reader_t *reader, bool *has_value);

// ============================================================================
// Utility functions
// ============================================================================

/**
 * Convert hex string to bytes
 * @param hex Hex string (with or without 0x prefix)
 * @param bytes Output buffer
 * @param max_bytes Maximum size of output buffer
 * @param actual_bytes Output parameter for actual bytes written
 * @return BCS_OK on success
 */
bcs_error_t bcs_hex_to_bytes(const char *hex, uint8_t *bytes, size_t max_bytes, size_t *actual_bytes);

/**
 * Convert bytes to hex string
 * @param bytes Input bytes
 * @param length Length of input
 * @param hex Output buffer (must be at least length*2 + 1 bytes)
 */
void bcs_bytes_to_hex(const uint8_t *bytes, size_t length, char *hex);

#endif // BCS_H