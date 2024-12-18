// -*- coding:utf-8-unix -*-

#include "Template.hpp"

// MARK: Interfaces (public)

void _DEVICE_::setup(const Address address, const Settings& settings) {
    setError(Result::FAILED_UNKNOWN);
    setAddress(address);
    setSettings(settings);
    set(State::WAIT_BEGIN);
}

void _DEVICE_::begin() {
    if (not in(State::WAIT_BEGIN)) { end(); }
    Wire.begin();
    delay(50);    // Wait for device startup
    if (not softReset()) { return; }
    if (not applySomeSettings()) { return; }
    set(State::IDLE);
}

void _DEVICE_::update() {
    switch (_state) {
    case State::BUSY: {
        if (1) { set(State::COMPLETE); }
        break;
    }
    case State::COMPLETE: {
        if (0) { set(State::ERROR); }

        _values.value = 1;
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

void _DEVICE_::end() {
    if (in(State::WAIT_BEGIN)) { return; }
    Wire.end();
    set(State::WAIT_BEGIN);
}

_DEVICE_::Result _DEVICE_::request() {
    if (not in(State::IDLE)) {
        setError(Result::FAILED_BUSY);
        return _error;
    }
    set(State::BUSY);
    return Result::SUCCESS;
}

_DEVICE_::Result _DEVICE_::read(int32_t* const value) {
    if (not in(State::AVAILABLE)) {
        setError(Result::FAILED_BUSY);
        return _error;
    }
    *value = _values.value;
    set(State::IDLE);
    return Result::SUCCESS;
}

_DEVICE_::Result _DEVICE_::softReset() {
    return Result::SUCCESS;
}

// MARK: Specific utils (private)

_DEVICE_::Result _DEVICE_::applySomeSettings() {
    use(_settings.some_parameter);
    return Result::SUCCESS;
}

// MARK: Common I2C utils (private)

_DEVICE_::Result _DEVICE_::read(const Register reg, uint8_t* const dst) {
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
    return Result::SUCCESS;
}

_DEVICE_::Result _DEVICE_::read(const Register reg, uint16_t* const dst) {
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

_DEVICE_::Result _DEVICE_::write(const Register reg, const int src) {
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

bool operator!(_DEVICE_::Result result) {
    return result != _DEVICE_::Result::SUCCESS;
}
_DEVICE_::_DEVICE_::Result operator&&(_DEVICE_::Result lhs, _DEVICE_::Result rhs) {
    return (lhs == _DEVICE_::Result::SUCCESS && rhs == _DEVICE_::Result::SUCCESS) ?
        _DEVICE_::Result::SUCCESS :
        _DEVICE_::Result::FAILED_UNKNOWN;
}
_DEVICE_::Result operator||(_DEVICE_::Result lhs, _DEVICE_::Result rhs) {
    return (lhs == _DEVICE_::Result::SUCCESS || rhs == _DEVICE_::Result::SUCCESS) ?
        _DEVICE_::Result::SUCCESS :
        _DEVICE_::Result::FAILED_UNKNOWN;
}
