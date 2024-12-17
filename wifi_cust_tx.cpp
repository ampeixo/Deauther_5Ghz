#include "wifi_cust_tx.h"

/*
 * Transmits a raw 802.11 frame.
 * Sequence number and Frame Check Sequence (FCS) are automatically handled.
 *
 * @param frame  Pointer to the raw frame data
 * @param length Length of the frame (excluding FCS)
 */
void wifi_tx_raw_frame(void* frame, size_t length) {
  void *wlan_ptr = (void *)**(uint32_t **)(rltk_wlan_info + 0x10);
  void *tx_frame = alloc_mgtxmitframe(wlan_ptr + 0xae0);

  if (tx_frame) { 
    update_mgntframe_attrib(wlan_ptr, tx_frame + 8);
    void *frame_buffer = (void *)(*(uint32_t *)(tx_frame + 0x80));

    memset(frame_buffer, 0, 0x68); // Clear the buffer
    memcpy((uint8_t *)frame_buffer + 0x28, frame, length); // Copy frame data

    *(uint32_t *)(tx_frame + 0x14) = length;
    *(uint32_t *)(tx_frame + 0x18) = length;

    dump_mgntframe(wlan_ptr, tx_frame);
  }
}

/*
 * Sends a deauthentication (deauth) frame.
 *
 * @param src_mac  6-byte array for the sender's MAC address
 * @param dst_mac  6-byte array for the destination MAC address or FF:FF:FF:FF:FF:FF for broadcast
 * @param reason   Deauth reason code (802.11 specification)
 */
void wifi_tx_deauth_frame(void* src_mac, void* dst_mac, uint16_t reason) {
  DeauthFrame frame = {0}; // Zero initialize the frame

  memcpy(frame.source, src_mac, 6);
  memcpy(frame.access_point, src_mac, 6);
  memcpy(frame.destination, dst_mac, 6);
  frame.reason = reason;

  wifi_tx_raw_frame(&frame, sizeof(DeauthFrame));
}

/*
 * Sends a basic beacon frame with a custom SSID.
 *
 * @param src_mac  6-byte array for the sender's MAC address
 * @param dst_mac  6-byte array for the destination MAC address or FF:FF:FF:FF:FF:FF for broadcast
 * @param ssid     Null-terminated string representing the SSID
 */
void wifi_tx_beacon_frame(void* src_mac, void* dst_mac, const char *ssid) {
  BeaconFrame frame = {0}; // Zero initialize the frame

  memcpy(frame.source, src_mac, 6);
  memcpy(frame.access_point, src_mac, 6);
  memcpy(frame.destination, dst_mac, 6);

  // Copy SSID into the frame and calculate its length
  for (frame.ssid_length = 0; ssid[frame.ssid_length] != '\0'; frame.ssid_length++) {
    frame.ssid[frame.ssid_length] = ssid[frame.ssid_length];
  }

  wifi_tx_raw_frame(&frame, 38 + frame.ssid_length);
}
