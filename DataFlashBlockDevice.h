/* mbed Microcontroller Library
 * Copyright (c) 2016 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_DATAFLASH_BLOCK_DEVICE_H
#define MBED_DATAFLASH_BLOCK_DEVICE_H

#include <atomic>

#include <mbed.h>

using bd_addr_t = uint64_t;
using bd_size_t = uint64_t;

/** Enum of standard error codes
 *
 *  @enum bd_error
 */
enum bd_error {
    BD_ERROR_OK                 = 0,     /*!< no error */
    BD_ERROR_DEVICE_ERROR       = -4001, /*!< device specific error */
};


/** BlockDevice for DataFlash flash devices
 *
 *  @code
 *  // Here's an example using the AT45DB on the K64F
 *  #include "mbed.h"
 *  #include "DataFlashBlockDevice.h"
 *
 *  // Create DataFlash on SPI bus with PTE5 as chip select
 *  DataFlashBlockDevice dataflash(PTE2, PTE4, PTE1, PTE5);
 *
 *  // Create DataFlash on SPI bus with PTE6 as write-protect
 *  DataFlashBlockDevice dataflash2(PTE2, PTE4, PTE1, PTE5, PTE6);
 *
 *  int main() {
 *      printf("dataflash test\n");
 *
 *      // Initialize the SPI flash device and print the memory layout
 *      dataflash.init();
 *      printf("dataflash size: %llu\n", dataflash.size());
 *      printf("dataflash read size: %llu\n", dataflash.get_read_size());
 *      printf("dataflash program size: %llu\n", dataflash.get_program_size());
 *      printf("dataflash erase size: %llu\n", dataflash.get_erase_size());
 *
 *      // Write "Hello World!" to the first block
 *      char *buffer = (char*)malloc(dataflash.get_erase_size());
 *      sprintf(buffer, "Hello World!\n");
 *      dataflash.erase(0, dataflash.get_erase_size());
 *      dataflash.program(buffer, 0, dataflash.get_erase_size());
 *
 *      // Read back what was stored
 *      dataflash.read(buffer, 0, dataflash.get_erase_size());
 *      printf("%s", buffer);
 *
 *      // Deinitialize the device
 *      dataflash.deinit();
 *  }
 *  @endcode
 */
class DataFlashBlockDevice {
public:
    /** Creates a DataFlashBlockDevice on a SPI bus specified by pins
     *
     *  @param mosi     SPI master out, slave in pin
     *  @param miso     SPI master in, slave out pin
     *  @param sclk     SPI clock pin
     *  @param csel     SPI chip select pin
     *  @param nowp     GPIO not-write-protect
     *  @param freq     Clock speed of the SPI bus (defaults to 40MHz)
     */
    DataFlashBlockDevice(PinName mosi,
                      PinName miso,
                      PinName sclk,
                      PinName csel,
                      int freq = 40000000,
                      PinName nowp = NC);

    /** Initialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    int init();

    /** Deinitialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    int deinit();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to write blocks to
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of read block size
     *  @return         0 on success, negative error code on failure
     */
    int read(void *buffer, bd_addr_t addr, bd_size_t size);

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of program block size
     *  @return         0 on success, negative error code on failure
     */
    int program(const void *buffer, bd_addr_t addr, bd_size_t size);

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of erase block size
     *  @return         0 on success, negative error code on failure
     */
    int erase(bd_addr_t addr, bd_size_t size);

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    bd_size_t get_read_size() const;

    /** Get the size of a programable block
     *
     *  @return         Size of a programable block in bytes
     *  @note Must be a multiple of the read size
     */
    bd_size_t get_program_size() const;

    /** Get the size of a eraseable block
     *
     *  @return         Size of a eraseable block in bytes
     *  @note Must be a multiple of the program size
     */
    bd_size_t get_erase_size() const;

    /** Get the size of an erasable block given address
     *
     *  @param addr     Address within the erasable block
     *  @return         Size of an erasable block in bytes
     *  @note Must be a multiple of the program size
     */
    bd_size_t get_erase_size(bd_addr_t addr) const;

    /** Get the total size of the underlying device
     *
     *  @return         Size of the underlying device in bytes
     */
    bd_size_t size() const;

    /** Convenience function for checking block read validity
	 *
	 *  @param addr     Address of block to begin reading from
	 *  @param size     Size to read in bytes
	 *  @return         True if read is valid for underlying block device
	 */
    bool is_valid_read(bd_addr_t addr, bd_size_t size) const;

    /** Convenience function for checking block program validity
	 *
	 *  @param addr     Address of block to begin writing to
	 *  @param size     Size to write in bytes
	 *  @return         True if program is valid for underlying block device
	 */
    bool is_valid_program(bd_addr_t addr, bd_size_t size) const;

    /** Convenience function for checking block erase validity
	 *
	 *  @param addr     Address of block to begin erasing
	 *  @param size     Size to erase in bytes
	 *  @return         True if erase is valid for underlying block device
	 */
    bool is_valid_erase(bd_addr_t addr, bd_size_t size) const;

private:
    // Master side hardware
    SPI _spi;
    DigitalOut _cs;
    DigitalOut _nwp;

    // Device configuration
    uint32_t _device_size;
    uint16_t _page_size;
    uint16_t _block_size;
    bool _is_initialized;
    std::atomic<uint32_t> _init_ref_count;

    // Internal functions
    uint16_t _get_register(uint8_t opcode);
    void _write_command(uint32_t command, const uint8_t *buffer, uint32_t size);
    void _write_enable(bool enable);
    int _sync(void);
    int _write_page(const uint8_t *buffer, uint32_t addr, uint32_t offset, uint32_t size);
    uint32_t _translate_address(bd_addr_t addr);
};


#endif  /* MBED_DATAFLASH_BLOCK_DEVICE_H */
