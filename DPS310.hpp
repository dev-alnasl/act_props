// -*- coding:utf-8-unix -*-
/**
 * @file   DPS310.hpp
 * @brief  Interface for the DPS310 barometric pressure sensor.
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
 * @class DPS310
 * @brief Interface for the device.
 *
 * Provides configuration, measurement, and calibration functions for the device.
 */
class DPS310 {
public:
    // MARK: Settings (public)

    /**
     * @brief Enum class defining possible I2C addresses for the device.
     *
     * The DPS310 device can be addressed via two possible I2C addresses:
     * - PRIMARY: 0x77 (default)
     * - SECONDARY: 0x76 (alternative)
     */
    enum class Address : uint8_t {
        PRIMARY = 0x77,     ///< Primary I2C address
        SECONDARY = 0x76    ///< Secondary I2C address
    };
    /**
     * @brief Helper function to retrieve the numeric value of an Address enum.
     */
    static constexpr uint8_t use(const Address e) { return static_cast<uint8_t>(e); }

    /**
     * @brief Enum class for available sampling rates for the device measurements.
     *
     * Defines the frequency at which the device can perform measurements.
     * Higher sampling rates increase measurement frequency but may increase power
     * consumption.
     */
    enum class SamplingRate : uint8_t {
        SAMPLING_1HZ = 0b000,     ///< 1 measurement per second (1 Hz)
        SAMPLING_2HZ = 0b001,     ///< 2 measurements per second (2 Hz)
        SAMPLING_4HZ = 0b010,     ///< 4 measurements per second (4 Hz)
        SAMPLING_8HZ = 0b011,     ///< 8 measurements per second (8 Hz)
        SAMPLING_16HZ = 0b100,    ///< 16 measurements per second (16 Hz)
        SAMPLING_32HZ = 0b101,    ///< 32 measurements per second (32 Hz)
        SAMPLING_64HZ = 0b110,    ///< 64 measurements per second (64 Hz)
        SAMPLING_128HZ = 0b111    ///< 128 measurements per second (128 Hz)
    };
    /**
     * @brief Helper function to retrieve the integer value of a SamplingRate enum.
     */
    static constexpr int use(const SamplingRate e) { return static_cast<int>(e); }

    /**
     * @brief Enum class for precision settings, representing oversampling rates.
     *
     * Defines the level of precision for measurements by configuring oversampling
     * settings. Higher precision provides more accurate measurements but may increase
     * the measurement time.
     */
    enum class Precision : uint8_t {
        LOW_1X = 0b0000,          ///< 1x oversampling: ~2.5 Pa RMS, ~3.6 ms
        LOW_2X = 0b0001,          ///< 2x oversampling: ~1.0 Pa RMS, ~5.2 ms
        LOW_4X = 0b0010,          ///< 4x oversampling: ~0.5 Pa RMS, ~8.4 ms
        LOW_8X = 0b0011,          ///< 8x oversampling: ~0.4 Pa RMS, ~14.8 ms
        STANDARD_16X = 0b0100,    ///< 16x oversampling: ~0.35 Pa RMS, ~27.6 ms
        HIGH_32X = 0b0101,        ///< 32x oversampling: ~0.3 Pa RMS, ~53.2 ms
        HIGH_64X = 0b0110,        ///< 64x oversampling: ~0.2 Pa RMS, ~104.4 ms
        HIGH_128X = 0b0111        ///< 128x oversampling: highest precision, ~206.8 ms
    };
    /**
     * @brief Helper function to retrieve the integer value of a Precision enum.
     */
    static constexpr int use(const Precision e) { return static_cast<int>(e); }

    /**
     * @brief Enum class for available operating modes of the device.
     *
     * Defines the modes in which the device can operate,
     * allowing for control over one-shot and continuous measurement modes.
     */
    enum class OperationMode : uint8_t {
        STANDBY = 0b000,                   ///< Standby mode (no measurements)
        ONE_SHOT_PRESSURE = 0b001,         ///< One-shot pressure measurement
        ONE_SHOT_TEMPERATURE = 0b010,      ///< One-shot temperature measurement
        CONTINUOUS_PRESSURE = 0b101,       ///< Continuous pressure measurement
        CONTINUOUS_TEMPERATURE = 0b110,    ///< Continuous temperature measurement
        CONTINUOUS_PRESSURE_AND_TEMPERATURE = 0b111    ///< Continuous pressure and
                                                       ///< temperature measurement
    };
    /**
     * @brief Helper function to retrieve the integer value of an OperationMode enum.
     */
    static constexpr int use(const OperationMode e) { return static_cast<int>(e); }

