#include "usb_config.h"
#include "usb_errno.h"
#include "usb_osal.h"
#include "usb_util.h"
#include "usbh_eth_shared_buf.h"

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif

uint8_t *g_usbh_eth_shared_rx_buffer;
uint8_t *g_usbh_eth_shared_tx_buffer;
uint8_t *g_usbh_eth_shared_int_buffer;
uint8_t *g_usbh_eth_shared_ctrl_buffer;

#ifdef CONFIG_USBHOST_ETH_SHARED_BUF_DYNAMIC
static void *g_usbh_eth_shared_rx_buffer_raw;
static void *g_usbh_eth_shared_tx_buffer_raw;
static void *g_usbh_eth_shared_int_buffer_raw;
static void *g_usbh_eth_shared_ctrl_buffer_raw;
static uint32_t g_usbh_eth_shared_buf_refcount;
#else
#if USBH_ETH_SHARED_RX_SIZE > 0U
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_rx_buffer_static[USBH_ETH_SHARED_RX_SIZE];
#endif
#if USBH_ETH_SHARED_TX_SIZE > 0U
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_tx_buffer_static[USBH_ETH_SHARED_TX_SIZE];
#endif
#if USBH_ETH_SHARED_INT_SIZE > 0U
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_int_buffer_static[USBH_ETH_SHARED_INT_SIZE];
#endif
#if USBH_ETH_SHARED_CTRL_SIZE > 0U
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_usbh_eth_shared_ctrl_buffer_static[USBH_ETH_SHARED_CTRL_SIZE];
#endif
#endif

#ifdef CONFIG_USBHOST_ETH_SHARED_BUF_DYNAMIC
static int usbh_eth_shared_buf_alloc_one(uint8_t **buffer, void **buffer_raw, uint32_t size)
{
    uint32_t alloc_size;

    if (size == 0U) {
        *buffer = NULL;
        *buffer_raw = NULL;
        return 0;
    }

    alloc_size = USB_ALIGN_UP(size, CONFIG_USB_ALIGN_SIZE);

#ifdef ESP_PLATFORM
    *buffer = heap_caps_aligned_alloc(CONFIG_USB_ALIGN_SIZE, alloc_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    *buffer_raw = *buffer;
#else
    *buffer_raw = usb_osal_malloc(alloc_size + CONFIG_USB_ALIGN_SIZE - 1U);
    if (*buffer_raw != NULL) {
        *buffer = (uint8_t *)USB_ALIGN_UP((uintptr_t)*buffer_raw, CONFIG_USB_ALIGN_SIZE);
    } else {
        *buffer = NULL;
    }
#endif

    if (*buffer == NULL) {
        return -USB_ERR_NOMEM;
    }

    memset(*buffer, 0, alloc_size);
    return 0;
}

static void usbh_eth_shared_buf_free_one(uint8_t **buffer, void **buffer_raw)
{
    if (*buffer_raw == NULL) {
        *buffer = NULL;
        return;
    }

#ifdef ESP_PLATFORM
    heap_caps_free(*buffer_raw);
#else
    usb_osal_free(*buffer_raw);
#endif

    *buffer = NULL;
    *buffer_raw = NULL;
}
#endif

int usbh_eth_shared_buf_alloc(void)
{
#ifdef CONFIG_USBHOST_ETH_SHARED_BUF_DYNAMIC
    int ret;

    if (g_usbh_eth_shared_buf_refcount > 0U) {
        g_usbh_eth_shared_buf_refcount++;
        return 0;
    }

    ret = usbh_eth_shared_buf_alloc_one(&g_usbh_eth_shared_rx_buffer, &g_usbh_eth_shared_rx_buffer_raw, USBH_ETH_SHARED_RX_SIZE);
    if (ret < 0) {
        goto errout;
    }

    ret = usbh_eth_shared_buf_alloc_one(&g_usbh_eth_shared_tx_buffer, &g_usbh_eth_shared_tx_buffer_raw, USBH_ETH_SHARED_TX_SIZE);
    if (ret < 0) {
        goto errout;
    }

    ret = usbh_eth_shared_buf_alloc_one(&g_usbh_eth_shared_int_buffer, &g_usbh_eth_shared_int_buffer_raw, USBH_ETH_SHARED_INT_SIZE);
    if (ret < 0) {
        goto errout;
    }

    ret = usbh_eth_shared_buf_alloc_one(&g_usbh_eth_shared_ctrl_buffer, &g_usbh_eth_shared_ctrl_buffer_raw, USBH_ETH_SHARED_CTRL_SIZE);
    if (ret < 0) {
        goto errout;
    }

    g_usbh_eth_shared_buf_refcount = 1U;
    return 0;

errout:
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_ctrl_buffer, &g_usbh_eth_shared_ctrl_buffer_raw);
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_int_buffer, &g_usbh_eth_shared_int_buffer_raw);
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_tx_buffer, &g_usbh_eth_shared_tx_buffer_raw);
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_rx_buffer, &g_usbh_eth_shared_rx_buffer_raw);
    return ret;
