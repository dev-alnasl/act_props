// -*- coding:utf-8-unix -*-
/**
 * @file   ADS1x1x.hpp
 * @brief  Interface for the ADS1x1x analog digital converter.
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
 * @class ADS1x1x
 * @brief Interface for the device.
 *
 * Provides configuration and conversion functions for the device.
 */
class ADS1x1x {
public:
    // MARK: Settings (public)

    /**
     * @brief Enum class defining possible I2C addresses for the device.
     *
     * The ADS1x1x adc can be addressed via two possible I2C addresses:
     * - PRIMARY: 0x48 (default)
     * - SECONDARY: 0x49 (alternative 1)
     * - TERTIARY: 0x4A (alternative 2)
     * - QUATERNARY: 0x4B (alternative 3)
     */
    enum class Address : uint8_t {
        PRIMARY = 0x48,
        SECONDARY = 0x49,
        TERTIARY = 0x4A,
        QUATERNARY = 0x4B
    };
    /**
     * @brief Helper function to retrieve the numeric value of an Address enum.
     */
    static constexpr uint8_t use(const Address e) { return static_cast<uint8_t>(e); }

    /**
     * @brief Enum class defining possible type for the device.
     *
     * The ADS1x1x adc has 2 types:
     * - ADS101x: 12-bit
     * - ADS111x: 16-bit
     */
    enum class DeviceType { ADS101x, ADS111x };

    /**
     * @brief Enum class for available channel configs for the device conversion.
     *
     * Defines the channel set at which the device can perform conversion.
     */
    enum class ChannelConfig : uint8_t {
        AIN0_AIN1 = 0x01,
        AIN0_AIN3 = 0x03,
        AIN1_AIN3 = 0x13,
        AIN2_AIN3 = 0x23,
        AIN0_GND = 0x00,
        AIN1_GND = 0x10,
        AIN2_GND = 0x20,
        AIN3_GND = 0x30
    };
    /**
     * @brief Helper function to retrieve the numeric value of a ChannelConfig enum.
     */
    static constexpr int use(const ChannelConfig e) { return static_cast<int>(e); }

    /**
     * @brief Enum class defining full-scale range configurations for the device.
     *
     * Specifies the input voltage range that the device can measure.
     * Smaller ranges provide better resolution, while larger ranges
     * support higher voltage inputs.
     */
    enum class FullScaleRange : uint16_t {
        FSR_6144mV = 6144,
        FSR_4096mV = 4096,
        FSR_2048mV = 2048,
        FSR_1024mV = 1024,
        FSR_0512mV = 512,
        FSR_0256mV = 256
    };
    /**
     * @brief Helper function to retrieve the numeric value of a FullScaleRange enum.
     */
    static constexpr int use(const FullScaleRange e) { return static_cast<int>(e); }

    /**
     * @brief Enum class defining data rates for the ADS1x1x ADC.
     *
     * Specifies the sampling rates in Samples Per Second (SPS) for the device.
     * Includes options for both ADS101x (12-bit) and ADS111x (16-bit) devices.
     */
    enum class DataRate : uint16_t {
        // Common
        DR_0128SPS = 128,
        DR_0250SPS = 250,
        // ADS101x
        DR_0490SPS = 490,
        DR_0920SPS = 920,
        DR_1600SPS = 1600,
        DR_2400SPS = 2400,
        DR_3300SPS = 3300,
        // ADS111x
        DR_0008SPS = 8,
        DR_0016SPS = 16,
        DR_0032SPS = 32,
        DR_0064SPS = 64,
        DR_0475SPS = 475,
        DR_0860SPS = 860
    };
    /**
     * @brief Helper function to retrieve the numeric value of a DataRate enum.
     */
    static constexpr int use(const DataRate e) { return static_cast<int>(e); }

    /**
     * @brief Configuration settings for the device.
     *
     * This structure encapsulates user-configurable settings for ADC operation,
     * allowing control over channel configuration, full-scale range, and data rate.
     */
    struct Settings {
        /// Channel configuration for the ADC
        ChannelConfig channel_config;

        /// Full-scale range for the ADC
        FullScaleRange full_scale_range;

        /// Data rate for ADC conversions
        DataRate data_rate;

        /**
         * @brief Predefined preset configurations for common use cases.
         */
        enum class Presets {
            DEFAULT    ///< Default configuration for general use
        };

