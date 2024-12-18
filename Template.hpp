// -*- coding:utf-8-unix -*-
/**
 * @file   Template.hpp
 * @brief  Interface for the <xxx sensor>.
 *
 * @copyright
 * (C) 2024 Mono Wireless Inc. All Rights Reserved.
 * Released under MW-OSSLA-*J,*E (MONO WIRELESS OPEN SOURCE SOFTWARE LICENSE AGREEMENT).
 */

#pragma once

/**
 * @brief Header file dependency.
 *
 * Includes the MWX library, which provides the necessary interfaces for communication.
 */
#include <TWELITE>

/**
 * @class _DEVICE_
 * @brief Interface for the device.
 *
 * Provides configuration, measurement, and calibration functions for the device.
 */
class _DEVICE_ {
public:
    // MARK: Settings (public)

    /**
     * @brief Enum class defining possible I2C addresses for the device.
     *
     * The _DEVICE_ device can be addressed via two possible I2C addresses:
     * - PRIMARY: 0xFF (default)
     * - SECONDARY: 0xFF (alternative)
     */
    enum class Address : uint8_t {
        PRIMARY = 0xFF,     ///< Primary I2C address
        SECONDARY = 0xFF    ///< Secondary I2C address
    };
    /**
     * @brief Helper function to retrieve the numeric value of an Address enum.
     */
    static constexpr uint8_t use(const Address e) { return static_cast<uint8_t>(e); }


    enum class SomeParameter : uint8_t {
        A = 0b01,
        B = 0b10,
    };
    /**
     * @brief Helper function to retrieve the integer value of a SomeParameter enum.
     */
    static constexpr int use(const SomeParameter e) { return static_cast<int>(e); }

    /**
     * @brief Configuration settings for the device measurements.
     *
     * This structure encapsulates user-configurable settings for both temperature
     * and pressure measurements, allowing control over sampling rate, precision,
     * temperature source, and preset profiles tailored for specific use cases.
     */
    struct Settings {
        /// Some parameter
        SomeParameter some_parameter;

        /**
         * @brief Predefined preset configurations for common use cases.
         */
        enum class Presets {
            DEFAULT,    ///< Default configuration for general use
        };

        /**
         * @brief Default constructor initializes settings with the DEFAULT preset.
         */
        Settings() : Settings(Presets::DEFAULT) {}

        /**
         * @brief Parameterized constructor for custom settings.
         *
         * @param sv Some value
         */
        Settings(SomeParameter sv = SomeParameter::A) : some_parameter(sv) {}

        /**
         * @brief Constructor for initializing settings with a preset configuration.
         *
         * This constructor allows users to initialize device settings
         * using predefined configurations tailored for specific applications.
         *
         * @param preset The preset configuration to apply.
         */
        Settings(Presets preset) {
            switch (preset) {
            case Presets::DEFAULT: {
                some_parameter = SomeParameter::A;
                break;
            }
            default: break;
            }
        }
    };

public:
    // MARK: Constants (public)


private:
    // MARK: Constants (private)


private:
    // MARK: States (private)

    /**
     * @brief Enumeration of internal states for the device.
     *
     * Represents the various operational and transitional states of the device
     * during its lifecycle. These states are used internally to manage device behavior,
     * ensuring proper sequencing for measurement and data retrieval.
     *
     * States:
     * - `WAIT_SETUP`: Waiting for initial setup to complete.
     * - `WAIT_BEGIN`: Waiting for the device to begin operation.
     * - `IDLE`: Device is idle and ready for a new measurement.
     * - `BUSY`: A measurement is in progress.
     * - `COMPLETE`: Measurement completed successfully.
     * - `ERROR`: An error occurred during measurement.
     * - `AVAILABLE`: Data is ready to be read.
     */
    enum class State : int {
        WAIT_SETUP,    ///< Waiting for setup to complete.
        WAIT_BEGIN,    ///< Waiting to begin operation.
        IDLE,          ///< Device is idle and ready for a new measurement.
        BUSY,          ///< Measurement in progress.
        COMPLETE,      ///< Measurement successful.
        ERROR,         ///< Error during temperature measurement.
        AVAILABLE      ///< Data is ready for reading.
    };
    /**
     * @brief Helper function to retrieve the numeric value of an State enum.
     */
    static constexpr int use(const State e) { return static_cast<int>(e); }