    /**
     * @brief Enum class for temperature measurement source selection.
     *
     * Defines the source for temperature readings:
     * - ASIC_LOW_POWER: Uses the internal ASIC sensor for low power consumption.
     * - MEMS_HIGH_PRECISION: Uses the external MEMS sensor for high precision.
     */
    enum class TemperatureSource : uint8_t {
        ASIC_LOW_POWER = 0,        ///< Internal ASIC temperature sensor for low power
        MEMS_HIGH_PRECISION = 1    ///< External MEMS temperature sensor for high
                                   ///< precision
    };
    /**
     * @brief Helper function to retrieve the integer value of a TemperatureSource enum.
     */
    static constexpr int use(const TemperatureSource e) { return static_cast<int>(e); }

    /**
     * @brief Configuration settings for the device measurements.
     *
     * This structure encapsulates user-configurable settings for both temperature
     * and pressure measurements, allowing control over sampling rate, precision,
     * temperature source, and preset profiles tailored for specific use cases.
     */
    struct Settings {
        /// Sampling rate for temperature measurements
        SamplingRate temperature_sampling_rate;

        /// Precision level (oversampling rate) for temperature measurements
        Precision temperature_precision;

        /// Source for temperature measurements
        /// (ASIC for low power, MEMS for high precision)
        TemperatureSource temperature_source;

        /// Sampling rate for pressure measurements
        SamplingRate pressure_sampling_rate;

        /// Precision level (oversampling rate) for pressure measurements
        Precision pressure_precision;

        /**
         * @brief Predefined preset configurations for common use cases.
         */
        enum class Presets {
            DEFAULT,                      ///< Default configuration for general use
            LOW_POWER_WEATHER_STATION,    ///< Low-power mode optimized for weather
                                          ///< stations
            STANDARD_PRECISION_INDOOR_NAVIGATION,    ///< Standard precision mode for
                                                     ///< indoor navigation
            HIGH_PRECISION_SPORTS    ///< High precision mode for sports applications
        };

        /**
         * @brief Default constructor initializes settings with the DEFAULT preset.
         */
        Settings() : Settings(Presets::DEFAULT) {}

        /**
         * @brief Parameterized constructor for custom settings.
         *
         * @param tsr Temperature sampling rate.
         * @param tp Temperature precision.
         * @param tsc Temperature source.
         * @param psr Pressure sampling rate.
         * @param pp Pressure precision.
         */
        Settings(SamplingRate tsr = SamplingRate::SAMPLING_1HZ,
                 Precision tp = Precision::LOW_1X,
                 TemperatureSource tsc = TemperatureSource::MEMS_HIGH_PRECISION,
                 SamplingRate psr = SamplingRate::SAMPLING_1HZ,
                 Precision pp = Precision::LOW_2X)
            : temperature_sampling_rate(tsr), temperature_precision(tp),
              temperature_source(tsc), pressure_sampling_rate(psr),
              pressure_precision(pp) {}

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
                temperature_sampling_rate = SamplingRate::SAMPLING_1HZ;
                temperature_precision = Precision::LOW_1X;
                temperature_source = TemperatureSource::MEMS_HIGH_PRECISION;
                pressure_sampling_rate = SamplingRate::SAMPLING_1HZ;
                pressure_precision = Precision::LOW_2X;
                break;
            }
            case Presets::LOW_POWER_WEATHER_STATION: {
                temperature_sampling_rate = SamplingRate::SAMPLING_1HZ;
                temperature_precision = Precision::LOW_1X;
                temperature_source = TemperatureSource::MEMS_HIGH_PRECISION;
                pressure_sampling_rate = SamplingRate::SAMPLING_1HZ;
                pressure_precision = Precision::LOW_2X;
                break;
            }
            case Presets::STANDARD_PRECISION_INDOOR_NAVIGATION: {
                temperature_sampling_rate = SamplingRate::SAMPLING_2HZ;
                temperature_precision = Precision::LOW_1X;
                temperature_source = TemperatureSource::MEMS_HIGH_PRECISION;
                pressure_sampling_rate = SamplingRate::SAMPLING_2HZ;
                pressure_precision = Precision::STANDARD_16X;
                break;
            }
            case Presets::HIGH_PRECISION_SPORTS: {
                temperature_sampling_rate = SamplingRate::SAMPLING_4HZ;
                temperature_precision = Precision::LOW_1X;
                temperature_source = TemperatureSource::MEMS_HIGH_PRECISION;
                pressure_sampling_rate = SamplingRate::SAMPLING_4HZ;
                pressure_precision = Precision::HIGH_64X;
                break;
            }
            default: break;
            }
        }
    };

