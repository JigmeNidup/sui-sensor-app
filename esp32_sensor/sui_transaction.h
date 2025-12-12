/**
 * Sui Transaction Helpers
 * High-level functions for modifying Sui transactions with sensor data
 */

 #ifndef SUI_TRANSACTION_H
 #define SUI_TRANSACTION_H
 
 #include "bcs.h"
 #include <stdint.h>
 #include <stddef.h>
 
 /**
  * Sensor data structure
  * Adjust fields to match your Move contract
  */
 typedef struct {
     uint16_t value1;      // e.g., temperature * 100
     uint16_t value2;      // e.g., humidity * 100
     uint16_t value3;      // e.g., pressure * 10
     uint16_t value4;      // e.g., voltage * 100
     uint64_t timestamp;   // Unix timestamp
 } sensor_data_t;
 
 /**
  * Gas object data structure
  */
 typedef struct {
     uint8_t object_id[32];    // Gas coin object ID
     uint64_t version;         // Object version
     uint8_t digest[32];       // Object digest
 } gas_object_t;
 
 /**
  * Transaction builder parameters
  */
 typedef struct {
     // Package and module
     uint8_t package_id[32];       // Move package ID
     const char *module_name;      // Module name (e.g., "sensor")
     const char *function_name;    // Function name (e.g., "record_reading")
 
    // Sensor object (Owned or Shared)
    uint8_t sensor_object_id[32]; // Sensor object ID
    uint64_t sensor_initial_shared_version;  // Initial shared version (or version for owned)
    uint8_t sensor_digest[32];   // Sensor object digest (for owned objects)
    bool sensor_mutable;          // Mutable flag
    bool only_transaction_kind;   // Build only transaction kind (no gas/sender)
 
     // Sensor data
     sensor_data_t sensor_data;    // Sensor readings
 
     // Transaction metadata
     uint8_t sender[32];           // Sender address
     gas_object_t gas_object;      // Gas payment object
     uint64_t gas_budget;          // Gas budget (e.g., 100000000)
     uint64_t gas_price;           // Gas price (e.g., 1000)
 } transaction_builder_t;
 
 /**
  * Build a complete Sui transaction from scratch
  *
  * Creates a full TransactionData structure with sensor data, ready for signing.
  * This builds the entire transaction in C without needing TypeScript template.
  *
  * @param params         Transaction builder parameters (sensor data, IDs, gas, etc.)
  * @param output_hex     Output: transaction hex (caller must free)
  * @param output_length  Output: length of transaction
  * @return BCS_OK on success, error code otherwise
  *
  * Example:
  *   transaction_builder_t params = {
  *       .package_id = {...},
  *       .module_name = "sensor",
  *       .function_name = "record_reading",
  *       .sensor_object_id = {...},
  *       .sensor_initial_shared_version = 5882390,
  *       .sensor_mutable = true,
  *       .sensor_data = { 2350, 6540, 10132, 850, 1730822400 },
  *       .sender = {...},
  *       .gas_object = {...},
  *       .gas_budget = 100000000,
  *       .gas_price = 1000
  *   };
  *
  *   char *tx_hex;
  *   size_t tx_len;
  *
  *   bcs_error_t err = sui_build_sensor_transaction(&params, &tx_hex, &tx_len);
  *
  *   if (err == BCS_OK) {
  *       // Sign and submit
  *       free(tx_hex);
  *   }
  */
 bcs_error_t sui_build_sensor_transaction(
     const transaction_builder_t *params,
     char **output_hex,
     size_t *output_length
 );
 
 /**
  * Modify a Sui transaction with sensor data
  *
  * Takes a FULL transaction (TransactionData) from TypeScript and replaces
  * Pure input values with actual sensor readings. Preserves Object references,
  * commands, gas payment, sender, and all other metadata.
  *
  * Works with complete TransactionData structure including:
  * - TransactionKind (inputs + commands)
  * - Sender address
  * - Gas payment objects
  * - Gas budget and price
  *
  * @param hex_tx         Input full transaction in hex format (from TypeScript)
  * @param sensor_data    Sensor readings to inject
  * @param output_hex     Output: modified transaction hex (caller must free)
  * @param output_length  Output: length of modified transaction
  * @return BCS_OK on success, error code otherwise
  *
  * Example:
  *   sensor_data_t data = { 2350, 6540, 10132, 850, 1730822400 };
  *   char *modified_tx;
  *   size_t modified_len;
  *
  *   bcs_error_t err = sui_modify_transaction_with_sensor_data(
  *       template_hex, &data, &modified_tx, &modified_len);
  *
  *   if (err == BCS_OK) {
  *       send_to_backend(modified_tx);  // Ready to sign with Transaction.from()
  *       free(modified_tx);
  *   }
  */
 bcs_error_t sui_modify_transaction_with_sensor_data(
     const char *hex_tx,
     const sensor_data_t *sensor_data,
     char **output_hex,
     size_t *output_length
 );
 
 /**
  * Lower-level function: Modify transaction with custom Pure values
  *
  * For more control, you can provide an array of Pure values to inject.
  *
  * @param hex_tx         Input transaction hex
  * @param pure_values    Array of byte arrays for Pure values
  * @param pure_lengths   Array of lengths for each Pure value
  * @param num_pures      Number of Pure values
  * @param output_hex     Output: modified transaction hex (caller must free)
  * @param output_length  Output: length of modified transaction
  * @return BCS_OK on success, error code otherwise
  */
 bcs_error_t sui_modify_transaction_with_pure_values(
     const char *hex_tx,
     const uint8_t **pure_values,
     const size_t *pure_lengths,
     size_t num_pures,
     char **output_hex,
     size_t *output_length
 );
 
 #endif // SUI_TRANSACTION_H
 