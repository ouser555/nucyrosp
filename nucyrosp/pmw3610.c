/* Copyright 2021 Colin Lam (Ploopy Corporation)
 * Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
 * Copyright 2019 Hiroyuki Okada
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pmw3610.h"
#include "wait.h"
#include "debug.h"
#include "gpio.h"

static bool isPMW3610 = true;
static bool sw_smart_flag = false;

void pmw3610_init(void) {
    // Initialize sensor serial pins.
    setPinOutput(PMW3610_SCLK_PIN);
    setPinOutput(PMW3610_SDIO_PIN);
    setPinOutput(PMW3610_CS_PIN);

    pmw3610_sync();
    // reboot the sensor.

    pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_ENABLE);

    pmw3610_write_reg(REG_POWER_UP_RESET, 0x5a);

    pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);
    // wait maximum time before sensor is ready.
    // this ensures that the sensor is actually ready after reset.
    wait_ms(55);


    pmw3610_sync();

    if(pmw3610_check_signature()){


        pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_ENABLE);
    // read a burst from the sensor and then discard it.
    // gets the sensor ready for write commands
    // (for example, setting the dpi).
        pmw3610_write_reg(REG_OBSERVATION, 0x00); // clear ob1

        //if((pmw3610_read_reg(REG_OBSERVATION)&0x0F)!=0x0F){
        //    isPMW3610 = false;
        //}
        isPMW3610 = true;

        pmw3610_write_reg(REG_PERFORMANCE, 0xFD);

        pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);

        //pmw3610_write_reg(REG_RES_STEP, PMW361_CPI_400); // 3200/200=16=0x10
        pmw3610_set_cpi(PMW361_CPI_800);
        //pmw3610_set_cpi(PMW361_CPI_3200);
        //pmw3610_set_cpi(400);
        /*
        pmw3610_write_reg(0x1E, 0x01);
        pmw3610_write_reg(0x1F, 0x00);
        pmw3610_write_reg(0x20, 0x00);
        pmw3610_write_reg(0x2A, 0x00);
        pmw3610_write_reg(0x2C, 0x00);
        pmw3610_write_reg(0x2E, 0x00);
        pmw3610_write_reg(0x2F, 0x00);
        pmw3610_write_reg(0x30, 0x00);
        pmw3610_write_reg(0x31, 0x00);
        pmw3610_write_reg(0x32, 0x00);
        pmw3610_write_reg(0x33, 0x00);
        pmw3610_write_reg(0x34, 0xFF);
        pmw3610_write_reg(0x35, 0xFF);
        pmw3610_write_reg(0x36, 0xFF);
        pmw3610_write_reg(0x37, 0xFF);
        */
        //pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);

        pmw3610_read_burst();

    // Pretty sure that this shouldn't be in the driver.
    // Probably device specific?
    // Set rest mode to default
    //pmw3610_write_reg(REG_Rest_Mode_Status, 0x00);
    // Set LED to be always on
    //pmw3610_write_reg(REG_Led_Control, 0x4);
    // Disable rest mode

    }else{
        isPMW3610 = false;
    }

}

// Perform a synchronization with sensor.
// Just as with the serial protocol, this is used by the slave to send a
// synchronization signal to the master.
void pmw3610_sync(void) {
    writePinLow(PMW3610_CS_PIN);
    wait_us(1);
    writePinHigh(PMW3610_CS_PIN);
}

void pmw3610_cs_select(void) {
    writePinLow(PMW3610_CS_PIN);
}

void pmw3610_cs_deselect(void) {
    writePinHigh(PMW3610_CS_PIN);
}

uint8_t pmw3610_serial_read(void) {
    //setPinInput(PMW3610_SDIO_PIN);
    setPinInputHigh(PMW3610_SDIO_PIN);
    uint8_t byte = 0;

    for (uint8_t i = 0; i < 8; ++i) { 
        writePinLow(PMW3610_SCLK_PIN);
        wait_us(1);

        byte = (byte << 1) | readPin(PMW3610_SDIO_PIN);

        writePinHigh(PMW3610_SCLK_PIN);
        wait_us(1);
    }

    return byte;
}

void pmw3610_serial_write(uint8_t data) {
    setPinOutput(PMW3610_SDIO_PIN);

    for (int8_t b = 7; b >= 0; b--) {
        writePinLow(PMW3610_SCLK_PIN);

        if (data & (1 << b))
            writePinHigh(PMW3610_SDIO_PIN);
        else
            writePinLow(PMW3610_SDIO_PIN);

        wait_us(2);

        writePinHigh(PMW3610_SCLK_PIN);
    }

    // This was taken from ADNS5050 driver.
    // There's no any info in PMW3320 datasheet about this...
    // tSWR. See page 15 of the ADNS5050 spec sheet.
    // Technically, this is only necessary if the next operation is an SDIO
    // read. This is not guaranteed to be the case, but we're being lazy.
    wait_us(4);

    // Note that tSWW is never necessary. All write operations require at
    // least 32us, which exceeds tSWW, so there's never a need to wait for it.
}

// Read a byte of data from a register on the sensor.
uint8_t pmw3610_read_reg(uint8_t reg_addr) {
    pmw3610_cs_select();

    pmw3610_serial_write(reg_addr);

    uint8_t byte = pmw3610_serial_read();

    // This was taken directly from ADNS5050 driver...
    // tSRW & tSRR. See page 15 of the ADNS5050 spec sheet.
    // Technically, this is only necessary if the next operation is an SDIO
    // read or write. This is not guaranteed to be the case.
    // Honestly, this wait could probably be removed.
    wait_us(1);

    pmw3610_cs_deselect();

    return byte;
}

