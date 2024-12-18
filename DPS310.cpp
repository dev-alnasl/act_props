// -*- coding:utf-8-unix -*-

#include "DPS310.hpp"

// MARK: Interfaces (public)

void DPS310::setup(const Address address, const Settings& settings) {
    setError(Result::FAILED_UNKNOWN);
    setAddress(address);
    setSettings(settings);
    set(State::WAIT_BEGIN);
}

void DPS310::begin() {
    if (not in(State::WAIT_BEGIN)) { end(); }
    Wire.begin();
    delay(50);    // Wait for device startup
    if (not(readId() == GENUINE_PRODUCT_ID)) { return; }
    if (not softReset()) { return; }
    if (not applyPressureSettings()) { return; }
    if (not applyTemperatureSettings()) { return; }
    if (not applyOperationMode(OperationMode::STANDBY)) { return; }
    set(State::IDLE);
}

void DPS310::update() {
    switch (_state) {
    case State::TEMP_BUSY: {
        uint8_t meas_cfg;
        if (not read(Register::MEAS_CFG, &meas_cfg)) { set(State::TEMP_ERROR); }
        if (hasBitSet(meas_cfg, use(MEAS_CFG::TMP_RDY))) { set(State::TEMP_COMPLETE); }
        break;
    }
    case State::TEMP_COMPLETE: {
        uint8_t temp_msb, temp_mid, temp_lsb;
        if (not read(Register::TMP_B2, &temp_msb)) { set(State::TEMP_ERROR); }
        if (not read(Register::TMP_B1, &temp_mid)) { set(State::TEMP_ERROR); }
        if (not read(Register::TMP_B0, &temp_lsb)) { set(State::TEMP_ERROR); }

        float tmp_reg =
            twosComplement((temp_msb << 16) | (temp_mid << 8) | temp_lsb, 24);
        _values.t_raw_scaled =
            tmp_reg / getScaleFactorFor(_settings.temperature_precision);

        _values.temperature = 0.5f * _coef.c0 + _coef.c1 * _values.t_raw_scaled;

        // Next, measure pressure
        set(State::PRES_BUSY);
        if (not applyOperationMode(OperationMode::ONE_SHOT_PRESSURE)) {
            set(State::PRES_ERROR);
        }
        break;
    }
    case State::TEMP_ERROR: {
        set(State::IDLE);
        break;
    }
    case State::PRES_BUSY: {
        uint8_t meas_cfg;
        if (not read(Register::MEAS_CFG, &meas_cfg)) { set(State::PRES_ERROR); }
        if (hasBitSet(meas_cfg, use(MEAS_CFG::PRS_RDY))) { set(State::PRES_COMPLETE); }
        break;
    }
    case State::PRES_COMPLETE: {
        uint8_t pres_msb, pres_mid, pres_lsb;
        if (not read(Register::PRS_B2, &pres_msb)) { set(State::PRES_ERROR); }
        if (not read(Register::PRS_B1, &pres_mid)) { set(State::PRES_ERROR); }
        if (not read(Register::PRS_B0, &pres_lsb)) { set(State::PRES_ERROR); }

        float prs_reg =
            twosComplement((pres_msb << 16) | (pres_mid << 8) | pres_lsb, 24);
        _values.p_raw_scaled =
            prs_reg / getScaleFactorFor(_settings.pressure_precision);

        float a = _coef.c00;
        float b = _values.p_raw_scaled
            * (_coef.c10
               + _values.p_raw_scaled * (_coef.c20 + _values.p_raw_scaled * _coef.c30));
        float c = _values.t_raw_scaled
            * (_coef.c01
               + _values.p_raw_scaled * (_coef.c11 + _values.p_raw_scaled * _coef.c21));
        _values.pressure = (a + b + c) / 100.0f;

        set(State::AVAILABLE);
        break;
    }
    case State::PRES_ERROR: {
        set(State::IDLE);
        break;
    }
    default: break;
    }
}

void DPS310::end() {
    if (in(State::WAIT_BEGIN)) { return; }
    Wire.end();
    set(State::WAIT_BEGIN);
}

DPS310::Result DPS310::request() {
    if (not in(State::IDLE)) {
        setError(Result::FAILED_BUSY);
        return _error;
    }
    // Starting with a temperature measurement
    if (not applyOperationMode(OperationMode::ONE_SHOT_TEMPERATURE)) { return _error; }
    set(State::TEMP_BUSY);
    return Result::SUCCESS;
}

DPS310::Result DPS310::read(float* const temperature, float* const pressure) {
    if (not in(State::AVAILABLE)) {
        setError(Result::FAILED_BUSY);
        return _error;
    }
    *temperature = _values.temperature;
    *pressure = _values.pressure;
    set(State::IDLE);
    return Result::SUCCESS;
}

DPS310::Result DPS310::softReset() {
    uint8_t meas_cfg;
    if (not write(Register::RESET, 0x09)) { return _error; }
    do {
        delay(12);
        if (not read(Register::MEAS_CFG, &meas_cfg)) { return _error; }
    } while (not hasBitSet(meas_cfg, use(MEAS_CFG::SENSOR_RDY)));
    return Result::SUCCESS;
}

// MARK: Specific utils (private)

DPS310::Result DPS310::applyPressureSettings() {
    uint8_t prs_cfg, cfg_reg;
    // PRS_CFG
    if (not read(Register::PRS_CFG, &prs_cfg)) { return _error; }
    setPattern(&prs_cfg, use(PRS_CFG::PM_RATE0), use(_settings.pressure_sampling_rate),
               3);
    setPattern(&prs_cfg, use(PRS_CFG::PM_PRC0), use(_settings.pressure_precision), 3);
    if (not write(Register::PRS_CFG, prs_cfg)) { return _error; }
    // CFG_REG
    if (not read(Register::CFG_REG, &cfg_reg)) { return _error; }
    setBit(&cfg_reg, use(CFG_REG::P_SHIFT),
           use(_settings.pressure_precision) > use(Precision::LOW_8X) ? 1 : 0);
    if (not write(Register::CFG_REG, cfg_reg)) { return _error; }
    return Result::SUCCESS;
}

