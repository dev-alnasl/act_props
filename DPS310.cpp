// -*- coding:utf-8-unix -*-

#include "DPS310.hpp"

// MARK: Interfaces (public)

void DPS310::setup(const Address address, const Settings& settings) {
    setError(DPS310::Result::FAILED_UNKNOWN);
    setAddress(address);
    setSettings(settings);
    setAs(DPS310::State::WAIT_BEGIN);
}

void DPS310::begin() {
    if (not isIn(DPS310::State::WAIT_BEGIN)) { end(); }
    Wire.begin();
    delay(50);    // Wait for device startup
    if (not(readId() == GENUINE_PRODUCT_ID)) { return; }
    if (not softReset()) { return; }
    if (not applyPressureSettings()) { return; }
    if (not applyTemperatureSettings()) { return; }
    if (not applyOperationMode(DPS310::OperationMode::STANDBY)) { return; }
    setAs(DPS310::State::IDLE);
}

void DPS310::update() {
    switch (_state) {
    case DPS310::State::TEMP_BUSY: {
        uint8_t meas_cfg;
        if (not readByte(DPS310::Register::MEAS_CFG, &meas_cfg)) {
            setAs(DPS310::State::TEMP_ERROR);
        }
        if (bitIsSet(meas_cfg, use(DPS310::MEAS_CFG::TMP_RDY))) {
            setAs(DPS310::State::TEMP_COMPLETE);
        }
        break;
    }
    case DPS310::State::TEMP_COMPLETE: {
        uint8_t temp_msb, temp_mid, temp_lsb;
        if (not readByte(DPS310::Register::TMP_B2, &temp_msb)) {
            setAs(DPS310::State::TEMP_ERROR);
        }
        if (not readByte(DPS310::Register::TMP_B1, &temp_mid)) {
            setAs(DPS310::State::TEMP_ERROR);
        }
        if (not readByte(DPS310::Register::TMP_B0, &temp_lsb)) {
            setAs(DPS310::State::TEMP_ERROR);
        }

        float tmp_reg =
            twosComplement((temp_msb << 16) | (temp_mid << 8) | temp_lsb, 24);
        _values.t_raw_scaled =
            tmp_reg / getScaleFactorFor(_settings.temperature_precision);

        _values.temperature = 0.5f * _coef.c0 + _coef.c1 * _values.t_raw_scaled;

        // Next, measure pressure
        setAs(DPS310::State::PRES_BUSY);
        if (not applyOperationMode(DPS310::OperationMode::ONE_SHOT_PRESSURE)) {
            setAs(DPS310::State::PRES_ERROR);
        }
        break;
    }
    case DPS310::State::TEMP_ERROR: {
        setAs(DPS310::State::IDLE);
        break;
    }
    case DPS310::State::PRES_BUSY: {
        uint8_t meas_cfg;
        if (not readByte(DPS310::Register::MEAS_CFG, &meas_cfg)) {
            setAs(DPS310::State::PRES_ERROR);
        }
        if (bitIsSet(meas_cfg, use(DPS310::MEAS_CFG::PRS_RDY))) {
            setAs(DPS310::State::PRES_COMPLETE);
        }
        break;
    }
    case DPS310::State::PRES_COMPLETE: {
        uint8_t pres_msb, pres_mid, pres_lsb;
        if (not readByte(DPS310::Register::PRS_B2, &pres_msb)) {
            setAs(DPS310::State::PRES_ERROR);
        }
        if (not readByte(DPS310::Register::PRS_B1, &pres_mid)) {
            setAs(DPS310::State::PRES_ERROR);
        }
        if (not readByte(DPS310::Register::PRS_B0, &pres_lsb)) {
            setAs(DPS310::State::PRES_ERROR);
        }

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

        setAs(DPS310::State::AVAILABLE);
        break;
    }
    case DPS310::State::PRES_ERROR: {
        setAs(DPS310::State::IDLE);
        break;
    }
    default: break;
    }
}

void DPS310::end() {
    if (isIn(DPS310::State::WAIT_BEGIN)) { return; }
    Wire.end();
    setAs(DPS310::State::WAIT_BEGIN);
}