#else
#if USBH_ETH_SHARED_RX_SIZE > 0U
    g_usbh_eth_shared_rx_buffer = g_usbh_eth_shared_rx_buffer_static;
#else
    g_usbh_eth_shared_rx_buffer = NULL;
#endif
#if USBH_ETH_SHARED_TX_SIZE > 0U
    g_usbh_eth_shared_tx_buffer = g_usbh_eth_shared_tx_buffer_static;
#else
    g_usbh_eth_shared_tx_buffer = NULL;
#endif
#if USBH_ETH_SHARED_INT_SIZE > 0U
    g_usbh_eth_shared_int_buffer = g_usbh_eth_shared_int_buffer_static;
#else
    g_usbh_eth_shared_int_buffer = NULL;
#endif
#if USBH_ETH_SHARED_CTRL_SIZE > 0U
    g_usbh_eth_shared_ctrl_buffer = g_usbh_eth_shared_ctrl_buffer_static;
#else
    g_usbh_eth_shared_ctrl_buffer = NULL;
#endif
    return 0;
#endif
}

void usbh_eth_shared_buf_free(void)
{
#ifdef CONFIG_USBHOST_ETH_SHARED_BUF_DYNAMIC
    if (g_usbh_eth_shared_buf_refcount == 0U) {
        return;
    }

    g_usbh_eth_shared_buf_refcount--;
    if (g_usbh_eth_shared_buf_refcount > 0U) {
        return;
    }

    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_ctrl_buffer, &g_usbh_eth_shared_ctrl_buffer_raw);
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_int_buffer, &g_usbh_eth_shared_int_buffer_raw);
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_tx_buffer, &g_usbh_eth_shared_tx_buffer_raw);
    usbh_eth_shared_buf_free_one(&g_usbh_eth_shared_rx_buffer, &g_usbh_eth_shared_rx_buffer_raw);
#endif
}

bool usbh_eth_shared_rx_buffer_overlaps(const uint8_t *buf, uint32_t len)
{
    uintptr_t start;
    uintptr_t end;
    uintptr_t shared_start;
    uintptr_t shared_end;

    if ((buf == NULL) || (len == 0U) || (USBH_ETH_SHARED_RX_SIZE == 0U) || (g_usbh_eth_shared_rx_buffer == NULL)) {
        return false;
    }

    shared_start = (uintptr_t)g_usbh_eth_shared_rx_buffer;
    if ((UINTPTR_MAX - shared_start) < (USBH_ETH_SHARED_RX_SIZE - 1U)) {
        return false;
    }

    shared_end = shared_start + USBH_ETH_SHARED_RX_SIZE - 1U;
    start = (uintptr_t)buf;
    if ((UINTPTR_MAX - start) < (len - 1U)) {
        return false;
    }

    end = start + len - 1U;
    return (start <= shared_end) && (end >= shared_start);
}
