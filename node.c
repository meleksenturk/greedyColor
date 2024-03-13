#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
#endif /* MAC_CONF_WITH_TSCH */

// static linkaddr_t coordinator_addr1 = {{0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
// static linkaddr_t coordinator_addr2 = {{0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

struct NodeColor {
  char *color;
  double value;
};

static struct NodeColor colorAndValue[] = {
  {"kirmizi", 15},
  {"siyah", 10},
  {"mavi", 20},
  {"pembe", 8},
  // {"sari",3}
};
static unsigned min_value = 0;
static unsigned colorArraySize = sizeof(colorAndValue) / sizeof(colorAndValue[0]);

static int getIndexOfMinColor();
static int packetReceived = 0;
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void updateColorArray(unsigned received_value) {
  int receivedIndex = -1;
  for (int i = 0; i < colorArraySize; i++) {
    if (colorAndValue[i].value == received_value) {
      receivedIndex = i;
      break;
    }
  }

  if (receivedIndex != -1) {
    LOG_INFO("Removing color: %s\n", colorAndValue[receivedIndex].color);

    for (int i = receivedIndex; i < colorArraySize-1; i++) {
      colorAndValue[i] = colorAndValue[i + 1];
    }

    colorArraySize--;
  }
}





void input_callback(const void *data, uint16_t len,
                    const linkaddr_t *src, const linkaddr_t *dest) {
  if (len == sizeof(unsigned)) {
    // LOG_INFO("melek");

    unsigned received_value;
    memcpy(&received_value, data, sizeof(received_value));

      LOG_INFO("Received valueee %u from ", received_value);
      LOG_INFO_LLADDR(src);
      LOG_INFO_(" in a single hop.\n");

      updateColorArray(received_value);
      // sendUpdatedArray();
      int minColorIndex = getIndexOfMinColor();
      // LOG_INFO("Meeleeek: %d",minColorIndex);
    if (minColorIndex != -1) {
      LOG_INFO("Min Color Index: %d\n", minColorIndex);

      LOG_INFO("My Color: %s\n", colorAndValue[minColorIndex].color);

    }
        packetReceived = 1;

  }
}

static unsigned minColorValue() {
  min_value = colorAndValue[0].value;

  for (int i = 1; i < colorArraySize; i++) {
    if (colorAndValue[i].value < min_value) {
      min_value = colorAndValue[i].value;
    }
  }

  LOG_INFO("Min Value:%u\n", min_value);

  return min_value;
}

static int getIndexOfMinColor() {
  min_value = minColorValue();

  for (int i = 0; i < colorArraySize; i++) {
    if (colorAndValue[i].value == min_value) {
      return i;
    }
  }
  return -1;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data) {
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&min_value;
  nullnet_len = sizeof(min_value);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);

  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    // int minColorIndex = getIndexOfMinColor();
    // if (minColorIndex != -1) {
    //   LOG_INFO("Min Color Index: %d\n", minColorIndex);
    //   LOG_INFO("My Color: %s\n", colorAndValue[minColorIndex].color);
    // }

    LOG_INFO("Sending %u to ", min_value);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");

    memcpy(nullnet_buf, &min_value, sizeof(min_value));
    nullnet_len = sizeof(min_value);
    NETSTACK_NETWORK.output(NULL);
    

   packetReceived = 0;

     while (!packetReceived) { //paket alinana kadar bekle
      PROCESS_WAIT_EVENT();
    }

    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