    /**
     * @brief Sets the device to a specific state.
     * @param state The state to set.
     */
    inline void set(const State state) { _state = state; }

    /**
     * @brief Checks if the device is in a specific state.
     * @param state The state to check against.
     * @return `true` if the device is in the specified state; `false` otherwise.
     */
    inline bool in(const State state) { return _state == state; }

public:
    // MARK: Results (public)
    /**
     * @brief Enum class for operation results of the device methods.
     *
     * Represents the status of operations performed by the device methods.
     * Used to indicate success or specific failure reasons.
     */
    enum class Result : int32_t {
        SUCCESS,                  ///< Operation completed successfully.
        FAILED_NOT_RESPONDING,    ///< Device is not responding.
        FAILED_BUSY,              ///< Device is busy with another operation.
        FAILED_UNKNOWN            ///< An unknown error occurred.
    };

    /**
     * @brief Logical NOT operator for the `Result` enum.
     * @param result The result to evaluate.
     * @return `true` if the result is not `SUCCESS`, otherwise `false`.
     */
    friend bool operator!(Result result);

    /**
     * @brief Logical AND operator for combining two `Result` values.
     * @param lhs Left-hand operand.
     * @param rhs Right-hand operand.
     * @return `Result::SUCCESS` if both results are `SUCCESS`,
     *         otherwise `FAILED_UNKNOWN`.
     */
    friend Result operator&&(Result lhs, Result rhs);

    /**
     * @brief Logical OR operator for combining two `Result` values.
     * @param lhs Left-hand operand.
     * @param rhs Right-hand operand.
     * @return `Result::SUCCESS` if either result is `SUCCESS`,
     *         otherwise `FAILED_UNKNOWN`.
     */
    friend Result operator||(Result lhs, Result rhs);

private:
    // MARK: Registers (private)

    /**
     * @brief Enum class representing I2C register addresses for the device.
     */
    enum class Register : uint8_t {
        REG0 = 0x00,
    };
    /**
     * @brief Helper function to retrieve the integer value of a `Register` enum.
     */
    static constexpr uint8_t use(const Register e) { return static_cast<uint8_t>(e); }

    // 0 Register (REG0)
    /**
     * @brief Configuration for the value.
     *
     * Defines bit positions in the REG0 register for controlling
     * something.
     */
    enum class REG0 : uint8_t {
        REG0_6 = 6,
        REG0_5 = 5,
        REG0_4 = 4,
        REG0_3= 3,
        REG0_2 = 2,
        REG0_1 = 1,
        REG0_0 = 0
    };
    /**
     * @brief Helper function to retrieve the integer value of a `REG0` enum.
     */
    static constexpr int use(const REG0 e) { return static_cast<int>(e); }

private:
    // MARK: Variables (private)

    /// Current state of the device
    State _state;

    /// Last encountered error
    Result _error;

    /// Error message corresponding to the last error
    char _error_message[49];

    /// I2C address of the device
    Address _address;

    /// Configuration settings for the device
    Settings _settings;

    /// Latest measured values
    struct {
        int32_t value;
    } _values;

public:
    // MARK: Const/Destructor (public)

    /**
     * @brief Constructor for the device interface.
     *
     * Initializes the device object with default settings, state, and error
     * status.
     */
    _DEVICE_()
        : _state(State::WAIT_SETUP), _error(Result::FAILED_UNKNOWN),
          _error_message { 0 }, _address(Address::PRIMARY),
          _settings(Settings(Settings::Presets::DEFAULT)),
          _values { 0 } {}

    /**
     * @brief Destructor for the device interface.
     *
     * Cleans up any resources or states before the object is destroyed.
     */
    ~_DEVICE_() {}

public:
    // MARK: Set/Get (public)

    /**
     * @brief Retrieves the latest error message.
     *
     * Provides the most recent error message encountered during
     * the operation of the device.
     *
     * @return A pointer to the character array containing the error message.
     */
    inline char* getErrorMessage() { return _error_message; }

