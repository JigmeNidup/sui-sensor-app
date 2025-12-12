#include "bcs.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// Internal helper functions
// ============================================================================

static bcs_error_t ensure_capacity(bcs_writer_t *writer, size_t additional_bytes) {
    size_t required = writer->position + additional_bytes;

    if (required <= writer->capacity) {
        return BCS_OK;
    }

    // Check max size limit
    if (writer->max_size > 0 && required > writer->max_size) {
        return BCS_ERROR_BUFFER_TOO_SMALL;
    }

    // Calculate new capacity
    size_t new_capacity = writer->capacity;
    while (new_capacity < required) {
        new_capacity += writer->allocate_size;
    }

    if (writer->max_size > 0 && new_capacity > writer->max_size) {
        new_capacity = writer->max_size;
    }

    if (new_capacity < required) {
        return BCS_ERROR_BUFFER_TOO_SMALL;
    }

    // Reallocate buffer
    uint8_t *new_buffer = (uint8_t*)realloc(writer->buffer, new_capacity);
    if (!new_buffer) {
        return BCS_ERROR_OUT_OF_MEMORY;
    }

    writer->buffer = new_buffer;
    writer->capacity = new_capacity;

    return BCS_OK;
}

// ============================================================================
// Writer implementation
// ============================================================================

bcs_error_t bcs_writer_init(bcs_writer_t *writer, size_t initial_capacity, size_t max_size) {
    if (!writer || initial_capacity == 0) {
        return BCS_ERROR_INVALID_INPUT;
    }

    writer->buffer = (uint8_t*)malloc(initial_capacity);
    if (!writer->buffer) {
        return BCS_ERROR_OUT_OF_MEMORY;
    }

    writer->capacity = initial_capacity;
    writer->position = 0;
    writer->max_size = max_size;
    writer->allocate_size = initial_capacity; // Grow by initial size each time

    return BCS_OK;
}

void bcs_writer_free(bcs_writer_t *writer) {
    if (writer && writer->buffer) {
        free(writer->buffer);
        writer->buffer = NULL;
        writer->capacity = 0;
        writer->position = 0;
    }
}

const uint8_t *bcs_writer_get_bytes(const bcs_writer_t *writer, size_t *length) {
    if (!writer || !length) {
        return NULL;
    }

    *length = writer->position;
    return writer->buffer;
}

bcs_error_t bcs_write_u8(bcs_writer_t *writer, uint8_t value) {
    bcs_error_t err = ensure_capacity(writer, 1);
    if (err != BCS_OK) return err;

    writer->buffer[writer->position++] = value;
    return BCS_OK;
}

bcs_error_t bcs_write_u16(bcs_writer_t *writer, uint16_t value) {
    bcs_error_t err = ensure_capacity(writer, 2);
    if (err != BCS_OK) return err;

    // Little endian
    writer->buffer[writer->position++] = value & 0xFF;
    writer->buffer[writer->position++] = (value >> 8) & 0xFF;

    return BCS_OK;
}

bcs_error_t bcs_write_u32(bcs_writer_t *writer, uint32_t value) {
    bcs_error_t err = ensure_capacity(writer, 4);
    if (err != BCS_OK) return err;

    // Little endian
    writer->buffer[writer->position++] = value & 0xFF;
    writer->buffer[writer->position++] = (value >> 8) & 0xFF;
    writer->buffer[writer->position++] = (value >> 16) & 0xFF;
    writer->buffer[writer->position++] = (value >> 24) & 0xFF;

    return BCS_OK;
}

bcs_error_t bcs_write_u64(bcs_writer_t *writer, uint64_t value) {
    bcs_error_t err = ensure_capacity(writer, 8);
    if (err != BCS_OK) return err;

    // Little endian
    for (int i = 0; i < 8; i++) {
        writer->buffer[writer->position++] = (value >> (i * 8)) & 0xFF;
    }

    return BCS_OK;
}

bcs_error_t bcs_write_u128(bcs_writer_t *writer, uint64_t high, uint64_t low) {
    bcs_error_t err;

    // Write low 64 bits first (little endian)
    err = bcs_write_u64(writer, low);
    if (err != BCS_OK) return err;

    // Write high 64 bits
    err = bcs_write_u64(writer, high);
    if (err != BCS_OK) return err;

    return BCS_OK;
}

bcs_error_t bcs_write_u256(bcs_writer_t *writer, const uint8_t *bytes) {
    if (!bytes) {
        return BCS_ERROR_INVALID_INPUT;
    }

    return bcs_write_fixed_bytes(writer, bytes, 32);
}

bcs_error_t bcs_write_bool(bcs_writer_t *writer, bool value) {
    return bcs_write_u8(writer, value ? 1 : 0);
}