DPS310::Result DPS310::request() {
    if (not isIn(DPS310::State::IDLE)) {
        setError(DPS310::Result::FAILED_BUSY);
        return _error;
    }
    // Starting with a temperature measurement
    if (not applyOperationMode(DPS310::OperationMode::ONE_SHOT_TEMPERATURE)) {
        return _error;
    }
    setAs(DPS310::State::TEMP_BUSY);
    return DPS310::Result::SUCCESS;
}

DPS310::Result DPS310::read(float* const temperature, float* const pressure) {
    if (not isIn(DPS310::State::AVAILABLE)) {
        setError(DPS310::Result::FAILED_BUSY);
        return _error;
    }
    *temperature = _values.temperature;
    *pressure = _values.pressure;
    setAs(DPS310::State::IDLE);
    return DPS310::Result::SUCCESS;
}

DPS310::Result DPS310::softReset() {
    uint8_t meas_cfg;
    if (not writeByte(DPS310::Register::RESET, 0x09)) { return _error; }
    do {
        delay(12);
        if (not readByte(DPS310::Register::MEAS_CFG, &meas_cfg)) { return _error; }
    } while (not bitIsSet(meas_cfg, use(DPS310::MEAS_CFG::SENSOR_RDY)));
    return DPS310::Result::SUCCESS;
}

// MARK: Specific utils (private)

DPS310::Result DPS310::applyPressureSettings() {
    uint8_t prs_cfg, cfg_reg;
    // PRS_CFG
    if (not readByte(DPS310::Register::PRS_CFG, &prs_cfg)) { return _error; }
    modifyByte(&prs_cfg, use(DPS310::PRS_CFG::PM_RATE0),
               use(_settings.pressure_sampling_rate), 3);
    modifyByte(&prs_cfg, use(DPS310::PRS_CFG::PM_PRC0),
               use(_settings.pressure_precision), 3);
    if (not writeByte(DPS310::Register::PRS_CFG, prs_cfg)) { return _error; }
    // CFG_REG
    if (not readByte(DPS310::Register::CFG_REG, &cfg_reg)) { return _error; }
    modifyByte(&cfg_reg, use(DPS310::CFG_REG::P_SHIFT),
               use(_settings.pressure_precision) > use(DPS310::Precision::LOW_8X) ? 1 :
                                                                                    0);
    if (not writeByte(DPS310::Register::CFG_REG, cfg_reg)) { return _error; }
    return DPS310::Result::SUCCESS;
}

DPS310::Result DPS310::applyTemperatureSettings() {
    uint8_t tmp_cfg, cfg_reg;
    // TMP_CFG
    if (not readByte(DPS310::Register::TMP_CFG, &tmp_cfg)) { return _error; }
    modifyByte(&tmp_cfg, use(DPS310::TMP_CFG::TMP_EXT),
               use(_settings.temperature_source));
    modifyByte(&tmp_cfg, use(DPS310::TMP_CFG::TMP_RATE0),
               use(_settings.temperature_sampling_rate), 3);
    modifyByte(&tmp_cfg, use(DPS310::TMP_CFG::TMP_PRC0),
               use(_settings.temperature_precision), 3);
    if (not writeByte(DPS310::Register::TMP_CFG, tmp_cfg)) { return _error; }
    // CFG_REG
    if (not readByte(DPS310::Register::CFG_REG, &cfg_reg)) { return _error; }
    modifyByte(
        &cfg_reg, use(DPS310::CFG_REG::T_SHIFT),
        use(_settings.temperature_precision) > use(DPS310::Precision::LOW_8X) ? 1 : 0);
    if (not writeByte(DPS310::Register::CFG_REG, cfg_reg)) { return _error; }
    if (not updateCoefficients()) { return _error; }
    return DPS310::Result::SUCCESS;
}

DPS310::Result DPS310::applyOperationMode(const DPS310::OperationMode mode) {
    uint8_t meas_cfg;
    if (not readByte(DPS310::Register::MEAS_CFG, &meas_cfg)) { return _error; }
    modifyByte(&meas_cfg, use(DPS310::MEAS_CFG::MEAS_CTRL0), use(mode), 3);
    if (not writeByte(DPS310::Register::MEAS_CFG, meas_cfg)) { return _error; }
    return DPS310::Result::SUCCESS;
}

