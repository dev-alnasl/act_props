// -*- coding:utf-8-unix -*-

#include "ADS1x1x.hpp"

// MARK: Interfaces (public)

void ADS1x1x::setup(const Address address, const DeviceType device_type,
                    const Settings& settings) {
    setError(Result::FAILED_UNKNOWN);
    setAddress(address);
    setDeviceType(device_type);
    setSettings(settings);
    set(State::WAIT_BEGIN);
    _latest_request_time = 0;
}

void ADS1x1x::begin() {
    if (not in(State::WAIT_BEGIN)) { end(); }
    Wire.begin();

    if (not applyFullScaleRange()) { return; }
    if (not applyDataRate()) { return; }

    uint16_t config_reg;
    if (not read(Register::CONFIG_REGISTER, &config_reg)) { return; }
    setBit(&config_reg, use(CONFIG_REGISTER::CONF_MODE), 1);    // Single-shot
    if (not write(Register::CONFIG_REGISTER, config_reg)) { return; }

    set(State::IDLE);
    _latest_request_time = 0;
}

void ADS1x1x::update() {
    switch (_state) {
    case State::BUSY: {
        if (millis() - _latest_request_time
            >= getConversionDelay(_settings.data_rate)) {
            set(State::COMPLETE);
            _latest_request_time = 0;
        }
        break;
    }
    case State::COMPLETE: {
        uint16_t conv_reg;
        if (not read(Register::CONVERSION_REGISTER, &conv_reg)) { set(State::ERROR); }
        switch (_device_type) {
        case DeviceType::ADS101x: {
            // 12bit
            _values.raw = conv_reg >> 4;
            _values.voltage = _values.raw * use(_settings.full_scale_range) / 0x7FF;
            break;
        }
        case DeviceType::ADS111x: {
            // 16bit
            _values.raw = conv_reg;
            _values.voltage = _values.raw * use(_settings.full_scale_range) / 0x7FFF;
            break;
        }
        default: break;
        }
        set(State::AVAILABLE);
        break;
    }
    case State::ERROR: {
        set(State::IDLE);
        break;
    }
    default: break;
    }
}

void ADS1x1x::end() {
    if (in(State::WAIT_BEGIN)) { return; }
    Wire.end();
    set(State::WAIT_BEGIN);
}

ADS1x1x::Result ADS1x1x::request(ChannelConfig channel_config) {
    if (not in(State::IDLE)) {
        setError(Result::FAILED_BUSY);
        return _error;
    }

    uint16_t config_reg;
    if (not read(Register::CONFIG_REGISTER, &config_reg)) { return _error; }
    setBit(&config_reg, use(CONFIG_REGISTER::CONF_OS), 1);
    switch (channel_config) {
    case ChannelConfig::AIN0_AIN1: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b000, 3);
        break;
    }
    case ChannelConfig::AIN0_AIN3: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b001, 3);
        break;
    }
    case ChannelConfig::AIN1_AIN3: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b010, 3);
        break;
    }
    case ChannelConfig::AIN2_AIN3: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b011, 3);
        break;
    }
    case ChannelConfig::AIN0_GND: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b100, 3);
        break;
    }
    case ChannelConfig::AIN1_GND: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b101, 3);
        break;
    }
    case ChannelConfig::AIN2_GND: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b110, 3);
        break;
    }
    case ChannelConfig::AIN3_GND: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_MUX0), 0b111, 3);
        break;
    }
    default: break;
    }
    if (not write(Register::CONFIG_REGISTER, config_reg)) { return _error; }
    set(State::BUSY);
    _latest_request_time = millis();
    return ADS1x1x::Result::SUCCESS;
}

ADS1x1x::Result ADS1x1x::read(uint16_t* const voltage) {
    if (not in(State::AVAILABLE)) {
        setError(Result::FAILED_BUSY);
        return _error;
    }
    *voltage = _values.voltage;
    set(State::IDLE);
    return Result::SUCCESS;
}

// MARK: Specific utils (private)

ADS1x1x::Result ADS1x1x::applyFullScaleRange() {
    uint16_t config_reg;
    if (not read(Register::CONFIG_REGISTER, &config_reg)) { return _error; }
    switch (_settings.full_scale_range) {
    case FullScaleRange::FSR_6144mV: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b000, 3);
        break;
    }
    case FullScaleRange::FSR_4096mV: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b001, 3);
        break;
    }
    case FullScaleRange::FSR_2048mV: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b010, 3);
        break;
    }
    case FullScaleRange::FSR_1024mV: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b011, 3);
        break;
    }
    case FullScaleRange::FSR_0512mV: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b100, 3);
        break;
    }
    case FullScaleRange::FSR_0256mV: {
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b101, 3);
        break;
    }
    default: {
        // Default FSR is 2048mV in both ADS101x and ADS111x
        setPattern(&config_reg, use(CONFIG_REGISTER::CONF_PGA0), 0b010, 3);
        break;
    }
    }
    if (not write(Register::CONFIG_REGISTER, config_reg)) { return _error; }
    return Result::SUCCESS;
}

