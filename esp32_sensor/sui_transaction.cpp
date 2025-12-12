/**
 * Sui Transaction Helpers - Implementation
 * Updated to match the server's execute-sponsored transaction structure
 */

#include "sui_transaction.h"
#include <stdlib.h>
#include <string.h>

bcs_error_t sui_build_sensor_transaction(
  const transaction_builder_t *params,
  char **output_hex,
  size_t *output_length) {
  if (!params || !output_hex || !output_length) {
    return BCS_ERROR_INVALID_INPUT;
  }

  bcs_writer_t writer;
  bcs_writer_init(&writer, 512, 0);
  bcs_error_t err = BCS_OK;

  // ========== TransactionData V1 ==========
  bcs_write_u8(&writer, 0x00);  // Version: V1

  // ========== TransactionKind: ProgrammableTransaction ==========
  bcs_write_u8(&writer, 0x00);  // Kind: ProgrammableTransaction

  // ========== Inputs (8 total: 1 Clock Object + 7 Pure values) ==========
  bcs_write_uleb128(&writer, 8);



  // Input 1: Pure - temperature (u64)
  bcs_write_u8(&writer, 0x00);
  bcs_write_uleb128(&writer, 8);
  bcs_write_u64(&writer, params->sensor_data.value1);

  // Input 2: Pure - humidity (u64)
  bcs_write_u8(&writer, 0x00);
  bcs_write_uleb128(&writer, 8);
  bcs_write_u64(&writer, params->sensor_data.value2);

  // Input 3: Pure - ec (u64)
  bcs_write_u8(&writer, 0x00);
  bcs_write_uleb128(&writer, 8);
  bcs_write_u64(&writer, params->sensor_data.value3);

  // Input 4: Pure - ph (u64)
  bcs_write_u8(&writer, 0x00);
  bcs_write_uleb128(&writer, 8);
  bcs_write_u64(&writer, params->sensor_data.value4);

  // Input 5: Pure - device_id (string)
  bcs_write_u8(&writer, 0x00);
  bcs_write_u8(&writer, 0x0d);
  bcs_write_uleb128(&writer, 12);  // "esp32-device" = 12 chars
  bcs_write_fixed_bytes(&writer, (const uint8_t *)"esp32-device", 12);

  // Input 6: Pure - sensor_type (string)
  bcs_write_u8(&writer, 0x00);
  bcs_write_u8(&writer, 0x05);
  bcs_write_uleb128(&writer, 4);  // "soil" = 4 chars
  bcs_write_fixed_bytes(&writer, (const uint8_t *)"soil", 4);

  // Input 7: Pure - location (string) - LAST!
  bcs_write_u8(&writer, 0x00);
  bcs_write_u8(&writer, 0x01);  // CallArg::Object
  bcs_write_uleb128(&writer, 0);  // Empty string

  // Input 0: Clock Object - Shared object (FIRST!)
  uint8_t clock_object_id[32] = { 0 };
  clock_object_id[31] = 0x06;   // Clock ID 0x6
  bcs_write_u8(&writer, 0x01);  // CallArg::Object
  bcs_write_u8(&writer, 0x01);  // ObjectArg::SharedObject (variant 1)
  bcs_write_fixed_bytes(&writer, clock_object_id, 32);
  bcs_write_u64(&writer, 1);    // Initial shared version = 1
  bcs_write_u8(&writer, 0x00);  // mutable = false

  // ========== Commands (1 MoveCall) ==========
  bcs_write_uleb128(&writer, 1);

  // Command: MoveCall
  bcs_write_u8(&writer, 0x00);

  // Package ID
  bcs_write_fixed_bytes(&writer, params->package_id, 32);

  // Module name
  bcs_write_string(&writer, params->module_name);

  // Function name
  bcs_write_string(&writer, params->function_name);

  // Type arguments (empty)
  bcs_write_uleb128(&writer, 0);

  // Arguments (8 inputs: indices 0-7)
  bcs_write_uleb128(&writer, 8);
  // Simple sequential indices
  for (int i = 0; i < 8; i++) {
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, i);    // Input index
  }

  // ========== Sender ==========
  bcs_write_fixed_bytes(&writer, params->sender, 32);

  // ========== Gas Data ==========
  bcs_write_uleb128(&writer, 1);  // 1 gas coin
  bcs_write_fixed_bytes(&writer, params->gas_object.object_id, 32);
  bcs_write_u64(&writer, params->gas_object.version);

  bcs_write_u8(&writer, 0x20);  // Argument::Input
  bcs_write_fixed_bytes(&writer, params->gas_object.digest, 32);

  bcs_write_fixed_bytes(&writer, params->sender, 32);  // Gas owner
  bcs_write_u64(&writer, params->gas_price);
  bcs_write_u64(&writer, params->gas_budget);

  // ========== Expiration ==========
  bcs_write_u8(&writer, 0x00);  // None expiration

  // ========== Get result ==========
  size_t result_length;
  const uint8_t *result_bytes = bcs_writer_get_bytes(&writer, &result_length);

  *output_hex = (char *)malloc(result_length * 2 + 1);
  if (!*output_hex) {
    err = BCS_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  bcs_bytes_to_hex(result_bytes, result_length, *output_hex);
  *output_length = result_length * 2;

cleanup:
  bcs_writer_free(&writer);
  return err;
}

