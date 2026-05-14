/* =============================================================================
 * mcp3208.c  —  Treiber-Implementierung für den MCP3208 12-Bit ADC
 * =============================================================================
 *
 * DIESES FILE IST KOMPLETT USER-CODE (NICHT VON CUBEMX GENERIERT).
 *
 * Alle Funktionen kommunizieren über SPI1 (hspi1, initialisiert von CubeMX).
 * Die CS-Leitung (PB6) wird manuell (Software-NSS) gesteuert.
 * =========================================================================== */

#include "mcp3208.h"
#include "spi.h"    /* hspi1-Handle (von CubeMX generiert, in spi.c definiert) */
#include <stdio.h>
/* ===========================================================================
 * MCP3208_Init
 * ===========================================================================
 * Stellt sicher, dass der CS-Pin nach dem Reset HIGH ist (MCP3208 inaktiv).
 * Diese Funktion muss NACH MX_GPIO_Init() und NACH MX_SPI1_Init() aufgerufen
 * werden, da der GPIO erst nach MX_GPIO_Init() konfiguriert ist.
 * =========================================================================== */
void MCP3208_Init(void)
{
    /* CS sicher auf HIGH setzen → MCP3208 deselektiert */
    MCP3208_CS_HIGH();

    /* Kurze Pause: MCP3208 benötigt nach Power-On min. 1 ms Einschwingzeit    */
    HAL_Delay(10);
}

/* ===========================================================================
 * MCP3208_ReadRaw
 * ===========================================================================
 * Führt einen vollständigen 3-Byte SPI-Transfer durch und gibt den 12-bit
 * Rohwert zurück.
 *
 * SPI-Protokoll im Detail (Kanal 0 als Beispiel):
 *
 *   tx[0] = 0x06 | (0 >> 2) = 0x06 = 0b00000110
 *           Bit1 = START-Bit (immer 1)
 *           Bit0 = SGL/DIFF  (1 = Single-Ended)
 *           D2   = MSB des Kanals (für Kanal 0..3 = 0, für 4..7 = 1)
 *
 *   tx[1] = (0 & 0x03) << 6 = 0x00 = 0b00000000
 *           D1, D0 = restliche Kanal-Bits
 *
 *   tx[2] = 0x00 (Dummy-Byte, taktet die Antwort-Bits heraus)
 *
 * Antwort in rx[]:
 *   rx[0]: Dummy (MCP3208 noch nicht bereit)
 *   rx[1]: Null-Bit + 4 obere Datenbits (B11..B8)
 *   rx[2]: 8 untere Datenbits (B7..B0)
 *
 * Ergebnis: ((rx[1] & 0x0F) << 8) | rx[2]
 * =========================================================================== */
uint16_t MCP3208_ReadRaw(uint8_t channel)
{
    /* ---- Eingabe validieren ---- */
    if (channel >= MCP3208_NUM_CHANNELS)
    {
        return 0xFFFFU;  /* Fehler: Ungültiger Kanal */
    }

    /* ---- TX-Bytes aufbauen ---- */
    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};

    /*
     * Byte 0: Start-Bit (immer 1) + SGL=1 (Single-Ended) + D2 (Bit 2 des Kanals)
     *   0x06 = 0b00000110
     *   channel >> 2: extrahiert Bit 2 des Kanals (nur bei Kanal 4..7 = 1)
     */
    tx[0] = 0x06U | (channel >> 2U);

    /*
     * Byte 1: D1 und D0 (Bits 1..0 des Kanals), in die oberen 2 Bit geschoben
     *   channel & 0x03: maskiert die unteren 2 Kanal-Bits
     *   << 6: schiebt sie auf Bitposition 7 und 6
     */
    tx[1] = (uint8_t)((channel & 0x03U) << 6U);

    /*
     * Byte 2: Dummy → wird nur gesendet, um 8 weitere Taktimpulse zu erzeugen
     *   (der MCP3208 sendet dabei die unteren 8 Datenbits zurück)
     */
    tx[2] = 0x00U;

    /* ---- SPI-Transfer ---- */
    MCP3208_CS_LOW();   /* CS aktiv (LOW) → Kommunikation beginnt            */

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
        &hspi1,         /* SPI-Handle (von CubeMX in spi.c konfiguriert)     */
        tx,             /* Sendepuffer                                        */
        rx,             /* Empfangspuffer                                     */
        3U,             /* 3 Bytes übertragen                                 */
        100U            /* Timeout in ms                                      */
    );

    /* nach HAL_SPI_TransmitReceive(...) und vor MCP3208_CS_HIGH() */
    extern UART_HandleTypeDef huart2;
    char dbg[64];
    int n = snprintf(dbg, sizeof(dbg), 
    "SPI rx: %02X %02X %02X\r\n", rx[0], rx[1], rx[2]);
    HAL_UART_Transmit(&huart2, (uint8_t*)dbg, n, 100);
    MCP3208_CS_HIGH();  /* CS inaktiv (HIGH) → Kommunikation beendet         */

    /* ---- Fehlerbehandlung ---- */
    if (status != HAL_OK)
    {
        return 0xFFFFU;
    }

    /* ---- 12-bit Ergebnis zusammensetzen ---- */
    /*
     * rx[1] enthält 4 obere Datenbits an Positionen 3..0 (Bits 11..8).
     *   (rx[1] & 0x0F) maskiert nur diese 4 Bits
     *   << 8: schiebt sie an die richtige Position
     * rx[2] enthält die 8 unteren Datenbits (Bits 7..0) direkt.
     */
    return (uint16_t)(((uint16_t)(rx[1] & 0x0FU) << 8U) | (uint16_t)rx[2]);
}

/* ===========================================================================
 * MCP3208_ReadVoltage
 * ===========================================================================
 * Wrapper um MCP3208_ReadRaw() mit automatischer Spannungsumrechnung.
 *
 * Formel: U [V] = Rohwert × (VREF / 4095)
 *   Beispiel: Rohwert = 2047 → 2047 × (3.3 / 4095) ≈ 1.648 V
 * =========================================================================== */
MCP3208_Status_t MCP3208_ReadVoltage(uint8_t channel, MCP3208_Result_t *result)
{
    if (result == NULL)
    {
        return MCP3208_ERROR;
    }

    uint16_t raw = MCP3208_ReadRaw(channel);

    if (raw == 0xFFFFU)
    {
        result->raw     = 0U;
        result->voltage = 0.0f;
        return MCP3208_ERROR;
    }

    result->raw     = raw;
    result->voltage = (float)raw * (MCP3208_VREF_V / (float)MCP3208_MAX_VALUE);

    return MCP3208_OK;
}

/* ===========================================================================
 * MCP3208_ReadAllChannels
 * ===========================================================================
 * Liest alle 8 Kanäle nacheinander (CH0 → CH7).
 * Nützlich für Datenlogger-Anwendungen (vgl. Erweiterungsideen im Experiment).
 * =========================================================================== */
MCP3208_Status_t MCP3208_ReadAllChannels(MCP3208_Result_t results[MCP3208_NUM_CHANNELS])
{
    if (results == NULL)
    {
        return MCP3208_ERROR;
    }

    MCP3208_Status_t overall = MCP3208_OK;

    for (uint8_t ch = 0U; ch < MCP3208_NUM_CHANNELS; ch++)
    {
        if (MCP3208_ReadVoltage(ch, &results[ch]) != MCP3208_OK)
        {
            overall = MCP3208_ERROR;
        }
    }

    return overall;
}