DPS310::Result DPS310::updateCoefficients() {
    // Set coefficient source
    uint8_t coef_srce, meas_cfg;
    if (not readByte(DPS310::Register::COEF_SRCE, &coef_srce)) { return _error; }
    modifyByte(&coef_srce, use(DPS310::COEF_SRCE::TMP_COEF_SRCE),
               use(_settings.temperature_source));
    if (not writeByte(DPS310::Register::COEF_SRCE, coef_srce)) { return _error; }
    // Wait for ready
    do {
        delay(1);
        if (not readByte(DPS310::Register::MEAS_CFG, &meas_cfg)) { return _error; }
    } while (not bitIsSet(meas_cfg, use(DPS310::MEAS_CFG::COEF_RDY)));
    // Read coefficients
    uint8_t c0_msb, c0_lsb_c1_msb, c1_lsb, c00_msb, c00_mid, c00_lsb_c10_msb, c10_mid,
        c10_lsb, c01_msb, c01_lsb, c11_msb, c11_lsb, c20_msb, c20_lsb, c21_msb, c21_lsb,
        c30_msb, c30_lsb;
    if (not(readByte(DPS310::Register::C0_MSB, &c0_msb)
            and readByte(DPS310::Register::C0_LSB_C1_MSB, &c0_lsb_c1_msb)
            and readByte(DPS310::Register::C1_LSB, &c1_lsb)
            and readByte(DPS310::Register::C00_MSB, &c00_msb)
            and readByte(DPS310::Register::C00_MID, &c00_mid)
            and readByte(DPS310::Register::C00_LSB_C10_MSB, &c00_lsb_c10_msb)
            and readByte(DPS310::Register::C10_MID, &c10_mid)
            and readByte(DPS310::Register::C10_LSB, &c10_lsb)
            and readByte(DPS310::Register::C01_MSB, &c01_msb)
            and readByte(DPS310::Register::C01_LSB, &c01_lsb)
            and readByte(DPS310::Register::C11_MSB, &c11_msb)
            and readByte(DPS310::Register::C11_LSB, &c11_lsb)
            and readByte(DPS310::Register::C20_MSB, &c20_msb)
            and readByte(DPS310::Register::C20_LSB, &c20_lsb)
            and readByte(DPS310::Register::C21_MSB, &c21_msb)
            and readByte(DPS310::Register::C21_LSB, &c21_lsb)
            and readByte(DPS310::Register::C30_MSB, &c30_msb)
            and readByte(DPS310::Register::C30_LSB, &c30_lsb))) {
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
    return DPS310::Result::SUCCESS;
}

// MARK: Common I2C utils (private)

DPS310::Result DPS310::readByte(const Register reg, uint8_t* const dst) {
    if (auto&& writer = Wire.get_writer(use(_address))) {
        writer << use(reg);
    } else {
        setError(DPS310::Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    if (auto&& reader = Wire.get_reader(use(_address), 1)) {
        reader >> *dst;
    } else {
        setError(DPS310::Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    return DPS310::Result::SUCCESS;
}

DPS310::Result DPS310::writeByte(const Register reg, const uint8_t src) {
    if (auto&& writer = Wire.get_writer(use(_address))) {
        writer << use(reg);
        writer << src;
    } else {
        setError(DPS310::Result::FAILED_NOT_RESPONDING);
        return _error;
    }
    return DPS310::Result::SUCCESS;
}

// MARK: Operators for results (global)

bool operator!(DPS310::Result result) {
    return result != DPS310::Result::SUCCESS;
}
DPS310::Result operator&&(DPS310::Result lhs, DPS310::Result rhs) {
    return (lhs == DPS310::Result::SUCCESS && rhs == DPS310::Result::SUCCESS) ?
        DPS310::Result::SUCCESS :
        DPS310::Result::FAILED_UNKNOWN;
}
DPS310::Result operator||(DPS310::Result lhs, DPS310::Result rhs) {
    return (lhs == DPS310::Result::SUCCESS || rhs == DPS310::Result::SUCCESS) ?
        DPS310::Result::SUCCESS :
        DPS310::Result::FAILED_UNKNOWN;
}
