#ifndef SPI_V3X_COMPAT
#define SPI_V3X_COMPAT

#if ESP_ARDUINO_VERSION_MAJOR >= 3

////////////////////////////////////////////////////////////////////////////////////////
// Macros override for Arduino ESP32 v3.x compatibility
////////////////////////////////////////////////////////////////////////////////////////

// Welcome to hell :fire:

#include "soc/spi_periph.h"
#include "soc/spi_struct.h"
#include "soc/spi_reg.h"
#include "soc/io_mux_reg.h"
#include "hal/spi_ll.h"

// Map SPI port to device struct
static inline spi_dev_t* _spi_dev(uint8_t port) {
    switch(port) {
        case HSPI: return &SPI2;
        case VSPI: return &SPI3;
        default:   return &SPI3;
    }
}

// Remove any previous SPI macro definitions
#undef WRITE_PERI_REG
#undef READ_PERI_REG
#undef SET_PERI_REG_MASK

#undef SPI_CMD_REG
#undef SPI_MOSI_DLEN_REG

#undef SPI_W0_REG
#undef SPI_W1_REG
#undef SPI_W2_REG
#undef SPI_W3_REG
#undef SPI_W4_REG
#undef SPI_W5_REG
#undef SPI_W6_REG
#undef SPI_W7_REG
#undef SPI_W8_REG
#undef SPI_W9_REG
#undef SPI_W10_REG
#undef SPI_W11_REG
#undef SPI_W12_REG
#undef SPI_W13_REG
#undef SPI_W14_REG
#undef SPI_W15_REG

#undef SPI_USR

// Basic register access compatibility
#define WRITE_PERI_REG(addr,val) (*(volatile uint32_t*)(addr) = (val))
#define READ_PERI_REG(addr) (*(volatile uint32_t*)(addr))
#define SET_PERI_REG_MASK(reg,mask) WRITE_PERI_REG((reg), READ_PERI_REG(reg) | (mask))

#define SPI_MOSI_DLEN_REG(port) (&(_spi_dev(port)->mosi_dlen.val))
#define SPI_CMD_REG(port) (&(_spi_dev(port)->cmd.val))

#define SPI_W0_REG(port)  (&(_spi_dev(port)->data_buf[0]))
#define SPI_W1_REG(port)  (&(_spi_dev(port)->data_buf[1]))
#define SPI_W2_REG(port)  (&(_spi_dev(port)->data_buf[2]))
#define SPI_W3_REG(port)  (&(_spi_dev(port)->data_buf[3]))
#define SPI_W4_REG(port)  (&(_spi_dev(port)->data_buf[4]))
#define SPI_W5_REG(port)  (&(_spi_dev(port)->data_buf[5]))
#define SPI_W6_REG(port)  (&(_spi_dev(port)->data_buf[6]))
#define SPI_W7_REG(port)  (&(_spi_dev(port)->data_buf[7]))
#define SPI_W8_REG(port)  (&(_spi_dev(port)->data_buf[8]))
#define SPI_W9_REG(port)  (&(_spi_dev(port)->data_buf[9]))
#define SPI_W10_REG(port) (&(_spi_dev(port)->data_buf[10]))
#define SPI_W11_REG(port) (&(_spi_dev(port)->data_buf[11]))
#define SPI_W12_REG(port) (&(_spi_dev(port)->data_buf[12]))
#define SPI_W13_REG(port) (&(_spi_dev(port)->data_buf[13]))
#define SPI_W14_REG(port) (&(_spi_dev(port)->data_buf[14]))
#define SPI_W15_REG(port) (&(_spi_dev(port)->data_buf[15]))

#define SPI_USR (1<<18)

#endif

#endif