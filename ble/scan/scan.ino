/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <bluefruit.h>
#define CFG_DEBUG 3

//BLEClientHidAdafruit hid;
bool connected = false;

/* For a list of EIR data types see:
 *    https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 *    Matching enum: cores/nRF5/SDK/components/softdevice/s132/headers/ble_gap.h */

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10); // for nrf52840 with native usb
  }

  Serial.println("Bluefruit52 Central ADV Scan Example");
  Serial.println("------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4); // Check bluefruit.h for supported values

  /* Set the device name */
  Bluefruit.setName("Bluefruit52");

  /* Set the LED interval for blinky pattern on BLUE LED */
  Bluefruit.setConnLedInterval(250);

  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter out packet with a min rssi
   * - Interval = 100 ms, window = 50 ms
   * - Use active scan (used to retrieve the optional scan response adv packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-80);
  //Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE);
  //Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE); // only invoke callback if detect bleuart service
  Bluefruit.Scanner.setInterval(160, 80); // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);  // Request scan response data
  Bluefruit.Scanner.start(0);             // 0 = Don't stop scanning after n seconds

  Serial.println("Scanning ...");
}

void connect_callback(uint16_t conn_handle)
{
  connected = true;
  Serial.println("Connected");
  // if (!Bluefruit.requestPairing(conn_handle))
  // {
  //   Serial.print("Failed to paired");
  //   return;
  // }
  // else
  // {
  //   Serial.println("paired !");
  // }

  BLEClientService remoteService (0x1812);

  if (remoteService.begin())
  {
    Serial.println("remoteService begin success");
  }
  else
  {
    Serial.println("remoteService begin failure");
  }

  if (remoteService.discover(conn_handle))
  {
    Serial.println("remoteService discover success");
  }
  else
  {
    Serial.println("remoteService discover failure");
  }

  ble_gattc_handle_range_t bck_range = Bluefruit.Discovery.getHandleRange();
  //Bluefruit.Discovery.setHandleRange(remoteService.getHandleRange());

  BLEClientCharacteristic _hidReport(0x2A4D);
  _hidReport.begin(&remoteService);
  BLEClientCharacteristic _hidReportMap(0x2A4E);
  _hidReportMap.begin(&remoteService);
  Bluefruit.Discovery.discoverCharacteristic(remoteService.connHandle(), _hidReport, _hidReportMap);

    // char central_name[32] = { 0 };
    // Bluefruit.Gatt.readCharByUuid(conn_handle, 0x2A4D ,central_name, sizeof(central_name));
    // Serial.printBuffer(central_name, 32);
    // Serial.println(" ");
  //Bluefruit.Discovery.setHandleRange(bck_range);

  // if (myService.discover(conn_handle))
  // {
  //   Serial.println("service discovering success");


  //   Bluefruit.Discovery.discoverCharacteristic(conn_handle, _hidReport);
  //   if (_hidReport.discovered()) {
  //     Serial.println("_hidReport discovered !");
  //     //_hidReport.setNotifyCallback(hid_report_notify_cb);

  //   } else {
  //       Serial.println("_hidReport not discovered :(");
  //   }
    // Bluefruit.disconnect(conn_handle);
  // }
  // else
  // {
  //   Serial.println("service discovering failure");
  // }
}

void hid_report_notify_cb(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{
  Serial.println("hid_report_notify_cb");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void)conn_handle;
  (void)reason;

  Serial.print("Disconnected, reason = 0x");
  Serial.println(reason, HEX);
  connected = false;
}

bool compareArrays(uint8_t a1[], uint8_t a2[], uint8_t length)
{
  //Serial.print("Comparing arrays\n");
  for (uint8_t i = 0; i < length; i++)
  {
    //Serial.printf("a1=%d a2=%d", a1[i], a2[i]);
    if (a1[i] != a2[i])
      return false;
  }
  //Serial.print("\n");

  return true;
}