ADS1x1x::Result ADS1x1x::applyDataRate() {
    uint16_t config_reg;
    if (not read(Register::CONFIG_REGISTER, &config_reg)) { return _error; }
    switch (_device_type) {
    case DeviceType::ADS101x: {
        switch (_settings.data_rate) {
        case DataRate::DR_0128SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b000, 3);
            break;
        }
        case DataRate::DR_0250SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b001, 3);
            break;
        }
        case DataRate::DR_0490SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b010, 3);
            break;
        }
        case DataRate::DR_0920SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b011, 3);
            break;
        }
        case DataRate::DR_1600SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b100, 3);
            break;
        }
        case DataRate::DR_2400SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b101, 3);
            break;
        }
        case DataRate::DR_3300SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b110, 3);
            break;
        }
        default: {    // Default for ADS101x is 1600 SPS
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b100, 3);
            break;
        }
        }
        break;
    }
    case DeviceType::ADS111x: {
        switch (_settings.data_rate) {
        case DataRate::DR_0008SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b000, 3);
            break;
        }
        case DataRate::DR_0016SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b001, 3);
            break;
        }
        case DataRate::DR_0032SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b010, 3);
            break;
        }
        case DataRate::DR_0064SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b011, 3);
            break;
        }
        case DataRate::DR_0128SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b100, 3);
            break;
        }
        case DataRate::DR_0250SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b101, 3);
            break;
        }
        case DataRate::DR_0475SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b110, 3);
            break;
        }
        case DataRate::DR_0860SPS: {
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b110, 3);
            break;
        }
        default: {    // Default for ADS111x is 128 SPS
            setPattern(&config_reg, use(CONFIG_REGISTER::CONF_DR0), 0b100, 3);
            break;
        }
        }
        break;
    }
    }
    if (not write(Register::CONFIG_REGISTER, config_reg)) { return _error; }
    return Result::SUCCESS;
}

// MARK: Common I2C utils (private)

ADS1x1x::Result ADS1x1x::read(const Register reg, uint8_t* const dst) {
    if (auto&& writer = Wire.get_writer(use(_address))) {
        writer << use(reg);
    } else {
        setError(Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    if (auto&& reader = Wire.get_reader(use(_address), 1)) {
        reader >> *dst;
    } else {
        setError(Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    return ADS1x1x::Result::SUCCESS;
}

ADS1x1x::Result ADS1x1x::read(const ADS1x1x::Register reg, uint16_t* const dst) {
    if (auto&& writer = Wire.get_writer(use(_address))) {
        writer << use(reg);
    } else {
        setError(Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    if (auto&& reader = Wire.get_reader(use(_address), 2)) {
        uint8_t byte1, byte2;
        reader >> byte1;
        reader >> byte2;
        *dst = (byte1 << 8) | byte2;
    } else {
        setError(Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    return Result::SUCCESS;
}

ADS1x1x::Result ADS1x1x::write(const Register reg, const int src) {
    if (auto&& writer = Wire.get_writer(use(_address))) {
        writer << use(reg);
        if (src <= 0xFF) {
            writer << src;
        } else {
            writer << ((src >> 8) & 0xFF);
            writer << (src & 0xFF);
        }
    } else {
        setError(Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    return Result::SUCCESS;
}

// MARK: Operators for results (global)

bool operator!(ADS1x1x::Result result) {
    return result != ADS1x1x::Result::SUCCESS;
}
ADS1x1x::ADS1x1x::Result operator&&(ADS1x1x::Result lhs, ADS1x1x::Result rhs) {
    return (lhs == ADS1x1x::Result::SUCCESS && rhs == ADS1x1x::Result::SUCCESS) ?
        ADS1x1x::Result::SUCCESS :
        ADS1x1x::Result::FAILED_UNKNOWN;
}
ADS1x1x::Result operator||(ADS1x1x::Result lhs, ADS1x1x::Result rhs) {
    return (lhs == ADS1x1x::Result::SUCCESS || rhs == ADS1x1x::Result::SUCCESS) ?
        ADS1x1x::Result::SUCCESS :
        ADS1x1x::Result::FAILED_UNKNOWN;
}