        /**
         * @brief Default constructor initializes settings with the DEFAULT preset.
         */
        Settings() : Settings(Presets::DEFAULT) {}

        /**
         * @brief Parameterized constructor for custom settings.
         *
         * @param cc Channel configuration.
         * @param fsr Full-scale range.
         * @param dr Data rate.
         */
        Settings(ChannelConfig cc = ChannelConfig::AIN0_AIN1,
                 FullScaleRange fsr = FullScaleRange::FSR_2048mV,
                 DataRate dr = DataRate::DR_0128SPS)
            : channel_config(cc), full_scale_range(fsr), data_rate(dr) {}

        /**
         * @brief Constructor for initializing settings with a preset configuration.
         *
         * This constructor allows users to initialize ADC settings
         * using predefined configurations tailored for specific applications.
         *
         * @param preset The preset configuration to apply.
         */
        Settings(Presets preset) {
            switch (preset) {
            case Presets::DEFAULT: {
                channel_config = ChannelConfig::AIN0_AIN1;
                full_scale_range = FullScaleRange::FSR_2048mV;
                data_rate = DataRate::DR_0128SPS;
                break;
            }
            default: break;
            }
        }
    };

public:
    // MARK: Constants (public)

    /**
     * @brief Helper function to extract the channel number from a ChannelConfig enum.
     *
     * @param channel_config The ChannelConfig enum value to process.
     * @return The channel number extracted from the specified ChannelConfig value.
     */
    static constexpr uint8_t chNumberFrom(const ChannelConfig channel_config) {
        return use(channel_config) >> 4;
    }

private:
    // MARK: Constants (private)

    /**
     * @brief Calculate the conversion delay based on the data rate.
     *
     * This function computes the approximate delay required for a single
     * conversion cycle of the ADS1x1x ADC. The delay is calculated using
     * the formula:
     * \f[
     * \text{delay} = \frac{1000 + \text{data rate} - 1}{\text{data rate}}
     * \f]
     *
     * @param dr The data rate (SPS) for which to calculate the delay.
     * @return The conversion delay in milliseconds.
     */
    static constexpr uint32_t getConversionDelay(const DataRate dr) {
        return (1000 + use(dr) - 1) / use(dr);
    }

private:
    // MARK: States (private)

    /**
     * @brief Enumeration of internal states for the ADS1x1x ADC.
     *
     * Represents the various operational and transitional states of the ADC
     * during its lifecycle. These states are used internally to manage device behavior,
     * ensuring proper sequencing for conversion and data retrieval.
     *
     * States:
     * - `WAIT_SETUP`: Waiting for initial setup to complete.
     * - `WAIT_BEGIN`: Waiting for the device to begin operation.
     * - `IDLE`: ADC is idle and ready for a new conversion.
     * - `BUSY`: Conversion is currently in progress.
     * - `COMPLETE`: Conversion completed successfully.
     * - `ERROR`: An error occurred during conversion.
     * - `AVAILABLE`: Data is ready to be read.
     */
    enum class State : int {
        WAIT_SETUP,    ///< Waiting for setup to complete.
        WAIT_BEGIN,    ///< Waiting to begin operation.
        IDLE,          ///< ADC is idle and ready for a new conversion.
        BUSY,          ///< Conversion in progress.
        COMPLETE,      ///< Conversion successful.
        ERROR,         ///< Error during conversion.
        AVAILABLE      ///< Data is ready for reading.
    };
    /**
     * @brief Helper function to retrieve the numeric value of a State enum.
     */
    static constexpr int use(const State e) { return static_cast<int>(e); }

    /**
     * @brief Sets the adc to a specific state.
     * @param state The state to set.
     */
    inline void set(const State state) { _state = state; }

    /**
     * @brief Checks if the adc is in a specific state.
     * @param state The state to check against.
     * @return `true` if the adc is in the specified state; `false` otherwise.
     */
    inline bool in(const State state) { return _state == state; }

public:
    // MARK: Results (public)