void scan_callback(ble_gap_evt_adv_report_t *report)
{
  PRINT_LOCATION();
  uint8_t len = 0;
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  uint8_t vr_addr[6] = {0x99, 0x64, 0x01, 0x96, 0xFF, 0xFF};

  if (!compareArrays(report->peer_addr.addr, vr_addr, 6))
  {
    Bluefruit.Scanner.resume();
    return;
  }
  else
  {
    if (connected)
      return;
    Bluefruit.Central.connect(report);
  }
  /* Display the timestamp and device address */
  if (report->type.scan_response)
  {
    Serial.printf("[SR%10d] Packet received from ", millis());
  }
  else
  {
    Serial.printf("[ADV%9d] Packet received from ", millis());
  }
  // MAC is in little endian --> print reverse
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.print("\n");

  /* Raw buffer contents */
  Serial.printf("%14s %d bytes\n", "PAYLOAD", report->data.len);
  if (report->data.len)
  {
    Serial.printf("%15s", " ");
    Serial.printBuffer(report->data.p_data, report->data.len, '-');
    Serial.println();
  }

  /* RSSI value */
  Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);

  /* Adv Type */
  Serial.printf("%14s ", "ADV TYPE");
  if (report->type.connectable)
  {
    Serial.print("Connectable ");
  }
  else
  {
    Serial.print("Non-connectable ");
  }

  if (report->type.directed)
  {
    Serial.println("directed");
  }
  else
  {
    Serial.println("undirected");
  }

  /* Shortened Local Name */
  if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, buffer, sizeof(buffer)))
  {
    Serial.printf("%14s %s\n", "SHORT NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  /* Complete Local Name */
  if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer)))
  {
    Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  /* TX Power Level */
  if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_TX_POWER_LEVEL, buffer, sizeof(buffer)))
  {
    Serial.printf("%14s %i\n", "TX PWR LEVEL", buffer[0]);
    memset(buffer, 0, sizeof(buffer));
  }

  /* Check for UUID16 Complete List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, buffer, sizeof(buffer));
  if (len)
  {
    printUuid16List(buffer, len);
  }

  /* Check for UUID16 More Available List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE, buffer, sizeof(buffer));
  if (len)
  {
    printUuid16List(buffer, len);
  }

  /* Check for UUID128 Complete List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, buffer, sizeof(buffer));
  if (len)
  {
    printUuid128List(buffer, len);
  }

  /* Check for UUID128 More Available List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE, buffer, sizeof(buffer));
  if (len)
  {
    printUuid128List(buffer, len);
  }

  /* Check for BLE UART UUID */
  if (Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE))
  {
    Serial.printf("%14s %s\n", "BLE UART", "UUID Found!");
  }

  /* Check for DIS UUID */
  if (Bluefruit.Scanner.checkReportForUuid(report, UUID16_SVC_DEVICE_INFORMATION))
  {
    Serial.printf("%14s %s\n", "DIS", "UUID Found!");
  }

  /* Check for Manufacturer Specific Data */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));
  if (len)
  {
    Serial.printf("%14s ", "MAN SPEC DATA");
    Serial.printBuffer(buffer, len, '-');
    Serial.println();
    memset(buffer, 0, sizeof(buffer));
  }

  Serial.println();

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  //Bluefruit.Scanner.resume();
}

void printUuid16List(uint8_t *buffer, uint8_t len)
{
  Serial.printf("%14s %s", "16-Bit UUID");
  for (int i = 0; i < len; i += 2)
  {
    uint16_t uuid16;
    memcpy(&uuid16, buffer + i, 2);
    Serial.printf("%04X ", uuid16);
  }
  Serial.println();
}

void printUuid128List(uint8_t *buffer, uint8_t len)
{
  (void)len;
  Serial.printf("%14s %s", "128-Bit UUID");

  // Print reversed order
  for (int i = 0; i < 16; i++)
  {
    const char *fm = (i == 4 || i == 6 || i == 8 || i == 10) ? "-%02X" : "%02X";
    Serial.printf(fm, buffer[15 - i]);
  }

  Serial.println();
}

void loop()
{
  // nothing to do
}