bcs_error_t bcs_write_uleb128(bcs_writer_t *writer, uint64_t value) {
    do {
        uint8_t byte = value & 0x7F;
        value >>= 7;

        if (value != 0) {
            byte |= 0x80;
        }

        bcs_error_t err = bcs_write_u8(writer, byte);
        if (err != BCS_OK) return err;

    } while (value != 0);

    return BCS_OK;
}

bcs_error_t bcs_write_bytes(bcs_writer_t *writer, const uint8_t *data, size_t length) {
    if (!data && length > 0) {
        return BCS_ERROR_INVALID_INPUT;
    }

    // Write length prefix
    bcs_error_t err = bcs_write_uleb128(writer, length);
    if (err != BCS_OK) return err;

    // Write data
    return bcs_write_fixed_bytes(writer, data, length);
}

bcs_error_t bcs_write_string(bcs_writer_t *writer, const char *str) {
    if (!str) {
        return BCS_ERROR_INVALID_INPUT;
    }

    size_t length = strlen(str);
    return bcs_write_bytes(writer, (const uint8_t *)str, length);
}

bcs_error_t bcs_write_fixed_bytes(bcs_writer_t *writer, const uint8_t *data, size_t length) {
    if (!data && length > 0) {
        return BCS_ERROR_INVALID_INPUT;
    }

    bcs_error_t err = ensure_capacity(writer, length);
    if (err != BCS_OK) return err;

    memcpy(writer->buffer + writer->position, data, length);
    writer->position += length;

    return BCS_OK;
}

bcs_error_t bcs_write_vec_length(bcs_writer_t *writer, size_t length) {
    return bcs_write_uleb128(writer, length);
}

bcs_error_t bcs_write_option_some(bcs_writer_t *writer) {
    return bcs_write_u8(writer, 1);
}

bcs_error_t bcs_write_option_none(bcs_writer_t *writer) {
    return bcs_write_u8(writer, 0);
}

// ============================================================================
// Reader implementation
// ============================================================================

void bcs_reader_init(bcs_reader_t *reader, const uint8_t *buffer, size_t length) {
    reader->buffer = buffer;
    reader->length = length;
    reader->position = 0;
}

size_t bcs_reader_remaining(const bcs_reader_t *reader) {
    if (reader->position >= reader->length) {
        return 0;
    }
    return reader->length - reader->position;
}