    /**
     * @brief Enum class for operation results of ADS1x1x methods.
     *
     * Represents the status of operations performed by ADS1x1x methods.
     * Used to indicate success or specific failure reasons.
     */
    enum class Result : int32_t {
        SUCCESS,                  ///< Operation completed successfully.
        FAILED_NOT_RESPONDING,    ///< ADC is not responding.
        FAILED_BUSY,              ///< ADC is busy with another operation.
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
     * @brief Enum class for I2C register addresses of the ADS1x1x ADC.
     *
     * This enumeration defines the main registers used for device configuration
     * and data acquisition:
     * - `CONVERSION_REGISTER`: Stores the result of the last ADC conversion.
     * - `CONFIG_REGISTER`: Used to configure ADC settings such as channel, data rate,
     *   and full-scale range.
     * - `LO_THRESH_REGISTER`: Low threshold for the comparator.
     * - `HI_THRESH_REGISTER`: High threshold for the comparator.
     */
    enum class Register : uint8_t {
        CONVERSION_REGISTER = 0x00,    ///< Conversion result register.
        CONFIG_REGISTER = 0x01,        ///< Configuration register.
        LO_THRESH_REGISTER = 0x02,     ///< Low threshold register for comparator.
        HI_THRESH_REGISTER = 0x03      ///< High threshold register for comparator.
    };
    /**
     * @brief Helper function to retrieve the numeric value of a Register enum.
     */
    static constexpr uint8_t use(const Register e) { return static_cast<uint8_t>(e); }

    /**
     * @brief Bitfield definitions for the CONFIG_REGISTER of the ADS1x1x ADC.
     *
     * This enumeration defines individual bits and groups of bits used in the
     * CONFIG_REGISTER for configuring the ADC's behavior. The CONFIG_REGISTER is
     * responsible for:
     * - Setting the operational mode (single-shot or continuous).
     * - Configuring the input multiplexer.
     * - Selecting the programmable gain amplifier (PGA) settings.
     * - Setting the data rate.
     * - Configuring the comparator mode, polarity, latching, and queue.
     *
     * Each field corresponds to a specific functionality within the ADC's configuration
     * register.
     */
    enum class CONFIG_REGISTER : int {
        CONF_OS = 15,      ///< Operational status (1 = start single-shot conversion).
        CONF_MUX2 = 14,    ///< Input multiplexer bit 2.
        CONF_MUX1 = 13,    ///< Input multiplexer bit 1.
        CONF_MUX0 = 12,    ///< Input multiplexer bit 0.
        CONF_PGA2 = 11,    ///< Programmable gain amplifier bit 2.
        CONF_PGA1 = 10,    ///< Programmable gain amplifier bit 1.
        CONF_PGA0 = 9,     ///< Programmable gain amplifier bit 0.
        CONF_MODE = 8,     ///< Operational mode (0 = continuous, 1 = single-shot).
        CONF_DR2 = 7,      ///< Data rate bit 2.
        CONF_DR1 = 6,      ///< Data rate bit 1.
        CONF_DR0 = 5,      ///< Data rate bit 0.
        CONF_COMP_MODE = 4,    ///< Comparator mode (0 = traditional, 1 = window).
        CONF_COMP_POL = 3,     ///< Comparator polarity (0 = active low, 1 = active
                               ///< high).
        CONF_COMP_LAT = 2,    ///< Comparator latching (0 = non-latching, 1 = latching).
        CONF_COMP_QUE1 = 1,    ///< Comparator queue bit 1.
        CONF_COMP_QUE0 = 0     ///< Comparator queue bit 0.
    };
    /**
     * @brief Helper function to retrieve the numeric value of a CONFIG_REGISTER enum.
     */
    static constexpr int use(const CONFIG_REGISTER e) { return static_cast<int>(e); }

private:
    // MARK: Variables (private)

    /// Current state of the adc
    State _state;

    /// Last encountered error
    Result _error;

    /// Error message corresponding to the last error
    char _error_message[49];

    /// I2C address of the adc
    Address _address;

    /// Configuration settings for the adc
    Settings _settings;

    /// Device type
    DeviceType _device_type;

    /// Last time data requested
    uint32_t _latest_request_time;

    /// Latest measured values
    struct {
        uint16_t raw;
        uint16_t voltage;
    } _values;

public:
    // MARK: Const/Destructor (public)

    /**
     * @brief Constructor for the ADS1x1x class.
     *
     * Initializes the ADS1x1x adc object with default settings, state, and error
     * status.
     */
    ADS1x1x()
        : _state(State::WAIT_SETUP), _address(Address::PRIMARY),
          _device_type(DeviceType::ADS101x),
          _settings(Settings(Settings::Presets::DEFAULT)), _latest_request_time(0),
          _values { 0 } {}

