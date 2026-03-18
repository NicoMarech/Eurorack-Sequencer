#pragma once

#include "main.h"

#include <array>
#include <cstddef>

class SpiAbstraction
{
public:
    using Callback = void (*)(SPI_HandleTypeDef *inHspi, void *inContext);

    static bool registerTxCompleteCallback(SPI_HandleTypeDef *inHspi, Callback inCallback, void *inContext = nullptr);
    static bool registerTxRxCompleteCallback(SPI_HandleTypeDef *inHspi, Callback inCallback, void *inContext = nullptr);
    static bool registerErrorCallback(SPI_HandleTypeDef *inHspi, Callback inCallback, void *inContext = nullptr);

    static void dispatchTxComplete(SPI_HandleTypeDef *inHspi);
    static void dispatchTxRxComplete(SPI_HandleTypeDef *inHspi);
    static void dispatchError(SPI_HandleTypeDef *inHspi);

    struct CallbackSlot
    {
        SPI_HandleTypeDef *hspi;
        Callback callback;
        void *context;
    };

    static constexpr std::size_t sMaxSlots = 8U;

private:
    static bool registerCallback(std::array<CallbackSlot, sMaxSlots> &inSlots,
                                 SPI_HandleTypeDef *inHspi,
                                 Callback inCallback,
                                 void *inContext);
    static void dispatch(std::array<CallbackSlot, sMaxSlots> &inSlots, SPI_HandleTypeDef *inHspi);

    static std::array<CallbackSlot, sMaxSlots> sTxCompleteSlots;
    static std::array<CallbackSlot, sMaxSlots> sTxRxCompleteSlots;
    static std::array<CallbackSlot, sMaxSlots> sErrorSlots;
};
