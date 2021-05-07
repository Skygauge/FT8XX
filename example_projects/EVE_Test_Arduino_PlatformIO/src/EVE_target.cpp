#include "EVE_target.h"

namespace EVE
{
	/* Callback for end-of-DMA-transfer */
	static void dma_callback(EventResponderRef event_responder)
	{
		((Port*)(event_responder.getContext()))->dma_done();
	}

	Port::Port()
	{
		spi_event.setContext(this);
		spi_event.attachImmediate(dma_callback);
	}

	void Port::dma_done(void)
	{
		dma_busy = 0;
		cs_clear();
	}

	void Port::dma_transfer(void)
	{
		cs_set();
		dma_busy = 42;
		SPI.transfer( ((uint8_t *) &buffer[0])+1, NULL, (((dma_buffer_index)*4)-1), spi_event);
	}
}