    /**
     * @brief Destructor for the ADS1x1x class.
     *
     * Cleans up any resources or states before the object is destroyed.
     */
    ~ADS1x1x() {}

public:
    // MARK: Set/Get (public)

    /**
     * @brief Retrieves the latest error message.
     *
     * Provides the most recent error message encountered during
     * the operation of the ADS1x1x adc.
     *
     * @return A pointer to the character array containing the error message.
     */
    inline char* getErrorMessage() { return _error_message; }

    /**
     * @brief Retrieves the I2C address of the adc.
     *
     * Provides the currently configured I2C address of the ADS1x1x adc.
     *
     * @return The `Address` enum representing the I2C address.
     */
    inline Address getAddress() const { return _address; }

    /**
     * @brief Sets the I2C address of the adc.
     *
     * Configures the I2C address for the ADS1x1x adc to the provided value.
     *
     * @param address The `Address` enum representing the desired I2C address.
     */
    inline void setAddress(const Address address) { _address = address; }

    /**
     * @brief Retrieves the current adc settings.
     *
     * Provides access to the current temperature and pressure settings
     * configured for the ADS1x1x adc.
     *
     * @return A reference to the `Settings` structure containing the configuration.
     */
    inline Settings& getSettings() { return _settings; }

    /**
     * @brief Configures the adc settings.
     *
     * Updates the temperature and pressure settings for the ADS1x1x adc
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
     * of the ADS1x1x adc.
     *
     * @return The `ADS1x1x::Result` indicating the error cause.
     */
    inline Result getError() const { return _error; }

    /**
     * @brief Sets a new error cause.
     *
     * Updates the internal error state and logs an appropriate
     * error message based on the cause.
     *
     * @param cause The `ADS1x1x::Result` representing the error cause.
     */
    inline void setError(const Result cause) {
        _error = cause;
        switch (cause) {
        case Result::FAILED_BUSY:
            snprintf_(_error_message, sizeof(_error_message), "Error: ADS1x1x is busy");
            break;
        case Result::FAILED_NOT_RESPONDING:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: ADS1x1x is not responding");
            break;
        case Result::FAILED_UNKNOWN:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: Unknown issue with ADS1x1x");
            break;
        default: break;
        }
    }

    /**
     * @brief Retrieves the device type of the ADC.
     *
     * Provides the current device type for the ADS1x1x ADC, which helps
     * in distinguishing between models such as ADS101x or ADS111x.
     *
     * @return The `DeviceType` enumeration representing the ADC model.
     */
    inline DeviceType getDeviceType() const { return _device_type; }

    /**
     * @brief Sets the device type of the ADC.
     *
     * Configures the device type for the ADS1x1x ADC to one of the supported models,
     * ensuring proper operation and compatibility with the driver.
     *
     * @param device_type The `DeviceType` enumeration representing the ADC model.
     */
    inline void setDeviceType(const DeviceType device_type) {
        _device_type = device_type;
    }

public:
    // MARK: Interfaces (public)

    /**
     * @brief Setup the ADS1x1x adc.
     *
     * Configures the ADS1x1x adc with the specified I2C address and settings.
     *
     * @param address The I2C address to use for communication with the adc (default:
     * PRIMARY).
     * @param device_type The device type of the adc (default: ADS101x).
     * @param settings The configuration settings for the adc (default: DEFAULT
     * preset).
     */
    void setup(const Address address = Address::PRIMARY,
               const DeviceType device_type = DeviceType::ADS101x,
               const Settings& settings = Settings(Settings::Presets::DEFAULT));

    /**
     * @brief Begin conversions.
     *
     * Initializes the adc and prepares it for operation, starting conversion
     * processes.
     */
    void begin();

    /**
     * @brief Update the adc state.
     *
     * Updates the adc's state and handles ongoing conversion tasks. This function
     * should be called periodically in the main loop to maintain adc functionality.
     */
    void update();

    /**
     * @brief End conversions.
     *
     * Stops the conversion processes and powers down the adc.
     */
    void end();

    /**
     * @brief Check if data is available for reading.
     *
     * Determines if the adc has completed conversions and the data is ready for
     * retrieval.
     *
     * @return `true` if data is available; otherwise, `false`.
     */
    inline bool available() { return in(ADS1x1x::State::AVAILABLE); }

