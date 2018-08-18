/* 
 * "Small Hello World" example. 
 * 
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example 
 * designs. It requires a STDOUT  device in your system's hardware. 
 *
 * The purpose of this example is to demonstrate the smallest possible Hello 
 * World application, using the Nios II HAL library.  The memory footprint
 * of this hosted application is ~332 bytes by default using the standard 
 * reference design.  For a more fully featured Hello World application
 * example, see the example titled "Hello World".
 *
 * The memory footprint of this example has been reduced by making the
 * following changes to the normal "Hello World" example.
 * Check in the Nios II Software Developers Manual for a more complete 
 * description.
 * 
 * In the SW Application project (small_hello_world):
 *
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 * In System Library project (small_hello_world_syslib):
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 *    - Define the preprocessor option ALT_NO_INSTRUCTION_EMULATION 
 *      This removes software exception handling, which means that you cannot 
 *      run code compiled for Nios II cpu with a hardware multiplier on a core 
 *      without a the multiply unit. Check the Nios II Software Developers 
 *      Manual for more details.
 *
 *  - In the System Library page:
 *    - Set Periodic system timer and Timestamp timer to none
 *      This prevents the automatic inclusion of the timer driver.
 *
 *    - Set Max file descriptors to 4
 *      This reduces the size of the file handle pool.
 *
 *    - Check Main function does not exit
 *    - Uncheck Clean exit (flush buffers)
 *      This removes the unneeded call to exit when main returns, since it
 *      won't.
 *
 *    - Check Don't use C++
 *      This builds without the C++ support code.
 *
 *    - Check Small C library
 *      This uses a reduced functionality C library, which lacks  
 *      support for buffering, file IO, floating point and getch(), etc. 
 *      Check the Nios II Software Developers Manual for a complete list.
 *
 *    - Check Reduced device drivers
 *      This uses reduced functionality drivers if they're available. For the
 *      standard design this means you get polled UART and JTAG UART drivers,
 *      no support for the LCD driver and you lose the ability to program 
 *      CFI compliant flash devices.
 *
 *    - Check Access device drivers directly
 *      This bypasses the device file system to access device drivers directly.
 *      This eliminates the space required for the device file system services.
 *      It also provides a HAL version of libc services that access the drivers
 *      directly, further reducing space. Only a limited number of libc
 *      functions are available in this configuration.
 *
 *    - Use ALT versions of stdio routines:
 *
 *           Function                  Description
 *        ===============  =====================================
 *        alt_printf       Only supports %s, %x, and %c ( < 1 Kbyte)
 *        alt_putstr       Smaller overhead than puts with direct drivers
 *                         Note this function doesn't add a newline.
 *        alt_putchar      Smaller overhead than putchar with direct drivers
 *        alt_getchar      Smaller overhead than getchar with direct drivers
 *
 */

#include "sys/alt_stdio.h"
#include <cstdint>

#include "system.h"
#include "nios2.h"
struct gen_qspi_type {
	volatile uint32_t CR;
	volatile uint32_t SPI_CBR;
	volatile uint32_t CS_DSR;
	volatile uint32_t RCR;
	volatile uint32_t OPSR;
	volatile uint32_t RIR;
	volatile uint32_t WIR;
	volatile uint32_t FCSR;
	volatile uint32_t FCCR;
	volatile uint32_t FCAR;
	volatile uint32_t FCWD0R;
	volatile uint32_t FCWD1R;
	volatile uint32_t FCRD0R;
	volatile uint32_t FCRD1R;
};
union FCSR_bits {
	struct {
		uint32_t opcode: 8;
		uint32_t num_addr_bytes: 3;
		uint32_t data_type: 1;
		uint32_t num_data_bytes: 4;
		uint32_t num_dummy_cycles: 5;
	};
	uint32_t FCSR;

	constexpr operator uint32_t() {
		return FCSR;
	}
} ;

auto qspi_flash = (gen_qspi_type*)GEN_QSPI_AVL_CSR_BASE;
constexpr FCSR_bits Read_JEDEC_ID = {{0x9f, 0, 1, 4, 0}};
constexpr FCSR_bits Read_SR = {{0x05, 0, 1, 2, 0}};
constexpr FCSR_bits Write_Enable = {{0x06, 0, 0, 0, 0}};
constexpr FCSR_bits Write_Enable_Volatile_SR = {{0x50, 0, 0, 0, 0}};
constexpr FCSR_bits Write_SR = {{0x01, 0, 0, 2, 0}};
constexpr FCSR_bits Sector_Erase = {{0x20, 3, 0, 0, 0}};
constexpr FCSR_bits Enable_QPI = {{0x38, 0, 0, 0, 0}};

int main()
{ 
  alt_putstr("Hello from Nios II!\r\n");

  qspi_flash->CS_DSR = 0;
  qspi_flash->WIR = 0x00000502;
  qspi_flash->SPI_CBR = 0x00000002;

  // read jedec id
  qspi_flash->FCSR = Read_JEDEC_ID;
  qspi_flash->FCCR = 0x00000001;

  // read Sr
  qspi_flash->FCSR = Read_SR;
  qspi_flash->FCCR = 0x00000001;

  // write enable volatile bits
  qspi_flash->FCSR = 0x00000050;
  qspi_flash->FCSR = Write_Enable_Volatile_SR;
  qspi_flash->FCCR = 0x00000001;

  // set qe bit
  qspi_flash->FCSR = Write_SR;
  qspi_flash->FCWD0R = 0x0200;
  qspi_flash->FCCR = 0x00000001;

  // enable write
  qspi_flash->FCSR = Write_Enable;
  qspi_flash->FCCR = 0x00000001;

//  qspi_flash->FCSR = Sector_Erase;
//  qspi_flash->FCAR = 0;
//  qspi_flash->FCCR = 0x00000001;

  // enable QPI
  qspi_flash->FCSR = Enable_QPI;
  qspi_flash->FCCR = 0x00000001;

  // set controller to quad mode
  qspi_flash->OPSR = 0x00022222;
  qspi_flash->RIR = 0x0000020b;

//  for(int i = 0; i < 256; i++) {
//	  *(char*)(i) = i;
//  }
  while(1) {
	  NIOS2_BREAK();
  }
  return 0;
}