    /**
     * @brief Retrieves the I2C address of the device.
     *
     * Provides the currently configured I2C address of the device.
     *
     * @return The `Address` enum representing the I2C address.
     */
    inline Address getAddress() const { return _address; }

    /**
     * @brief Sets the I2C address of the device.
     *
     * Configures the I2C address for the device to the provided value.
     *
     * @param address The `Address` enum representing the desired I2C address.
     */
    inline void setAddress(const Address address) { _address = address; }

    /**
     * @brief Retrieves the current device settings.
     *
     * Provides access to the current temperature and pressure settings
     * configured for the device.
     *
     * @return A reference to the `Settings` structure containing the configuration.
     */
    inline Settings& getSettings() { return _settings; }

    /**
     * @brief Configures the device settings.
     *
     * Updates the temperature and pressure settings for the device
     * using the provided configuration.
     *
     * @param settings The `Settings` structure containing the desired configuration.
     */
    inline void setSettings(const Settings& settings) { _settings = settings; }

private:
    // MARK: Set/Get (private)

    /**
     * @brief Retrieves the latest error cause.
     *
     * Provides the most recent error encountered during the operation
     * of the device.
     *
     * @return The `_DEVICE_::Result` indicating the error cause.
     */
    inline Result getError() const { return _error; }

    /**
     * @brief Sets a new error cause.
     *
     * Updates the internal error state and logs an appropriate
     * error message based on the cause.
     *
     * @param cause The `_DEVICE_::Result` representing the error cause.
     */
    inline void setError(const Result cause) {
        _error = cause;
        switch (cause) {
        case Result::FAILED_BUSY:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: _DEVICE_ is busy");
            break;
        case Result::FAILED_NOT_RESPONDING:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: _DEVICE_ is not responding");
            break;
        case Result::FAILED_UNKNOWN:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: Unknown issue with _DEVICE_");
            break;
        default: break;
        }
    }

public:
    // MARK: Interfaces (public)

    /**
     * @brief Setup the device.
     *
     * Configures the device with the specified I2C address and settings.
     *
     * @param address The I2C address to use for communication with the device (default:
     * PRIMARY).
     * @param settings The configuration settings for the device (default: DEFAULT
     * preset).
     */
    void setup(const Address address = Address::PRIMARY,
               const Settings& settings = Settings(Settings::Presets::DEFAULT));

    /**
     * @brief Begin measurements.
     *
     * Initializes the device and prepares it for operation, starting measurement
     * processes.
     */
    void begin();

    /**
     * @brief Update the device state.
     *
     * Updates the device's state and handles ongoing measurement tasks. This function
     * should be called periodically in the main loop to maintain device functionality.
     */
    void update();

    /**
     * @brief End measurements.
     *
     * Stops the measurement processes and powers down the device.
     */
    void end();

    /**
     * @brief Check if data is available for reading.
     *
     * Determines if the device has completed measurements and the data is ready for
     * retrieval.
     *
     * @return `true` if data is available; otherwise, `false`.
     */
    inline bool available() { return in(_DEVICE_::State::AVAILABLE); }

    /**
     * @brief Prepare the device for sleep mode.
     *
     * Performs any necessary steps to put the device into a low-power state.
     */
    inline void onSleep() const {}

    /**
     * @brief Wake the device from sleep mode.
     *
     * Restores the device from a low-power state, preparing it for operation.
     */
    inline void onWakeup() const {}

    /**
     * @brief Request temperature and pressure measurement.
     *
     * Initiates a measurement sequence for both temperature and pressure
     * using the configured settings of the device.
     *
     * @return `_DEVICE_::Result` indicating the success or failure of the request.
     */
    Result request();

    /**
     * @brief Read temperature and pressure data after a measurement request.
     *
     * Retrieves the temperature and pressure data measured by the device.
     * Ensure `request()` has been called before using this method.
     *
     * @param value Pointer to store the value (Any).
     * @return `_DEVICE_::Result` indicating the success or failure of the read
     * operation.
     */
    Result read(int32_t* const value);

    /**
     * @brief Perform a software reset of the device.
     *
     * Resets the device to its default state without a hardware power cycle.
     *
     * @return `_DEVICE_::Result` indicating the success or failure of the reset
     * operation.
     */
    Result softReset();

private:
    // MARK: Specific utils (private)