void pmw3610_write_reg(uint8_t reg_addr, uint8_t data) {
    pmw3610_cs_select();
    pmw3610_serial_write(0b10000000 | reg_addr);
    pmw3610_serial_write(data);
    pmw3610_cs_deselect();
}

report_pmw3610_t pmw3610_read_burst(void) {
    

    

    report_pmw3610_t data;
    data.dx = 0;
    data.dy = 0;

    if(isPMW3610){

    pmw3610_cs_select();

    pmw3610_serial_write(REG_MOTION_BURST);

    uint8_t mot = pmw3610_serial_read();    //0
    uint8_t lx = pmw3610_serial_read();     //1
    uint8_t ly = pmw3610_serial_read();     //2
    uint8_t hxy = pmw3610_serial_read();    //3

    uint8_t squal = pmw3610_serial_read();    //4
    uint8_t shutterh = pmw3610_serial_read();    //5
    uint8_t shutterl = pmw3610_serial_read();    //6

    pmw3610_cs_deselect();
    if(squal>0){squal=15;}
//smart algorithm
    int16_t shutter = ((int16_t)(shutterh & 0x01) << 8) + shutterl;
    if (sw_smart_flag && shutter < 45) {
        pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_ENABLE);
        pmw3610_write_reg(0x32, 0x00);
        pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);
        sw_smart_flag = false;
    }
    if (sw_smart_flag && shutter > 45) {
        pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_ENABLE);
        pmw3610_write_reg(0x32, 0x80);
        pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);
        sw_smart_flag = true;
    }

    // Probably burst mode may include contents of delta_xy register,
    // which contain HI parts of x/y deltas, but I had no luck finding it.
    // Probably it's required to activate 12-bit mode to access this data.
    // So we end burst mode early to not read unneeded information.
    //pmw3610_cs_deselect();

    //uint8_t tmp = mot;
    //mot = hxy;


    //int16_t x = TOINT16((buf[PMW3610_X_L_POS] + ((buf[PMW3610_XY_H_POS] & 0xF0) << 4)),12) / CONFIG_PMW3610_CPI_DIVIDOR;
    //int16_t y = TOINT16((buf[PMW3610_Y_L_POS] + ((buf[PMW3610_XY_H_POS] & 0x0F) << 8)),12) / CONFIG_PMW3610_CPI_DIVIDOR;
    if(mot & 0x80){
        data.dx = TOINT16(lx + ((hxy & 0xF0) << 4),12);
        data.dy = TOINT16(ly + ((hxy & 0x0F) << 8),12);
    }
    //data.dx = convert_twoscomp(lx);
    //data.dy = convert_twoscomp(ly);
    }

    return data;
}

// Convert a two's complement byte from an unsigned data type into a signed
// data type.
int8_t convert_twoscomp(uint8_t data) {
    if ((data & 0x80) == 0x80)
        return -128 + (data & 0x7F);
    else
        return data;
}

uint16_t pmw3610_get_cpi(void) {

    //pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_ENABLE);
    pmw3610_write_reg(REG_SPI_PAGE0, 0xFF);
    uint8_t cpival = pmw3610_read_reg(REG_RES_STEP);
    pmw3610_write_reg(REG_SPI_PAGE1, 0x00);
    //pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);
    // 0x1F is an inversion of 0x20 which is 0b100000
    //return (uint16_t)((cpival & 0x1F) * PMW3610_CPI_STEP);
    //return (uint16_t)((cpival+1) & 0xFF);
    return (uint16_t)(cpival+1);
}


#if 0
void pmw3610_set_cpi(uint16_t cpi) {
    uint8_t cpival = constrain((cpi / PMW3610_CPI_STEP) - 1U, 0, (PMW3610_CPI_MAX / PMW3610_CPI_STEP) - 1U);
    // Fifth bit is probably a control bit.
    // PMW3320 datasheet don't have any info on this, so this is a pure guess.
    pmw3610_write_reg(REG_RES_STEP, 0x20 | cpival);
}
#else
void pmw3610_set_cpi(uint16_t cpi) {

    //pmw3610_sync();
    //uint8_t cpival = constrain((cpi / PMW3610_CPI_STEP) - 1U, 0, (PMW3610_CPI_MAX / PMW3610_CPI_STEP) - 1U);
    // Fifth bit is probably a control bit.
    // PMW3320 datasheet don't have any info on this, so this is a pure guess.

    pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_ENABLE);

    pmw3610_write_reg(REG_SPI_PAGE0, 0xFF);
    pmw3610_write_reg(REG_RES_STEP, cpi);
    pmw3610_write_reg(REG_SPI_PAGE1, 0x00);

    pmw3610_write_reg(REG_SPI_CLK_ON_REQ, SPI_CLOCK_CMD_DISABLE);

}
#endif
bool pmw3610_check_signature(void) {
    uint8_t pid  = pmw3610_read_reg(REG_Product_ID);
    //uint8_t revid = pmw3610_read_reg(REG_Inverse_Product_ID);

    //return (pid == PMW3610_PID && revid == PMW3610_RID);
    return (pid == PMW3610_PID);
}