public:
    // MARK: Constants (public)

    /**
     * @brief Returns the measurement time in milliseconds for the specified precision.
     *
     * This function provides an approximate measurement duration, in milliseconds,
     * based on the specified oversampling precision. Higher precision typically results
     * in longer measurement times.
     *
     * @param precision The oversampling precision level, defined in the `Precision`
     * enum.
     * @return Measurement time in milliseconds for the given precision setting.
     */
    static inline uint16_t getMeasurementTimeFor(const Precision precision) {
        switch (precision) {
        case Precision::LOW_1X: return 4;           ///< 1x oversampling, ~4 ms
        case Precision::LOW_2X: return 6;           ///< 2x oversampling, ~6 ms
        case Precision::LOW_4X: return 9;           ///< 4x oversampling, ~9 ms
        case Precision::LOW_8X: return 15;          ///< 8x oversampling, ~15 ms
        case Precision::STANDARD_16X: return 28;    ///< 16x oversampling, ~28 ms
        case Precision::HIGH_32X: return 54;        ///< 32x oversampling, ~54 ms
        case Precision::HIGH_64X: return 105;       ///< 64x oversampling, ~105 ms
        case Precision::HIGH_128X: return 207;      ///< 128x oversampling, ~207 ms
        default: return 0;                          ///< Invalid precision setting
        }
    }