    /**
     * @brief Prepare the adc for sleep mode.
     *
     * Performs any necessary steps to put the adc into a low-power state.
     */
    inline void onSleep() const {}

    /**
     * @brief Wake the adc from sleep mode.
     *
     * Restores the adc from a low-power state, preparing it for operation.
     */
    inline void onWakeup() const {}

    /**
     * @brief Request conversion with the specific channel.
     *
     * Initiates a conversion sequence using the configured settings of
     * the ADS1x1x adc.
     * @param channel_config Channel to use.
     * @return `ADS1x1x::Result` indicating the success or failure of the request.
     */
    Result request(ChannelConfig channel_config);

    /**
     * @brief Read voltage data after a conversion request.
     *
     * Retrieves the voltage data converted by the adc.
     * Ensure `request()` has been called before using this method.
     *
     * @param voltage Pointer to store the voltage value (mV).
     * @return `ADS1x1x::Result` indicating the success or failure of the read
     * operation.
     */
    Result read(uint16_t* const voltage);

private:
    // MARK: Specific utils (private)

    /**
     * @brief Apply saved full scale range configurations from settings.
     *
     * Updates the adc's full scale range (FSR) settings based on the
     * current configuration stored in the `Settings` structure.
     *
     * @return `ADS1x1x::Result` indicating the success or failure of the operation.
     */
    Result applyFullScaleRange();

    /**
     * @brief Apply saved data rate configurations from settings.
     *
     * Updates the adc's data rate (DR) settings based on the
     * current configuration stored in the `Settings` structure.
     *
     * @return `ADS1x1x::Result` indicating the success or failure of the operation.
     */
    Result applyDataRate();

private:
    // MARK: Common I2C utils (private)

    /**
     * @brief Read 8-bit data via I2C.
     *
     * Reads a single byte of data from the specified register address.
     *
     * @param reg Register address to read from.
     * @param dst Pointer to the variable where the read data will be stored.
     * @return A `ADS1x1x::Result` indicating success or failure of the read operation.
     */
    Result read(const Register reg, uint8_t* const dst);

    /**
     * @brief Read 16-bit data via I2C.
     *
     * Reads 2 bytes of data from the specified register address.
     *
     * @param reg Register address to read from.
     * @param dst Pointer to the variable where the read data will be stored.
     * @return A `ADS1x1x::Result` indicating success or failure of the read operation.
     */
    Result read(const Register reg, uint16_t* const dst);

    /**
     * @brief Write 8-bit or 16-bit data via I2C.
     *
     * Writes a single or two byte(s) of data to the specified register address.
     *
     * @param reg Register address to write to.
     * @param src Data byte to write.
     * @return A `ADS1x1x::Result` indicating success or failure of the write operation.
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
};

// MARK: Operators for results (global)

/**
 * @brief Logical NOT operator for `ADS1X1X::Result`.
 *
 * Evaluates whether the provided result is not `ADS1X1X::Result::SUCCESS`.
 *
 * @param result The `ADS1X1X::Result` value to evaluate.
 * @return `true` if the result is not `ADS1x1x::Result::SUCCESS`; otherwise, `false`.
 */
bool operator!(ADS1x1x::Result result);

/**
 * @brief Logical AND operator for combining `ADS1x1x::Result` values.
 *
 * Combines two `ADS1x1x::Result` values. If both values are `ADS1x1x::Result::SUCCESS`,
 * the result is `ADS1x1x::Result::SUCCESS`; otherwise, the result is
 * `ADS1x1x::Result::FAILED_UNKNOWN`.
 *
 * @param lhs The left-hand operand.
 * @param rhs The right-hand operand.
 * @return The combined `ADS1x1x::Result` value.
 */
ADS1x1x::Result operator&&(ADS1x1x::Result lhs, ADS1x1x::Result rhs);

/**
 * @brief Logical OR operator for combining `ADS1x1x::Result` values.
 *
 * Combines two `ADS1x1x::Result` values. If either value is `ADS1x1x::Result::SUCCESS`,
 * the result is `ADS1x1x::Result::SUCCESS`; otherwise, the result is
 * `ADS1x1x::Result::FAILED_UNKNOWN`.
 *
 * @param lhs The left-hand operand.
 * @param rhs The right-hand operand.
 * @return The combined `ADS1x1x::Result` value.
 */
ADS1x1x::Result operator||(ADS1x1x::Result lhs, ADS1x1x::Result rhs);