    /**
     * @brief Apply some configurations from settings.
     *
     * Updates the device's some measurement settings based on the
     * current configuration stored in the `Settings` structure.
     *
     * @return `_DEVICE_::Result` indicating the success or failure of the operation.
     */
    Result applySomeSettings();

private:
    // MARK: Common I2C utils (private)

    /**
     * @brief Read 8-bit data via I2C.
     *
     * Reads a single byte of data from the specified register address.
     *
     * @param reg Register address to read from.
     * @param dst Pointer to the variable where the read data will be stored.
     * @return A `_DEVICE_::Result` indicating success or failure of the read operation.
     */
    Result read(const Register reg, uint8_t* const dst);

    /**
     * @brief Read 16-bit data via I2C.
     *
     * Reads 2 bytes of data from the specified register address.
     *
     * @param reg Register address to read from.
     * @param dst Pointer to the variable where the read data will be stored.
     * @return A `_DEVICE_::Result` indicating success or failure of the read operation.
     */
    Result read(const Register reg, uint16_t* const dst);

    /**
     * @brief Write 8-bit or 16-bit data via I2C.
     *
     * Writes a single or two byte(s) of data to the specified register address.
     *
     * @param reg Register address to write to.
     * @param src Data byte to write.
     * @return A `_DEVICE_::Result` indicating success or failure of the write
     * operation.
     */
    Result write(const Register reg, const int src);

private:
    // MARK: Common byte utils (private)

    /**
     * @brief Set a single bit in the target byte.
     *
     * Changes the value of a specific bit in the target byte to the given value (0 or
     * 1).
     *
     * @param target Pointer to the byte to modify.
     * @param position Bit position to modify (from LSB).
     * @param value New value for the bit (0 or 1).
     */
    static inline void
    setBit(uint8_t* const target, const int position, const int value) {
        if (value > 0) {
            *target |= (1U << position);
        } else {
            *target &= ~(1U << position);
        }
    }

    /**
     * @brief Set a single bit in the target word.
     *
     * Changes the value of a specific bit in the target word to the given value (0 or
     * 1).
     *
     * @param target Pointer to the word to modify.
     * @param position Bit position to modify (from LSB).
     * @param value New value for the bit (0 or 1).
     */
    static inline void
    setBit(uint16_t* const target, const int position, const int value) {
        if (value > 0) {
            *target |= (1U << position);
        } else {
            *target &= ~(1U << position);
        }
    }

    /**
     * @brief Set multiple bits in the target byte.
     *
     * Updates a range of bits in the target byte with the specified value, starting at
     * the given bit position and spanning the specified width.
     *
     * @param target Pointer to the byte to modify.
     * @param position Starting bit position to modify (from LSB).
     * @param value The new value to write into the bit range (e.g., 0b101).
     * @param width Width of the bit range to modify (in bits).
     */
    static inline void setPattern(uint8_t* const target, const int position,
                                  const int value, const int width) {
        *target =
            (*target & ~((0xFF >> (8 - width)) << position)) | (value << position);
    }

    /**
     * @brief Set multiple bits in the target word.
     *
     * Updates a range of bits in the target word with the specified value, starting at
     * the given bit position and spanning the specified width.
     *
     * @param target Pointer to the word to modify.
     * @param position Starting bit position to modify (from LSB).
     * @param value The new value to write into the bit range (e.g., 0b101).
     * @param width Width of the bit range to modify (in bits).
     */
    void setPattern(uint16_t* target, int position, int value, int width) {
        *target =
            (*target & ~((0xFFFF >> (16 - width)) << position)) | (value << position);
    }

    /**
     * @brief Check if a specific bit is set in a target byte.
     *
     * Determines whether the specified bit in the target byte is set.
     *
     * @param target The byte to check.
     * @param bit The bit position to check (from LSB).
     * @return `true` if the specified bit is set; otherwise, `false`.
     */
    static inline bool hasBitSet(const uint8_t target, const int bit) {
        return target & (1U << bit);
    }

