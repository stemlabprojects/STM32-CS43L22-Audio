#ifndef AUDIO_H
#define AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Audio codec interface standard */
#define CODEC_STANDARD 0x04

/* I2S standard */
#define I2S_STANDARD I2S_STANDARD_PHILIPS

/* Audio driver structure used by the STM32F4 Discovery BSP */
typedef struct
{
    uint32_t (*Init)(uint16_t DeviceAddr,
                     uint16_t OutputDevice,
                     uint8_t Volume,
                     uint32_t AudioFreq);

    void (*DeInit)(void);

    uint32_t (*ReadID)(uint16_t DeviceAddr);

    uint32_t (*Play)(uint16_t DeviceAddr,
                     uint16_t *pBuffer,
                     uint16_t Size);

    uint32_t (*Pause)(uint16_t DeviceAddr);

    uint32_t (*Resume)(uint16_t DeviceAddr);

    uint32_t (*Stop)(uint16_t DeviceAddr,
                     uint32_t Cmd);

    uint32_t (*SetFrequency)(uint16_t DeviceAddr,
                             uint32_t AudioFreq);

    uint32_t (*SetVolume)(uint16_t DeviceAddr,
                          uint8_t Volume);

    uint32_t (*SetMute)(uint16_t DeviceAddr,
                        uint32_t Cmd);

    uint32_t (*SetOutputMode)(uint16_t DeviceAddr,
                              uint8_t Output);

    uint32_t (*Reset)(uint16_t DeviceAddr);

} AUDIO_DrvTypeDef;

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_H */
