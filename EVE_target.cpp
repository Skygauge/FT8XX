#include "EVE_target.h"

#define DEBUG_SERIAL false

namespace EVE
{
	/* Callback for end-of-DMA-transfer */
	static void dma_callback(EventResponderRef event_responder)
	{
		((Port*)(event_responder.getContext()))->dma_finish();
	}

	Port::Port(uint8_t CS, uint8_t RESET) : cs(CS), reset(RESET)
    {
        spi_event.setContext(this);
        spi_event.attachImmediate(dma_callback);
        SPI.begin();
    };

    void Port::init(int speed)
    {
        set_speed(speed);
        pinMode(cs, OUTPUT);
        pinMode(reset, OUTPUT);
        cs_clear();
        pdn_set();
        delay(1);
        pdn_clear();
    }

    void Port::set_speed(int speed)
    {
        SPI.endTransaction();
        SPI.beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
    }

    void Port::dma_begin(uint32_t ftAddress)
    {
        buffer[0] = ((uint8_t)(ftAddress >> 16) | MEM_WRITE) | (ftAddress & 0x0000ff00) |  ((uint8_t)(ftAddress) << 16);
        buffer[0] = buffer[0] << 8;
        dma_buffer_index = 1;
    }

	void Port::dma_finish()
	{
		dma_busy = 0;
		cs_clear();
	}

	void Port::dma_transfer()
	{
		cs_set();
		dma_busy = 42;
		SPI.transfer( ((uint8_t *) &buffer[0])+1, NULL, (((dma_buffer_index)*4)-1), spi_event);
        dma_period = 0;
	}

    bool Port::is_dma_busy()
    {
        if(dma_busy && (dma_period > DMA_TIMEOUT)) {
            dma_busy = 0;
#if DEBUG_SERIAL
            Serial.println("DMA TIMEOUT!");
#endif
        }
        return dma_busy;
    }

    void Port::cs_set()
    {
        digitalWriteFast(cs, LOW); /* make EVE listen */
    }

    void Port::cs_clear()
    {
        digitalWriteFast(cs, HIGH); /* tell EVE to stop listen */
    }

    void Port::transmit(uint8_t data)
    {
        SPI.transfer(data);
    }

    void Port::transmit(uint8_t * data, uint len)
    {
        SPI.transfer(data, len);
    }

    void Port::transmit(const uint8_t * data, uint len)
    {
        SPI.transfer(data, NULL, len);
    }

    void Port::transmit_32(uint32_t data)
    {
        transmit((uint8_t)(data));
        transmit((uint8_t)(data >> 8));
        transmit((uint8_t)(data >> 16));
        transmit((uint8_t)(data >> 24));
    }

    void Port::transmit_dma(uint32_t data)
    {
        buffer[dma_buffer_index++] = data;
    }

    uint8_t Port::receive(uint8_t data)
    {
        return SPI.transfer(data);
    }

    void Port::pdn_set()
    {
        digitalWriteFast(reset, LOW); /* go into power-down */
    }

    void Port::pdn_clear()
    {
        digitalWriteFast(reset, HIGH); /* power up */
    }
}