    /**
     * @brief Check if a specific bit is set in a target word.
     *
     * Determines whether the specified bit in the target word is set.
     *
     * @param target The word to check.
     * @param bit The bit position to check (from LSB).
     * @return `true` if the specified bit is set; otherwise, `false`.
     */
    static inline bool hasBitSet(const uint16_t target, const int bit) {
        return target & (1U << bit);
    }

    /**
     * @brief Check if specific bits are set in a target byte.
     *
     * Determines whether a specific bit pattern is present in the target byte,
     * starting at a given position and spanning the specified width.
     *
     * @param target The byte to check.
     * @param shift Starting bit position to check (from LSB).
     * @param bits The bit pattern to verify (e.g., 0b101).
     * @param width Width of the bit pattern (in bits).
     * @return `true` if the bit pattern is set; otherwise, `false`.
     */
    static inline bool
    hasPattern(const uint8_t target, const int shift, const int bits, const int width) {
        return ((target >> shift) & ((1U << width) - 1)) == bits;
    }

    /**
     * @brief Check if specific bits are set in a target word.
     *
     * Determines whether a specific bit pattern is present in the target word,
     * starting at a given position and spanning the specified width.
     *
     * @param target The word to check.
     * @param shift Starting bit position to check (from LSB).
     * @param bits The bit pattern to verify (e.g., 0b101).
     * @param width Width of the bit pattern (in bits).
     * @return `true` if the bit pattern is set; otherwise, `false`.
     */
    static inline bool hasPattern(const uint16_t target, const int shift,
                                  const int bits, const int width) {
        return ((target >> shift) & ((1U << width) - 1)) == bits;
    }

private:
    // MARK: Common misc. utils (private)

    /**
     * @brief Compute the two's complement of a value.
     *
     * Converts the given raw value to its two's complement representation
     * based on the specified bit length. This function handles both positive
     * and negative numbers, ensuring proper sign extension.
     *
     * @param raw_value The raw input value to be converted.
     * @param bit_length The bit length of the input value.
     * @return The two's complement representation of the input value.
     */
    static inline int32_t twosComplement(int32_t raw_value, const int bit_length) {
        // Check if the sign bit is set by shifting to the sign bit position
        if (raw_value & (1U << (bit_length - 1))) {
            // Perform sign extension by setting upper bits to 1
            raw_value |= ~((1U << bit_length) - 1);
        } else {
            // Clear upper bits to zero for positive numbers
            raw_value &= (1U << bit_length) - 1;
        }
        return raw_value;
    }
};

// MARK: Operators for results (global)

/**
 * @brief Logical NOT operator for `_DEVICE_::Result`.
 *
 * Evaluates whether the provided result is not `_DEVICE_::Result::SUCCESS`.
 *
 * @param result The `_DEVICE_::Result` value to evaluate.
 * @return `true` if the result is not `_DEVICE_::Result::SUCCESS`; otherwise, `false`.
 */
bool operator!(_DEVICE_::Result result);

/**
 * @brief Logical AND operator for combining `_DEVICE_::Result` values.
 *
 * Combines two `_DEVICE_::Result` values. If both values are
 * `_DEVICE_::Result::SUCCESS`, the result is `_DEVICE_::Result::SUCCESS`; otherwise,
 * the result is
 * `_DEVICE_::Result::FAILED_UNKNOWN`.
 *
 * @param lhs The left-hand operand.
 * @param rhs The right-hand operand.
 * @return The combined `_DEVICE_::Result` value.
 */
_DEVICE_::Result operator&&(_DEVICE_::Result lhs, _DEVICE_::Result rhs);

/**
 * @brief Logical OR operator for combining `_DEVICE_::Result` values.
 *
 * Combines two `_DEVICE_::Result` values. If either value is
 * `_DEVICE_::Result::SUCCESS`, the result is `_DEVICE_::Result::SUCCESS`; otherwise,
 * the result is
 * `_DEVICE_::Result::FAILED_UNKNOWN`.
 *
 * @param lhs The left-hand operand.
 * @param rhs The right-hand operand.
 * @return The combined `_DEVICE_::Result` value.
 */
_DEVICE_::Result operator||(_DEVICE_::Result lhs, _DEVICE_::Result rhs);
