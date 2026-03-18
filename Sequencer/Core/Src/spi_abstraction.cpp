#include "spi_abstraction.h"

std::array<SpiAbstraction::CallbackSlot, SpiAbstraction::sMaxSlots> SpiAbstraction::sTxCompleteSlots{};
std::array<SpiAbstraction::CallbackSlot, SpiAbstraction::sMaxSlots> SpiAbstraction::sTxRxCompleteSlots{};
std::array<SpiAbstraction::CallbackSlot, SpiAbstraction::sMaxSlots> SpiAbstraction::sErrorSlots{};

bool SpiAbstraction::registerTxCompleteCallback(SPI_HandleTypeDef *inHspi, Callback inCallback, void *inContext)
{
    return registerCallback(sTxCompleteSlots, inHspi, inCallback, inContext);
}

bool SpiAbstraction::registerTxRxCompleteCallback(SPI_HandleTypeDef *inHspi, Callback inCallback, void *inContext)
{
    return registerCallback(sTxRxCompleteSlots, inHspi, inCallback, inContext);
}

bool SpiAbstraction::registerErrorCallback(SPI_HandleTypeDef *inHspi, Callback inCallback, void *inContext)
{
    return registerCallback(sErrorSlots, inHspi, inCallback, inContext);
}

void SpiAbstraction::dispatchTxComplete(SPI_HandleTypeDef *inHspi)
{
    dispatch(sTxCompleteSlots, inHspi);
}

void SpiAbstraction::dispatchTxRxComplete(SPI_HandleTypeDef *inHspi)
{
    dispatch(sTxRxCompleteSlots, inHspi);
}

void SpiAbstraction::dispatchError(SPI_HandleTypeDef *inHspi)
{
    dispatch(sErrorSlots, inHspi);
}

bool SpiAbstraction::registerCallback(std::array<CallbackSlot, sMaxSlots> &inSlots,
                                      SPI_HandleTypeDef *inHspi,
                                      Callback inCallback,
                                      void *inContext)
{
    if ((inHspi == nullptr) || (inCallback == nullptr))
    {
        return false;
    }

    for (std::size_t index = 0U; index < sMaxSlots; ++index)
    {
        if (inSlots[index].hspi == inHspi)
        {
            inSlots[index].callback = inCallback;
            inSlots[index].context = inContext;
            return true;
        }
    }

    for (std::size_t index = 0U; index < sMaxSlots; ++index)
    {
        if (inSlots[index].hspi == nullptr)
        {
            inSlots[index].hspi = inHspi;
            inSlots[index].callback = inCallback;
            inSlots[index].context = inContext;
            return true;
        }
    }

    return false;
}

void SpiAbstraction::dispatch(std::array<CallbackSlot, sMaxSlots> &inSlots, SPI_HandleTypeDef *inHspi)
{
    if (inHspi == nullptr)
    {
        return;
    }

    for (std::size_t index = 0U; index < sMaxSlots; ++index)
    {
        if ((inSlots[index].hspi == inHspi) && (inSlots[index].callback != nullptr))
        {
            inSlots[index].callback(inHspi, inSlots[index].context);
            return;
        }
    }
}
