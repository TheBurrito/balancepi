-include ../../common.mk

#sets the hex file to be built as part of a full build
all: balancepihat.hex i2c_imu_test.hex ahrs_test.hex

#adds appropriate libraries to the compiled firmware
balancepihat.hex: system.a util.a base.a

i2c_imu_test.hex: system.a util.a

ahrs_test.hex: system.a imu.a util.a