// Alternative simpler transaction builder matching the server's structure more closely
bcs_error_t sui_build_simple_sensor_transaction(
  const transaction_builder_t *params,
  char **output_hex,
  size_t *output_length) {

  if (!params || !output_hex || !output_length) {
    return BCS_ERROR_INVALID_INPUT;
  }

  bcs_writer_t writer;
  bcs_writer_init(&writer, 512, 0);
  bcs_error_t err = BCS_OK;

  // Build a simplified transaction matching what the server expects
  // This is based on the server's Transaction structure:
  // - Set sender
  // - Set gas payment
  // - Set gas budget
  // - Make move call with arguments

  // For now, we'll build a minimal transaction
  // In production, you'd need to match the exact BCS structure

  // Write sender
  bcs_write_fixed_bytes(&writer, params->sender, 32);

  // Write gas payment count (1)
  bcs_write_uleb128(&writer, 1);

  // Write gas object
  bcs_write_fixed_bytes(&writer, params->gas_object.object_id, 32);
  bcs_write_u64(&writer, params->gas_object.version);
  bcs_write_fixed_bytes(&writer, params->gas_object.digest, 32);

  // Write gas budget
  bcs_write_u64(&writer, params->gas_budget);

  // Write gas price
  bcs_write_u64(&writer, params->gas_price);

  // Write sequence number (0 for new transaction)
  bcs_write_u64(&writer, 0);

  // Write expiration (0 for no expiration)
  bcs_write_u8(&writer, 0x00);  // None expiration

  // Write transaction kind (ProgrammableTransaction)
  bcs_write_u8(&writer, 0x00);

  // Write inputs count
  bcs_write_uleb128(&writer, 8);  // 8 inputs as per server

  // Write inputs... (this would need to match exactly what the server builds)
  // For now, we'll return a simplified version

  size_t result_length;
  const uint8_t *result_bytes = bcs_writer_get_bytes(&writer, &result_length);

  // Convert to hex
  *output_hex = (char *)malloc(result_length * 2 + 1);
  if (!*output_hex) {
    err = BCS_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  bcs_bytes_to_hex(result_bytes, result_length, *output_hex);
  *output_length = result_length * 2;

cleanup:
  bcs_writer_free(&writer);
  return err;
}

bcs_error_t sui_modify_transaction_with_pure_values(
  const char *hex_tx,
  const uint8_t **pure_values,
  const size_t *pure_lengths,
  size_t num_pures,
  char **output_hex,
  size_t *output_length) {
  if (!hex_tx || !pure_values || !pure_lengths || !output_hex || !output_length) {
    return BCS_ERROR_INVALID_INPUT;
  }

  // Convert hex to bytes
  size_t max_length = strlen(hex_tx) / 2;
  uint8_t *tx_bytes = (uint8_t *)malloc(max_length);
  if (!tx_bytes) {
    return BCS_ERROR_OUT_OF_MEMORY;
  }

  size_t tx_length;
  bcs_error_t err = bcs_hex_to_bytes(hex_tx, tx_bytes, max_length, &tx_length);
  if (err != BCS_OK) {
    free(tx_bytes);
    return err;
  }

  // Parse transaction
  bcs_reader_t reader;
  bcs_reader_init(&reader, tx_bytes, tx_length);

  // Read TransactionData version byte
  uint8_t version;
  err = bcs_read_u8(&reader, &version);
  if (err != BCS_OK) {
    free(tx_bytes);
    return err;
  }

  // Read TransactionKind type
  uint8_t kind;
  err = bcs_read_u8(&reader, &kind);
  if (err != BCS_OK) {
    free(tx_bytes);
    return err;
  }

  uint64_t num_inputs;
  err = bcs_read_uleb128(&reader, &num_inputs);
  if (err != BCS_OK) {
    free(tx_bytes);
    return err;
  }

  // Rebuild transaction
  bcs_writer_t writer;
  bcs_writer_init(&writer, 512, 0);

  // Write TransactionData version byte
  bcs_write_u8(&writer, version);

  // Write TransactionKind type
  bcs_write_u8(&writer, kind);
  bcs_write_uleb128(&writer, num_inputs);

  // Process inputs
  size_t pure_idx = 0;
  bool success = true;

  for (uint64_t i = 0; i < num_inputs && success; i++) {
    uint8_t input_type;
    err = bcs_read_u8(&reader, &input_type);
    if (err != BCS_OK) {
      success = false;
      break;
    }
    bcs_write_u8(&writer, input_type);

    if (input_type == 0) {  // Pure - replace with new value
      // Read old value (discard)
      uint64_t old_len;
      err = bcs_read_uleb128(&reader, &old_len);
      if (err != BCS_OK) {
        success = false;
        break;
      }

      uint8_t *old_data = (uint8_t *)malloc((size_t)old_len);
      if (!old_data) {
        err = BCS_ERROR_OUT_OF_MEMORY;
        success = false;
        break;
      }
      err = bcs_read_bytes(&reader, old_data, (size_t)old_len);
      free(old_data);
      if (err != BCS_OK) {
        success = false;
        break;
      }

      // Write new value
      if (pure_idx < num_pures) {
        bcs_write_uleb128(&writer, pure_lengths[pure_idx]);
        bcs_write_fixed_bytes(&writer, pure_values[pure_idx], pure_lengths[pure_idx]);
      } else {
        // Not enough pure values provided - keep old value
        bcs_write_uleb128(&writer, old_len);
        bcs_write_fixed_bytes(&writer, old_data, (size_t)old_len);
      }
      pure_idx++;
    } else if (input_type == 1) {  // Object - copy unchanged
      uint8_t variant;
      err = bcs_read_u8(&reader, &variant);
      if (err != BCS_OK) {
        success = false;
        break;
      }
      bcs_write_u8(&writer, variant);

      uint8_t obj_id[32];
      err = bcs_read_bytes(&reader, obj_id, 32);
      if (err != BCS_OK) {
        success = false;
        break;
      }
      bcs_write_fixed_bytes(&writer, obj_id, 32);

      if (variant == 0) {  // ImmOrOwnedObject
        uint64_t obj_version;
        err = bcs_read_u64(&reader, &obj_version);
        if (err != BCS_OK) {
          success = false;
          break;
        }
        bcs_write_u64(&writer, obj_version);

        uint8_t digest[32];
        err = bcs_read_bytes(&reader, digest, 32);
        if (err != BCS_OK) {
          success = false;
          break;
        }
        bcs_write_fixed_bytes(&writer, digest, 32);
      } else if (variant == 1) {  // SharedObject
        uint64_t initial_shared_version;
        err = bcs_read_u64(&reader, &initial_shared_version);
        if (err != BCS_OK) {
          success = false;
          break;
        }
        bcs_write_u64(&writer, initial_shared_version);

        uint8_t is_mutable;
        err = bcs_read_u8(&reader, &is_mutable);
        if (err != BCS_OK) {
          success = false;
          break;
        }
        bcs_write_u8(&writer, is_mutable);
      } else if (variant == 2) {  // Receiving
        uint64_t obj_version;
        err = bcs_read_u64(&reader, &obj_version);
        if (err != BCS_OK) {
          success = false;
          break;
        }
        bcs_write_u64(&writer, obj_version);

        uint8_t digest[32];
        err = bcs_read_bytes(&reader, digest, 32);
        if (err != BCS_OK) {
          success = false;
          break;
        }
        bcs_write_fixed_bytes(&writer, digest, 32);
      }
    }
  }

  if (success) {
    // Copy the rest of the transaction (commands, sender, gas payment, gas budget/price)
    size_t remaining = bcs_reader_remaining(&reader);
    if (remaining > 0) {
      uint8_t *rest = (uint8_t *)malloc(remaining);
      if (!rest) {
        err = BCS_ERROR_OUT_OF_MEMORY;
        success = false;
      } else {
        err = bcs_read_bytes(&reader, rest, remaining);
        if (err != BCS_OK) {
          free(rest);
          success = false;
        } else {
          bcs_write_fixed_bytes(&writer, rest, remaining);
          free(rest);
        }
      }
    }
  }

  if (success) {
    // Get result
    size_t result_length;
    const uint8_t *result_bytes = bcs_writer_get_bytes(&writer, &result_length);

    // Convert to hex
    *output_hex = (char *)malloc(result_length * 2 + 1);
    if (!*output_hex) {
      err = BCS_ERROR_OUT_OF_MEMORY;
      success = false;
    } else {
      bcs_bytes_to_hex(result_bytes, result_length, *output_hex);
      *output_length = result_length * 2;
    }
  }

  // Cleanup
  bcs_writer_free(&writer);
  free(tx_bytes);

  if (!success) {
    return err;
  }

  return BCS_OK;
}

bcs_error_t sui_modify_transaction_with_sensor_data(
  const char *hex_tx,
  const sensor_data_t *sensor_data,
  char **output_hex,
  size_t *output_length) {
  if (!sensor_data) {
    return BCS_ERROR_INVALID_INPUT;
  }

  // Serialize sensor values to bytes (as u64 for server compatibility)
  uint8_t temperature_bytes[8];
  uint64_t temp_u64 = sensor_data->value1;
  for (int i = 0; i < 8; i++) {
    temperature_bytes[i] = (temp_u64 >> (i * 8)) & 0xFF;
  }

  uint8_t humidity_bytes[8];
  uint64_t hum_u64 = sensor_data->value2;
  for (int i = 0; i < 8; i++) {
    humidity_bytes[i] = (hum_u64 >> (i * 8)) & 0xFF;
  }

  uint8_t ec_bytes[8];
  uint64_t ec_u64 = sensor_data->value4;
  for (int i = 0; i < 8; i++) {
    ec_bytes[i] = (ec_u64 >> (i * 8)) & 0xFF;
  }

  uint8_t ph_bytes[8];
  uint64_t ph_u64 = sensor_data->value3;
  for (int i = 0; i < 8; i++) {
    ph_bytes[i] = (ph_u64 >> (i * 8)) & 0xFF;
  }

  uint8_t timestamp_bytes[8];
  for (int i = 0; i < 8; i++) {
    timestamp_bytes[i] = (sensor_data->timestamp >> (i * 8)) & 0xFF;
  }

  // Create arrays for the low-level function
  const uint8_t *pure_values[] = {
    temperature_bytes,
    humidity_bytes,
    ec_bytes,
    ph_bytes,
    timestamp_bytes
  };

  const size_t pure_lengths[] = { 8, 8, 8, 8, 8 };

  return sui_modify_transaction_with_pure_values(
    hex_tx,
    pure_values,
    pure_lengths,
    5,
    output_hex,
    output_length);
}