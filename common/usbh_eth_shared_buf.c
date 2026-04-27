#include "usb_config.h"
#include "usb_util.h"
#include "usbh_eth_shared_buf.h"

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_rx_buffer[USBH_ETH_SHARED_RX_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_tx_buffer[USBH_ETH_SHARED_TX_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_int_buffer[USBH_ETH_SHARED_INT_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_ctrl_buffer[USBH_ETH_SHARED_CTRL_SIZE];

bool usbh_eth_shared_rx_buffer_contains(const uint8_t *buf, uint32_t len)
{
    uintptr_t start;
    uintptr_t end;
    uintptr_t shared_start = (uintptr_t)g_usbh_eth_shared_rx_buffer;
    uintptr_t shared_end = shared_start + USBH_ETH_SHARED_RX_SIZE;

    if ((buf == NULL) || (len == 0U)) {
        return false;
    }

    start = (uintptr_t)buf;
    end = start + len;

    if (end < start) {
        return false;
    }

    return (start >= shared_start) && (end <= shared_end);
}
