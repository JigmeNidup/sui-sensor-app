/// Module: sensor_storage::sensor_storage
/// Contract to store IoT sensor data on Sui blockchain
module sensor_storage::sensor_storage {
    // Move 2024 imports common modules by default
    use sui::event;
    use sui::clock::{Self, Clock};
    use std::string;

    // ========================
    // 1. CUSTOM TYPES
    // ========================

    /// Main object storing sensor data
    public struct SensorData has key, store {
        id: UID,
        temperature: u64,        // Temperature in Celsius * 100 (e.g., 2500 = 25.00°C)
        humidity: u64,           // Humidity in percentage * 100 (e.g., 6500 = 65.00%)
        ec: u64,                 // Electrical Conductivity in µS/cm
        ph: u64,                 // pH level * 100 (e.g., 700 = 7.00)
        timestamp: u64,          // When data was recorded
        device_id: string::String, // Device identifier
        location: string::String, // Location (empty string if not provided)
        sensor_type: string::String, // Type of sensor
    }

    /// Event emitted when new sensor data is stored
    public struct SensorDataStoredEvent has copy, drop {
        object_id: address,
        device_id: string::String,
        temperature: u64,
        humidity: u64,
        ec: u64,
        ph: u64,
        timestamp: u64,
        sensor_type: string::String,
    }

    // ========================
    // 2. ERROR CODES
    // ========================

    /// Error codes for validation
    const E_INVALID_TEMPERATURE: u64 = 1;
    const E_INVALID_HUMIDITY: u64 = 2;
    const E_INVALID_EC: u64 = 3;
    const E_INVALID_PH: u64 = 4;

    // ========================
    // 3. VALIDATION FUNCTIONS
    // ========================

    /// Validate temperature (-50°C to 100°C range)
    fun validate_temperature(temp: u64): bool {
        temp <= 10000  // 0-100°C range in hundredths
    }

    /// Validate humidity (0-100% range)
    fun validate_humidity(hum: u64): bool {
        hum <= 10000  // 0-100% in hundredths
    }

    /// Validate EC (0-5000 µS/cm typical for soil)
    fun validate_ec(ec_val: u64): bool {
        ec_val <= 50000  // 0-5000 µS/cm in tens
    }

    /// Validate pH (0-14 scale)
    fun validate_ph(ph_val: u64): bool {
        ph_val <= 1400  // 0-14.00 in hundredths
    }

    // ========================
    // 4. CORE FUNCTIONS
    // ========================

    /// Main entry function to store sensor data
    entry fun store_sensor_data(
        temperature: u64,
        humidity: u64,
        ec: u64,
        ph: u64,
        device_id: vector<u8>,
        sensor_type: vector<u8>,
        location: vector<u8>,
        clock: &Clock,
        ctx: &mut TxContext
    ) {
        // Validate input data
        assert!(validate_temperature(temperature), E_INVALID_TEMPERATURE);
        assert!(validate_humidity(humidity), E_INVALID_HUMIDITY);
        assert!(validate_ec(ec), E_INVALID_EC);
        assert!(validate_ph(ph), E_INVALID_PH);

        let device_id_str = string::utf8(device_id);
        let sensor_type_str = string::utf8(sensor_type);
        let location_str = string::utf8(location);
        let timestamp = clock::timestamp_ms(clock);

        // Create SensorData object
        let sensor_data = SensorData {
            id: object::new(ctx),
            temperature,
            humidity,
            ec,
            ph,
            timestamp,
            device_id: device_id_str,
            location: location_str,
            sensor_type: sensor_type_str,
        };

        // Emit event
        let object_id = object::uid_to_address(&sensor_data.id);
        event::emit(SensorDataStoredEvent {
            object_id,
            device_id: device_id_str,
            temperature,
            humidity,
            ec,
            ph,
            timestamp,
            sensor_type: sensor_type_str,
        });

        // Transfer to transaction sender
        transfer::public_transfer(sensor_data, tx_context::sender(ctx));
    }

    // ========================
    // 5. VIEW/HELPER FUNCTIONS
    // ========================

    /// Get temperature in Celsius (converted from stored format)
    public fun get_temperature(sensor_data: &SensorData): u64 {
        sensor_data.temperature
    }

    /// Get humidity in percentage (converted from stored format)
    public fun get_humidity(sensor_data: &SensorData): u64 {
        sensor_data.humidity
    }

    /// Get electrical conductivity
    public fun get_ec(sensor_data: &SensorData): u64 {
        sensor_data.ec
    }

    /// Get pH level
    public fun get_ph(sensor_data: &SensorData): u64 {
        sensor_data.ph
    }

    /// Get timestamp
    public fun get_timestamp(sensor_data: &SensorData): u64 {
        sensor_data.timestamp
    }

    /// Get device ID
    public fun get_device_id(sensor_data: &SensorData): string::String {
        sensor_data.device_id
    }

    /// Get location
    public fun get_location(sensor_data: &SensorData): string::String {
        sensor_data.location
    }

    /// Get sensor type
    public fun get_sensor_type(sensor_data: &SensorData): string::String {
        sensor_data.sensor_type
    }

    /// Get all sensor readings
    public fun get_readings(sensor_data: &SensorData): (u64, u64, u64, u64) {
        (
            sensor_data.temperature,
            sensor_data.humidity,
            sensor_data.ec,
            sensor_data.ph
        )
    }

    /// Get all sensor data
    public fun get_all_data(sensor_data: &SensorData): (
        u64,
        u64,
        u64,
        u64,
        u64,
        string::String,
        string::String,
        string::String
    ) {
        (
            sensor_data.temperature,
            sensor_data.humidity,
            sensor_data.ec,
            sensor_data.ph,
            sensor_data.timestamp,
            sensor_data.device_id,
            sensor_data.location,
            sensor_data.sensor_type
        )
    }
}