DPS310::Result DPS310::applyTemperatureSettings() {
    uint8_t tmp_cfg, cfg_reg;
    // TMP_CFG
    if (not read(Register::TMP_CFG, &tmp_cfg)) { return _error; }
    setBit(&tmp_cfg, use(TMP_CFG::TMP_EXT), use(_settings.temperature_source));
    setPattern(&tmp_cfg, use(TMP_CFG::TMP_RATE0),
               use(_settings.temperature_sampling_rate), 3);
    setPattern(&tmp_cfg, use(TMP_CFG::TMP_PRC0), use(_settings.temperature_precision),
               3);
    if (not write(Register::TMP_CFG, tmp_cfg)) { return _error; }
    // CFG_REG
    if (not read(Register::CFG_REG, &cfg_reg)) { return _error; }
    setBit(&cfg_reg, use(CFG_REG::T_SHIFT),
           use(_settings.temperature_precision) > use(Precision::LOW_8X) ? 1 : 0);
    if (not write(Register::CFG_REG, cfg_reg)) { return _error; }
    if (not updateCoefficients()) { return _error; }
    return Result::SUCCESS;
}

DPS310::Result DPS310::applyOperationMode(const OperationMode mode) {
    uint8_t meas_cfg;
    if (not read(Register::MEAS_CFG, &meas_cfg)) { return _error; }
    setPattern(&meas_cfg, use(MEAS_CFG::MEAS_CTRL0), use(mode), 3);
    if (not write(Register::MEAS_CFG, meas_cfg)) { return _error; }
    return Result::SUCCESS;
}

DPS310::Result DPS310::updateCoefficients() {
    // Set coefficient source
    uint8_t coef_srce, meas_cfg;
    if (not read(Register::COEF_SRCE, &coef_srce)) { return _error; }
    setBit(&coef_srce, use(COEF_SRCE::TMP_COEF_SRCE),
           use(_settings.temperature_source));
    if (not write(Register::COEF_SRCE, coef_srce)) { return _error; }
    // Wait for ready
    do {
        delay(1);
        if (not read(Register::MEAS_CFG, &meas_cfg)) { return _error; }
    } while (not hasBitSet(meas_cfg, use(MEAS_CFG::COEF_RDY)));
    // Read coefficients
    uint8_t c0_msb, c0_lsb_c1_msb, c1_lsb, c00_msb, c00_mid, c00_lsb_c10_msb, c10_mid,
        c10_lsb, c01_msb, c01_lsb, c11_msb, c11_lsb, c20_msb, c20_lsb, c21_msb, c21_lsb,
        c30_msb, c30_lsb;
    if (not(read(Register::C0_MSB, &c0_msb)
            and read(Register::C0_LSB_C1_MSB, &c0_lsb_c1_msb)
            and read(Register::C1_LSB, &c1_lsb) and read(Register::C00_MSB, &c00_msb)
            and read(Register::C00_MID, &c00_mid)
            and read(Register::C00_LSB_C10_MSB, &c00_lsb_c10_msb)
            and read(Register::C10_MID, &c10_mid) and read(Register::C10_LSB, &c10_lsb)
            and read(Register::C01_MSB, &c01_msb) and read(Register::C01_LSB, &c01_lsb)
            and read(Register::C11_MSB, &c11_msb) and read(Register::C11_LSB, &c11_lsb)
            and read(Register::C20_MSB, &c20_msb) and read(Register::C20_LSB, &c20_lsb)
            and read(Register::C21_MSB, &c21_msb) and read(Register::C21_LSB, &c21_lsb)
            and read(Register::C30_MSB, &c30_msb)
            and read(Register::C30_LSB, &c30_lsb))) {
        return _error;
    }
    _coef.setC0(c0_msb, c0_lsb_c1_msb);
    _coef.setC1(c0_lsb_c1_msb, c1_lsb);
    _coef.setC00(c00_msb, c00_mid, c00_lsb_c10_msb);
    _coef.setC10(c00_lsb_c10_msb, c10_mid, c10_lsb);
    _coef.setC01(c01_msb, c01_lsb);
    _coef.setC11(c11_msb, c11_lsb);
    _coef.setC11(c11_msb, c11_lsb);
    _coef.setC20(c20_msb, c20_lsb);
    _coef.setC21(c21_msb, c21_lsb);
    _coef.setC30(c30_msb, c30_lsb);
    return Result::SUCCESS;
}

// MARK: Common I2C utils (private)

DPS310::Result DPS310::read(const Register reg, uint8_t* const dst) {
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

DPS310::Result DPS310::read(const Register reg, uint16_t* const dst) {
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

DPS310::Result DPS310::write(const Register reg, const int src) {
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

bool operator!(DPS310::Result result) {
    return result != DPS310::Result::SUCCESS;
}
DPS310::DPS310::Result operator&&(DPS310::Result lhs, DPS310::Result rhs) {
    return (lhs == DPS310::Result::SUCCESS && rhs == DPS310::Result::SUCCESS) ?
        DPS310::Result::SUCCESS :
        DPS310::Result::FAILED_UNKNOWN;
}
DPS310::Result operator||(DPS310::Result lhs, DPS310::Result rhs) {
    return (lhs == DPS310::Result::SUCCESS || rhs == DPS310::Result::SUCCESS) ?
        DPS310::Result::SUCCESS :
        DPS310::Result::FAILED_UNKNOWN;
}
