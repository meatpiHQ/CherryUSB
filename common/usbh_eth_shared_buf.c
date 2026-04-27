#include "usb_config.h"
#include "usb_util.h"
#include "usbh_eth_shared_buf.h"

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_rx_buffer[USBH_ETH_SHARED_RX_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_tx_buffer[USBH_ETH_SHARED_TX_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_int_buffer[USBH_ETH_SHARED_INT_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_ctrl_buffer[USBH_ETH_SHARED_CTRL_SIZE];

bool usbh_eth_shared_rx_buffer_overlaps(const uint8_t *buf, uint32_t len)
{
    uintptr_t start;
    uintptr_t end;
    uintptr_t shared_start = (uintptr_t)g_usbh_eth_shared_rx_buffer;
    uintptr_t shared_end;

    if ((buf == NULL) || (len == 0U) || (USBH_ETH_SHARED_RX_SIZE == 0U)) {
        return false;
    }

    shared_end = shared_start + USBH_ETH_SHARED_RX_SIZE - 1U;
    start = (uintptr_t)buf;
    if (((uintptr_t)len - 1U) > (UINTPTR_MAX - start)) {
        return false;
    }

    end = start + len - 1U;
    return (start <= shared_end) && (end >= shared_start);
}
