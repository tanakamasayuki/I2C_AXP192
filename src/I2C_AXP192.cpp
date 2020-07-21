#include "I2C_AXP192.h"

I2C_AXP192::I2C_AXP192(uint8_t deviceAddress, TwoWire &i2cPort) {
  _deviceAddress = deviceAddress;
  _i2cPort = &i2cPort;
}

void I2C_AXP192::begin(I2C_AXP192_InitDef initDef) {
  ESP_LOGD("AXP192", "Begin");

  setEXTEN(initDef.EXTEN);
  setBACKUP(initDef.BACKUP);

  setDCDC1(initDef.DCDC1);
  setDCDC2(initDef.DCDC2);
  setDCDC3(initDef.DCDC3);

  setLDO2(initDef.LDO2);
  setLDO3(initDef.LDO3);

  setGPIO0(initDef.GPIO0);
  setGPIO1(initDef.GPIO1);
  setGPIO2(initDef.GPIO2);
  setGPIO3(initDef.GPIO3);
  setGPIO4(initDef.GPIO4);

  WriteByte(0x84, 0b11110010);  // ADC 200Hz
  WriteByte(0x82, 0b11111111);  // ADC All Enable
  WriteByte(0x33, 0b11000000);  // Charge 4.2V, 100mA
  WriteByte(0x36, 0b00001100);  // PEK 128ms, PW OFF 4S
  WriteByte(0x30, 0b10000000);  // VBUS Open
  WriteByte(0x39, 0b11111100);  // Temp Protection
  WriteByte(0x31, 0b00000100);  // Power Off 3.0V
}

uint8_t I2C_AXP192::ReadByte(uint8_t Address) {
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(Address);
  _i2cPort->endTransmission();
  _i2cPort->requestFrom(_deviceAddress, 1);
  uint8_t val = _i2cPort->read();

  ESP_LOGD("AXP192", "ReadByte(%02X) = %02X", Address, val);
  return val;
}

void I2C_AXP192::WriteByte(uint8_t Address, uint8_t Data) {
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(Address);
  _i2cPort->write(Data);
  _i2cPort->endTransmission();
  ESP_LOGD("AXP192", "WriteByte(%02X) = %02X", Address, Data);
}

void I2C_AXP192::BitOn(uint8_t Address, uint8_t Bit) {
  uint8_t add = Address;
  uint8_t val = ReadByte(add) | Bit;
  WriteByte(add, val);
}

void I2C_AXP192::BitOff(uint8_t Address, uint8_t Bit) {
  uint8_t add = Address;
  uint8_t val = ReadByte(add) & ~Bit;
  WriteByte(add, val);
}

void I2C_AXP192::setDCDC1(int voltage) {
  uint8_t add = 0x12;
  uint8_t val;

  if (voltage < 700 || 3500 < voltage) {
    // Disable
    BitOff(add, (1 << 0));
    return;
  } else {
    // Enable
    BitOn(add, (1 << 0));
  }

  // Set
  add = 0x26;
  val = ((voltage - 700) / 25) & 0x7f;
  WriteByte(add, val);
}

void I2C_AXP192::setDCDC2(int voltage) {
  uint8_t add = 0x12;
  uint8_t val;

  if (voltage < 700 || 3500 < voltage) {
    // Disable
    BitOff(add, (1 << 4));
    return;
  } else {
    // Enable
    BitOn(add, (1 << 4));
  }

  // Set
  add = 0x23;
  val = ((voltage - 700) / 25) & 0x7f;
  WriteByte(add, val);
}

void I2C_AXP192::setDCDC3(int voltage) {
  uint8_t add = 0x12;
  uint8_t val;

  if (voltage < 700 || 3500 < voltage) {
    // Disable
    BitOff(add, (1 << 1));
    return;
  } else {
    // Enable
    BitOn(add, (1 << 1));
  }

  // Set
  add = 0x27;
  val = ((voltage - 700) / 25) & 0x7f;
  WriteByte(add, val);
}

void I2C_AXP192::setLDO2(int voltage) {
  uint8_t add = 0x12;
  uint8_t val;

  if (voltage < 1800 || 3300 < voltage) {
    // Disable
    BitOff(add, (1 << 2));
    return;
  } else {
    // Enable
    BitOn(add, (1 << 2));
  }

  // Set
  add = 0x28;
  val = ReadByte(add) & 0x0f;
  val |= (((voltage - 1800) / 100) & 0x0f ) << 4;
  WriteByte(add, val);
}

void I2C_AXP192::setLDO3(int voltage) {
  uint8_t add = 0x12;
  uint8_t val;

  if (voltage < 1800 || 3300 < voltage) {
    // Disable
    BitOff(add, (1 << 3));
    return;
  } else {
    // Enable
    BitOn(add, (1 << 3));
  }

  // Set
  add = 0x28;
  val = ReadByte(add) & 0xf0;
  val |= (((voltage - 1800) / 100) & 0x0f );
  WriteByte(add, val);
}

void I2C_AXP192::setEXTEN(bool enable) {
  uint8_t add = 0x12;

  if (enable) {
    // Enable
    BitOn(add, (1 << 6));
  } else {
    // Disable
    BitOff(add, (1 << 6));
  }
}

void I2C_AXP192::setBACKUP(bool enable) {
  uint8_t add = 0x35;

  if (enable) {
    // Enable
    BitOn(add, (1 << 7));
  } else {
    // Disable
    BitOff(add, (1 << 7));
  }
}

void I2C_AXP192::setGPIO0(int voltage) {
  uint8_t add = 0x90;
  uint8_t val;

  if (voltage < 1800 || 3300 < voltage) {
    // Set Mode
    WriteByte(add, voltage);
  } else {
    // LDO
    WriteByte(add, 0b00000010);

    // Set voltage
    add = 0x91;
    val = (((voltage - 1800) / 100) & 0x0f ) << 4;
    WriteByte(add, val);
  }
}

void I2C_AXP192::setGPIO1(int voltage) {
}

void I2C_AXP192::setGPIO2(int voltage) {
}

void I2C_AXP192::setGPIO3(int voltage) {
}

void I2C_AXP192::setGPIO4(int voltage) {
}
