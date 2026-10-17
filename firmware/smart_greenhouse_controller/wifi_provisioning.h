#pragma once

/*
 * Smart Greenhouse Controller
 * Wi-Fi Provisioning Module
 *
 * Handles:
 * - connection to previously saved Wi-Fi
 * - captive portal setup
 * - Wi-Fi connection status
 * - credential reset for maintenance/testing
 */


/*
 * Initialize Wi-Fi.
 *
 * Returns true when the controller successfully
 * connects to a Wi-Fi network.
 *
 * Returns false if provisioning or connection
 * times out.
 */
bool initializeWiFiProvisioning();


/*
 * Returns true when Wi-Fi is currently connected.
 */
bool isWiFiConnected();


/*
 * Prints the current network information
 * to the Serial Monitor.
 */
void printWiFiStatus();


/*
 * Deletes saved Wi-Fi credentials.
 *
 * Intended only for maintenance/testing.
 */
void clearWiFiCredentials();

/*
 * Processes the non-blocking Wi-Fi configuration portal.
 *
 * Must be called repeatedly from loop().
 */
void processWiFiProvisioning();