private:
    // MARK: Constants (private)

    /**
     * @brief Returns the scale factor for temperature and pressure compensation.
     *
     * This function provides the scale factor used in the compensation calculation
     * for temperature and pressure measurements based on the oversampling precision.
     * Each precision level has a predefined scale factor, which enhances measurement
     * accuracy by appropriately adjusting raw sensor data.
     *
     * @param precision The precision level (oversampling) defined in the `Precision`
     * enum.
     * @return The scale factor corresponding to the specified precision level.
     */
    static inline float getScaleFactorFor(const Precision precision) {
        switch (precision) {
        case Precision::LOW_1X: return 524288.f;          ///< 1x oversampling
        case Precision::LOW_2X: return 1572864.f;         ///< 2x oversampling
        case Precision::LOW_4X: return 3670016.f;         ///< 4x oversampling
        case Precision::LOW_8X: return 7864320.f;         ///< 8x oversampling
        case Precision::STANDARD_16X: return 253952.f;    ///< 16x oversampling
        case Precision::HIGH_32X: return 516096.f;        ///< 32x oversampling
        case Precision::HIGH_64X: return 1040384.f;       ///< 64x oversampling
        case Precision::HIGH_128X: return 2088960.f;      ///< 128x oversampling
        default: return 0;                                ///< Invalid precision
        }
    }

    /**
     * @brief Genuine product ID for the device.
     *
     * This constant stores the expected product ID value for the device. It is
     * used during initialization to confirm the device type and ensure compatibility
     * with the driver.
     */
    static const uint8_t GENUINE_PRODUCT_ID = 0x10;

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
     * - `TEMP_BUSY`: A temperature measurement is in progress.
     * - `TEMP_COMPLETE`: Temperature measurement completed successfully.
     * - `TEMP_ERROR`: An error occurred during temperature measurement.
     * - `PRES_BUSY`: A pressure measurement is in progress.
     * - `PRES_COMPLETE`: Pressure measurement completed successfully.
     * - `PRES_ERROR`: An error occurred during pressure measurement.
     * - `AVAILABLE`: Data is ready to be read.
     */
    enum class State : int {
        WAIT_SETUP,       ///< Waiting for setup to complete.
        WAIT_BEGIN,       ///< Waiting to begin operation.
        IDLE,             ///< Device is idle and ready for a new measurement.
        TEMP_BUSY,        ///< Temperature measurement in progress.
        TEMP_COMPLETE,    ///< Temperature measurement successful.
        TEMP_ERROR,       ///< Error during temperature measurement.
        PRES_BUSY,        ///< Pressure measurement in progress.
        PRES_COMPLETE,    ///< Pressure measurement successful.
        PRES_ERROR,       ///< Error during pressure measurement.
        AVAILABLE         ///< Data is ready for reading.
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
     *
     * This enumeration defines all the register addresses used for configuring and
     * operating the device. Registers include those for reading raw data,
     * configuring sampling rates and precision, managing interrupts, and storing
     * calibration coefficients.
     *
     * Key Register Groups:
     * - **Pressure and Temperature Data**: PRS_B2, PRS_B1, PRS_B0, TMP_B2, TMP_B1,
     * TMP_B0.
     * - **Configuration**: PRS_CFG, TMP_CFG, MEAS_CFG, CFG_REG.
     * - **Status and Interrupts**: INT_STS, FIFO_STS.
     * - **Calibration**: Coefficient registers (e.g., C0_MSB, C10_MSB).
     * - **Control**: RESET for device resets and FIFO management.
     */
    enum class Register : uint8_t {
        PRS_B2 = 0x00,     ///< MSB of pressure data
        PRS_B1 = 0x01,     ///< Middle byte of pressure data
        PRS_B0 = 0x02,     ///< LSB of pressure data
        TMP_B2 = 0x03,     ///< MSB of temperature data
        TMP_B1 = 0x04,     ///< Middle byte of temperature data
        TMP_B0 = 0x05,     ///< LSB of temperature data
        PRS_CFG = 0x06,    ///< Configures pressure measurement rate and oversampling
        TMP_CFG = 0x07,    ///< Configures temperature measurement rate and oversampling
        MEAS_CFG = 0x08,    ///< Sets operating modes (e.g., standby, one-shot)
        CFG_REG = 0x09,     ///< Configures interrupts, bit shifts, and FIFO settings
        INT_STS = 0x0A,    ///< Status of interrupts for temperature, pressure, and FIFO
        FIFO_STS = 0x0B,           ///< Flags indicating FIFO full/empty status
        RESET = 0x0C,              ///< Commands for soft reset and FIFO flush
        PRODUCT_ID = 0x0D,         ///< Identifies product type and revision
        C0_MSB = 0x10,             ///< MSB of calibration coefficient C0
        C0_LSB_C1_MSB = 0x11,      ///< LSB of C0 and MSB of C1 combined
        C1_LSB = 0x12,             ///< LSB of calibration coefficient C1
        C00_MSB = 0x13,            ///< MSB of calibration coefficient C00
        C00_MID = 0x14,            ///< Middle byte of calibration coefficient C00
        C00_LSB_C10_MSB = 0x15,    ///< LSB of C00 and MSB of C10 combined
        C10_MID = 0x16,            ///< Middle byte of calibration coefficient C10
        C10_LSB = 0x17,            ///< LSB of calibration coefficient C10
        C01_MSB = 0x18,            ///< MSB of calibration coefficient C01
        C01_LSB = 0x19,            ///< LSB of calibration coefficient C01
        C11_MSB = 0x1A,            ///< MSB of calibration coefficient C11
        C11_LSB = 0x1B,            ///< LSB of calibration coefficient C11
        C20_MSB = 0x1C,            ///< MSB of calibration coefficient C20
        C20_LSB = 0x1D,            ///< LSB of calibration coefficient C20
        C21_MSB = 0x1E,            ///< MSB of calibration coefficient C21
        C21_LSB = 0x1F,            ///< LSB of calibration coefficient C21
        C30_MSB = 0x20,            ///< MSB of calibration coefficient C30
        C30_LSB = 0x21,            ///< LSB of calibration coefficient C30
        COEF_SRCE = 0x28           ///< Source of calibration coefficients
    };
    /**
     * @brief Helper function to retrieve the integer value of a `Register` enum.
     */
    static constexpr uint8_t use(const Register e) { return static_cast<uint8_t>(e); }

    // Pressure Configuration Register (PRS_CFG)
    /**
     * @brief Configuration for the pressure measurement rate and oversampling.
     *
     * Defines bit positions in the PRS_CFG register for controlling
     * the pressure measurement rate and precision.
     */
    enum class PRS_CFG : uint8_t {
        PM_RATE2 = 6,    // Pressure measurement rate bit 2
        PM_RATE1 = 5,    // Pressure measurement rate bit 1
        PM_RATE0 = 4,    // Pressure measurement rate bit 0
        PM_PRC3 = 3,     // Pressure oversampling precision bit 3
        PM_PRC2 = 2,     // Pressure oversampling precision bit 2
        PM_PRC1 = 1,     // Pressure oversampling precision bit 1
        PM_PRC0 = 0      // Pressure oversampling precision bit 0
    };
    /**
     * @brief Helper function to retrieve the integer value of a `PRS_CFG` enum.
     */
    static constexpr int use(const PRS_CFG e) { return static_cast<int>(e); }

    /**
     * @brief Configuration for the temperature measurement rate and oversampling.
     *
     * Defines bit positions in the TMP_CFG register for controlling
     * the temperature measurement rate, precision, and sensor selection.
     */
    enum class TMP_CFG : uint8_t {
        TMP_EXT = 7,      ///< External temperature sensor selection
                          ///< (1 = external, 0 =internal)
        TMP_RATE2 = 6,    ///< Temperature measurement rate bit 2
        TMP_RATE1 = 5,    ///< Temperature measurement rate bit 1
        TMP_RATE0 = 4,    ///< Temperature measurement rate bit 0
        TMP_PRC3 = 3,     ///< Temperature oversampling precision bit 3
        TMP_PRC2 = 2,     ///< Temperature oversampling precision bit 2
        TMP_PRC1 = 1,     ///< Temperature oversampling precision bit 1
        TMP_PRC0 = 0      ///< Temperature oversampling precision bit 0
    };
    /**
     * @brief Helper function to retrieve the integer value of a `TMP_CFG` enum.
     */
    static constexpr int use(const TMP_CFG e) { return static_cast<int>(e); }

    /**
     * @brief Configuration for measurement control and status.
     *
     * Defines bit positions in the MEAS_CFG register for controlling
     * the measurement mode and checking readiness of data and sensors.
     */
    enum class MEAS_CFG : uint8_t {
        COEF_RDY = 7,      ///< Calibration coefficients ready flag (1 = ready)
        SENSOR_RDY = 6,    ///< Sensor initialization complete flag (1 = ready)
        TMP_RDY = 5,       ///< Temperature measurement ready flag (1 = new data ready)
        PRS_RDY = 4,       ///< Pressure measurement ready flag (1 = new data ready)
        MEAS_CTRL2 = 2,    ///< Measurement mode control bit 2
        MEAS_CTRL1 = 1,    ///< Measurement mode control bit 1
        MEAS_CTRL0 = 0     ///< Measurement mode control bit 0
    };
    /**
     * @brief Helper function to retrieve the integer value of a MEAS_CFG enum.
     */
    static constexpr int use(const MEAS_CFG e) { return static_cast<int>(e); }

    /**
     * @brief Configuration for interrupts, bit shifts, and FIFO settings.
     *
     * Defines bit positions in the CFG_REG register for managing
     * interrupts, bit shifting, and FIFO functionality.
     */
    enum class CFG_REG : uint8_t {
        INT_HL = 7,      ///< Interrupt active level (1 = active high, 0 = active low)
        INT_FIFO = 6,    ///< Enable FIFO full interrupt (1 = enable)
        INT_TMP = 5,     ///< Enable temperature ready interrupt (1 = enable)
        INT_PRS = 4,     ///< Enable pressure ready interrupt (1 = enable)
        T_SHIFT = 3,     ///< Enable temperature result bit shift (1 = shift right)
        P_SHIFT = 2,     ///< Enable pressure result bit shift (1 = shift right)
        FIFO_EN = 1,     ///< Enable FIFO (1 = enable)
        SPI_MODE = 0     ///< Select SPI mode (1 = 3-wire, 0 = 4-wire)
    };
    /**
     * @brief Helper function to retrieve the integer value of a CFG_REG enum.
     */
    static constexpr int use(const CFG_REG e) { return static_cast<int>(e); }

    /**
     * @brief Interrupt status indicators.
     *
     * Defines bit positions in the INT_STS register for checking
     * the interrupt status of FIFO, temperature, and pressure measurements.
     */
    enum class INT_STS : uint8_t {
        INT_FIFO_FULL = 2,    ///< FIFO full interrupt status (1 = active)
        INT_TMP = 1,          ///< Temperature measurement interrupt status (1 = active)
        INT_PRS = 0           ///< Pressure measurement interrupt status (1 = active)
    };
    /**
     * @brief Helper function to retrieve the integer value of an INT_STS enum.
     */
    static constexpr int use(const INT_STS e) { return static_cast<int>(e); }

    /**
     * @brief FIFO status indicators.
     *
     * Defines bit positions in the FIFO_STS register for checking
     * whether the FIFO is full or empty.
     */
    enum class FIFO_STS : uint8_t {
        FIFO_FULL = 1,    ///< FIFO full flag (1 = full)
        FIFO_EMPTY = 0    ///< FIFO empty flag (1 = empty)
    };
    /**
     * @brief Helper function to retrieve the integer value of a FIFO_STS enum.
     */
    static constexpr int use(const FIFO_STS e) { return static_cast<int>(e); }

    /**
     * @brief Controls soft reset and FIFO flush commands.
     *
     * Defines bit positions in the RESET register for performing
     * soft resets and flushing the FIFO buffer.
     */
    enum class RESET : uint8_t {
        FIFO_FLUSH = 7,    ///< Command to flush FIFO (1 = flush)
        SOFT_RST3 = 3,     ///< Soft reset bit 3 of the 4-bit reset sequence
        SOFT_RST2 = 2,     ///< Soft reset bit 2 of the 4-bit reset sequence
        SOFT_RST1 = 1,     ///< Soft reset bit 1 of the 4-bit reset sequence
        SOFT_RST0 = 0      ///< Soft reset bit 0 of the 4-bit reset sequence
    };
    /**
     * @brief Helper function to retrieve the integer value of a RESET enum.
     */
    static constexpr int use(const RESET e) { return static_cast<int>(e); }

    /**
     * @brief Configuration for temperature coefficient source.
     *
     * Defines the source for temperature coefficients:
     * - ASIC for low power consumption.
     * - MEMS for high precision measurements.
     */
    enum class COEF_SRCE : uint8_t {
        TMP_COEF_SRCE = 7    ///< Temperature coefficient source (1 = MEMS, 0 = ASIC)
    };
    /**
     * @brief Helper function to retrieve the integer value of a COEF_SRCE enum.
     */
    static constexpr int use(const COEF_SRCE e) { return static_cast<int>(e); }

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

    /// Current operational mode
    OperationMode _operation_mode;

    /// Calibration coefficients used for compensation
    struct {
        int32_t c0, c1, c00, c10, c01, c11, c20, c21, c30;
        void setC0(const uint8_t c0_msb, const uint8_t c0_lsb_c1_msb) {
            c0 = twosComplement((c0_msb << 4) | ((c0_lsb_c1_msb >> 4) & 0x0F), 12);
        }
        inline void setC1(const uint8_t c0_lsb_c1_msb, const uint8_t c1_lsb) {
            c1 = twosComplement(((c0_lsb_c1_msb & 0x0F) << 8) | c1_lsb, 12);
        }
        inline void setC00(const uint8_t c00_msb, const uint8_t c00_mid,
                           const uint8_t c00_lsb_c10_msb) {
            c00 = twosComplement(
                (c00_msb << 12) | (c00_mid << 4) | ((c00_lsb_c10_msb >> 4) & 0x0F), 20);
        }
        inline void setC10(const uint8_t c00_lsb_c10_msb, const uint8_t c10_mid,
                           const uint8_t c10_lsb) {
            c10 = twosComplement(
                ((c00_lsb_c10_msb & 0x0F) << 16) | (c10_mid << 8) | c10_lsb, 20);
        }
        inline void setC01(const uint8_t c01_msb, const uint8_t c01_lsb) {
            c01 = twosComplement((c01_msb << 8) | c01_lsb, 16);
        }
        inline void setC11(const uint8_t c11_msb, const uint8_t c11_lsb) {
            c11 = twosComplement((c11_msb << 8) | c11_lsb, 16);
        }
        inline void setC20(const uint8_t c20_msb, const uint8_t c20_lsb) {
            c20 = twosComplement((c20_msb << 8) | c20_lsb, 16);
        }
        inline void setC21(const uint8_t c21_msb, const uint8_t c21_lsb) {
            c21 = twosComplement((c21_msb << 8) | c21_lsb, 16);
        }
        inline void setC30(const uint8_t c30_msb, const uint8_t c30_lsb) {
            c30 = twosComplement((c30_msb << 8) | c30_lsb, 16);
        }
    } _coef;

    /// Latest measured values
    struct {
        float t_raw_scaled;    ///< Scaled raw temperature data
        float temperature;     ///< Latest temperature in °C
        float p_raw_scaled;    ///< Scaled raw pressure data
        float pressure;        ///< Latest pressure in hPa
    } _values;

public:
    // MARK: Const/Destructor (public)

    /**
     * @brief Constructor for the device interface.
     *
     * Initializes the device object with default settings, state, and error
     * status.
     */
    DPS310()
        : _state(State::WAIT_SETUP), _error(Result::FAILED_UNKNOWN),
          _error_message { 0 }, _address(Address::PRIMARY),
          _settings(Settings(Settings::Presets::DEFAULT)),
          _operation_mode(OperationMode::STANDBY), _coef { 0 }, _values { 0 } {}

    /**
     * @brief Destructor for the device interface.
     *
     * Cleans up any resources or states before the object is destroyed.
     */
    ~DPS310() {}

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
     * @return The `DPS310::Result` indicating the error cause.
     */
    inline Result getError() const { return _error; }

    /**
     * @brief Sets a new error cause.
     *
     * Updates the internal error state and logs an appropriate
     * error message based on the cause.
     *
     * @param cause The `DPS310::Result` representing the error cause.
     */
    inline void setError(const Result cause) {
        _error = cause;
        switch (cause) {
        case Result::FAILED_BUSY:
            snprintf_(_error_message, sizeof(_error_message), "Error: DPS310 is busy");
            break;
        case Result::FAILED_NOT_RESPONDING:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: DPS310 is not responding");
            break;
        case Result::FAILED_UNKNOWN:
            snprintf_(_error_message, sizeof(_error_message),
                      "Error: Unknown issue with DPS310");
            break;
        default: break;
        }
    }

    /**
     * @brief Retrieves the current operation mode.
     *
     * Provides the current operating mode of the device.
     *
     * @return The `DPS310::OperationMode` representing the current mode.
     */
    inline OperationMode getOperationMode() const { return _operation_mode; }

    /**
     * @brief Sets the operation mode.
     *
     * Updates the device's operation mode to the specified value.
     *
     * @param mode The `DPS310::OperationMode` to set.
     */
    inline void setOperationMode(const OperationMode mode) { _operation_mode = mode; }

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
    inline bool available() { return in(DPS310::State::AVAILABLE); }

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
     * @return `DPS310::Result` indicating the success or failure of the request.
     */
    Result request();

    /**
     * @brief Read temperature and pressure data after a measurement request.
     *
     * Retrieves the temperature and pressure data measured by the device.
     * Ensure `request()` has been called before using this method.
     *
     * @param temperature Pointer to store the temperature value (°C).
     * @param pressure Pointer to store the pressure value (hPa).
     * @return `DPS310::Result` indicating the success or failure of the read operation.
     */
    Result read(float* const temperature, float* const pressure);

    /**
     * @brief Calculate altitude based on measured pressure and sea-level pressure.
     *
     * Computes the altitude using the barometric formula with the
     * provided sea-level pressure and the measured pressure.
     *
     * @param sealevel_pressure Reference sea-level pressure (hPa).
     * @return Calculated altitude (m).
     */
    inline float calcAltitude(const float sealevel_pressure) const {
        return 44330.0f * (1.0f - powf(_values.pressure / sealevel_pressure, 0.1903f));
    }

    /**
     * @brief Perform a software reset of the device.
     *
     * Resets the device to its default state without a hardware power cycle.
     *
     * @return `DPS310::Result` indicating the success or failure of the reset
     * operation.
     */
    Result softReset();

    /**
     * @brief Read the production and revision ID of the device.
     *
     * Retrieves the product ID and revision number to verify the presence of the device.
     *
     * @retval `DPS310::GENUINE_PRODUCT_ID` if the device is found and valid.
     * @retval `0` if the device is not found or invalid.
     */
    inline uint8_t readId() {
        uint8_t id;
        return (read(DPS310::Register::PRODUCT_ID, &id) == DPS310::Result::SUCCESS) ?
            id :
            0;
    }

private:
    // MARK: Specific utils (private)

    /**
     * @brief Apply saved pressure configurations from settings.
     *
     * Updates the device's pressure measurement settings based on the
     * current configuration stored in the `Settings` structure.
     *
     * @return `DPS310::Result` indicating the success or failure of the operation.
     */
    Result applyPressureSettings();

    /**
     * @brief Apply saved temperature configurations from settings.
     *
     * Updates the device's temperature measurement settings based on the
     * current configuration stored in the `Settings` structure.
     *
     * @return `DPS310::Result` indicating the success or failure of the operation.
     */
    Result applyTemperatureSettings();

    /**
     * @brief Apply the given operation mode.
     *
     * Configures the device to operate in the specified mode.
     *
     * @param mode The `OperationMode` to apply.
     * @return `DPS310::Result` indicating the success or failure of the operation.
     */
    Result applyOperationMode(const OperationMode mode);

    /**
     * @brief Read and update coefficient values.
     *
     * Reads the calibration coefficients from the device and updates
     * the internal data used for temperature and pressure compensation.
     *
     * @return `DPS310::Result` indicating the success or failure of the operation.
     */
    Result updateCoefficients();

private:
    // MARK: Common I2C utils (private)

    /**
     * @brief Read 8-bit data via I2C.
     *
     * Reads a single byte of data from the specified register address.
     *
     * @param reg Register address to read from.
     * @param dst Pointer to the variable where the read data will be stored.
     * @return A `DPS310::Result` indicating success or failure of the read operation.
     */
    Result read(const Register reg, uint8_t* const dst);

    /**
     * @brief Read 16-bit data via I2C.
     *
     * Reads 2 bytes of data from the specified register address.
     *
     * @param reg Register address to read from.
     * @param dst Pointer to the variable where the read data will be stored.
     * @return A `DPS310::Result` indicating success or failure of the read operation.
     */
    Result read(const Register reg, uint16_t* const dst);

    /**
     * @brief Write 8-bit or 16-bit data via I2C.
     *
     * Writes a single or two byte(s) of data to the specified register address.
     *
     * @param reg Register address to write to.
     * @param src Data byte to write.
     * @return A `DPS310::Result` indicating success or failure of the write operation.
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
    static inline bool hasPattern(const uint8_t target, const int shift,
                                     const int bits, const int width) {
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
 * @brief Logical NOT operator for `DPS310::Result`.
 *
 * Evaluates whether the provided result is not `DPS310::Result::SUCCESS`.
 *
 * @param result The `DPS310::Result` value to evaluate.
 * @return `true` if the result is not `DPS310::Result::SUCCESS`; otherwise, `false`.
 */
bool operator!(DPS310::Result result);

/**
 * @brief Logical AND operator for combining `DPS310::Result` values.
 *
 * Combines two `DPS310::Result` values. If both values are `DPS310::Result::SUCCESS`,
 * the result is `DPS310::Result::SUCCESS`; otherwise, the result is
 * `DPS310::Result::FAILED_UNKNOWN`.
 *
 * @param lhs The left-hand operand.
 * @param rhs The right-hand operand.
 * @return The combined `DPS310::Result` value.
 */
DPS310::Result operator&&(DPS310::Result lhs, DPS310::Result rhs);

/**
 * @brief Logical OR operator for combining `DPS310::Result` values.
 *
 * Combines two `DPS310::Result` values. If either value is `DPS310::Result::SUCCESS`,
 * the result is `DPS310::Result::SUCCESS`; otherwise, the result is
 * `DPS310::Result::FAILED_UNKNOWN`.
 *
 * @param lhs The left-hand operand.
 * @param rhs The right-hand operand.
 * @return The combined `DPS310::Result` value.
 */
DPS310::Result operator||(DPS310::Result lhs, DPS310::Result rhs);