bcs_error_t bcs_read_u8(bcs_reader_t *reader, uint8_t *value) {
    if (!value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    if (reader->position >= reader->length) {
        return BCS_ERROR_BUFFER_UNDERFLOW;
    }

    *value = reader->buffer[reader->position++];
    return BCS_OK;
}

bcs_error_t bcs_read_u16(bcs_reader_t *reader, uint16_t *value) {
    if (!value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    if (reader->position + 2 > reader->length) {
        return BCS_ERROR_BUFFER_UNDERFLOW;
    }

    // Little endian
    *value = (uint16_t)reader->buffer[reader->position] |
             ((uint16_t)reader->buffer[reader->position + 1] << 8);

    reader->position += 2;
    return BCS_OK;
}

bcs_error_t bcs_read_u32(bcs_reader_t *reader, uint32_t *value) {
    if (!value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    if (reader->position + 4 > reader->length) {
        return BCS_ERROR_BUFFER_UNDERFLOW;
    }

    // Little endian
    *value = (uint32_t)reader->buffer[reader->position] |
             ((uint32_t)reader->buffer[reader->position + 1] << 8) |
             ((uint32_t)reader->buffer[reader->position + 2] << 16) |
             ((uint32_t)reader->buffer[reader->position + 3] << 24);

    reader->position += 4;
    return BCS_OK;
}

bcs_error_t bcs_read_u64(bcs_reader_t *reader, uint64_t *value) {
    if (!value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    if (reader->position + 8 > reader->length) {
        return BCS_ERROR_BUFFER_UNDERFLOW;
    }

    // Little endian
    *value = 0;
    for (int i = 0; i < 8; i++) {
        *value |= ((uint64_t)reader->buffer[reader->position + i]) << (i * 8);
    }

    reader->position += 8;
    return BCS_OK;
}

bcs_error_t bcs_read_u128(bcs_reader_t *reader, uint64_t *high, uint64_t *low) {
    if (!high || !low) {
        return BCS_ERROR_INVALID_INPUT;
    }

    bcs_error_t err;

    // Read low 64 bits first (little endian)
    err = bcs_read_u64(reader, low);
    if (err != BCS_OK) return err;

    // Read high 64 bits
    err = bcs_read_u64(reader, high);
    if (err != BCS_OK) return err;

    return BCS_OK;
}

bcs_error_t bcs_read_u256(bcs_reader_t *reader, uint8_t *bytes) {
    return bcs_read_bytes(reader, bytes, 32);
}

bcs_error_t bcs_read_bool(bcs_reader_t *reader, bool *value) {
    if (!value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    uint8_t byte;
    bcs_error_t err = bcs_read_u8(reader, &byte);
    if (err != BCS_OK) return err;

    if (byte > 1) {
        return BCS_ERROR_INVALID_INPUT;
    }

    *value = byte != 0;
    return BCS_OK;
}

bcs_error_t bcs_read_uleb128(bcs_reader_t *reader, uint64_t *value) {
    if (!value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    *value = 0;
    uint64_t shift = 0;

    while (true) {
        if (reader->position >= reader->length) {
            return BCS_ERROR_BUFFER_UNDERFLOW;
        }

        uint8_t byte = reader->buffer[reader->position++];
        *value |= ((uint64_t)(byte & 0x7F)) << shift;

        if ((byte & 0x80) == 0) {
            break;
        }

        shift += 7;
        if (shift >= 64) {
            return BCS_ERROR_OVERFLOW;
        }
    }

    return BCS_OK;
}

bcs_error_t bcs_read_bytes(bcs_reader_t *reader, uint8_t *buffer, size_t length) {
    if (!buffer && length > 0) {
        return BCS_ERROR_INVALID_INPUT;
    }

    if (reader->position + length > reader->length) {
        return BCS_ERROR_BUFFER_UNDERFLOW;
    }

    memcpy(buffer, reader->buffer + reader->position, length);
    reader->position += length;

    return BCS_OK;
}

bcs_error_t bcs_read_string(bcs_reader_t *reader, char *buffer, size_t max_length, size_t *actual_length) {
    if (!buffer || !actual_length) {
        return BCS_ERROR_INVALID_INPUT;
    }

    // Read length
    uint64_t length;
    bcs_error_t err = bcs_read_uleb128(reader, &length);
    if (err != BCS_OK) return err;

    if (length >= max_length) {
        return BCS_ERROR_BUFFER_TOO_SMALL;
    }

    // Read string data
    err = bcs_read_bytes(reader, (uint8_t *)buffer, (size_t)length);
    if (err != BCS_OK) return err;

    buffer[length] = '\0';
    *actual_length = (size_t)length;

    return BCS_OK;
}

bcs_error_t bcs_read_vec_length(bcs_reader_t *reader, size_t *length) {
    if (!length) {
        return BCS_ERROR_INVALID_INPUT;
    }

    uint64_t len;
    bcs_error_t err = bcs_read_uleb128(reader, &len);
    if (err != BCS_OK) return err;

    *length = (size_t)len;
    return BCS_OK;
}

bcs_error_t bcs_read_option_tag(bcs_reader_t *reader, bool *has_value) {
    if (!has_value) {
        return BCS_ERROR_INVALID_INPUT;
    }

    uint8_t tag;
    bcs_error_t err = bcs_read_u8(reader, &tag);
    if (err != BCS_OK) return err;

    if (tag > 1) {
        return BCS_ERROR_INVALID_INPUT;
    }

    *has_value = tag != 0;
    return BCS_OK;
}

// ============================================================================
// Utility functions
// ============================================================================

bcs_error_t bcs_hex_to_bytes(const char *hex, uint8_t *bytes, size_t max_bytes, size_t *actual_bytes) {
    if (!hex || !bytes || !actual_bytes) {
        return BCS_ERROR_INVALID_INPUT;
    }

    // Skip 0x prefix if present
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }

    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) {
        return BCS_ERROR_INVALID_INPUT;
    }

    size_t byte_len = hex_len / 2;
    if (byte_len > max_bytes) {
        return BCS_ERROR_BUFFER_TOO_SMALL;
    }

    for (size_t i = 0; i < byte_len; i++) {
        char high = hex[i * 2];
        char low = hex[i * 2 + 1];

        if (!isxdigit(high) || !isxdigit(low)) {
            return BCS_ERROR_INVALID_INPUT;
        }

        uint8_t h = (high <= '9') ? (high - '0') : (tolower(high) - 'a' + 10);
        uint8_t l = (low <= '9') ? (low - '0') : (tolower(low) - 'a' + 10);

        bytes[i] = (h << 4) | l;
    }

    *actual_bytes = byte_len;
    return BCS_OK;
}

void bcs_bytes_to_hex(const uint8_t *bytes, size_t length, char *hex) {
    const char hex_chars[] = "0123456789abcdef";

    for (size_t i = 0; i < length; i++) {
        hex[i * 2] = hex_chars[(bytes[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[bytes[i] & 0x0F];
    }

    hex[length * 2] = '\0';
}