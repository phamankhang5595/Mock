#ifndef _HEADER_HAL_H_
#define _HEADER_HAL_H_
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOOT_SECTOR_BYTE 512

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief <Read Boot Sector>
 *
 * @param *BootSecBuff <storage Boot data>
 *
 * @return <Data of Boot>.
 */
void HAL_ReadBootSec(uint8_t *BootSecBuff);

/*!
 * @brief <Read one sector>
 *
 * @param Address <locate start address of sector>.
 * @param *SecBuff <storage Sector data>.
 *
 * @return <A pointer storage sector data>.
 */
void HAL_ReadSec(unsigned int Address, uint8_t *SecBuff);

/*!
 * @brief <Read multi sector>
 *
 * @param Address <locate start address of sector>.
 * @param SecBuff <storage Sector data>.
 * @param NumOFSec <Number of sector>.
 *
 * @return <>.
 */
void HAL_ReadMultiSec(unsigned int FirstAddress, int NumOFSec, uint8_t *MultiSecBuff);

/*!
 * @brief <Close file>
 * @param g_Fp <FILE pointer>.
 *
 * @return <>.
 */
void HAL_CloseFile();

/*!
 * @brief <initialize file>
 *
 * @param FileName <File name>.
 *
 * @return <>.
 */
void HAL_File_init(char *FileName);

#endif // HAL_H_INCLUDED
