/* =============================================================================
 * mcp3208.h  —  Treiber-Header für den MCP3208 12-Bit ADC (8 Kanäle, SPI)
 * =============================================================================
 *
 * DIESES FILE IST KOMPLETT USER-CODE (NICHT VON CUBEMX GENERIERT).
 *
 * Hardware-Zuordnung (gemäß Experimentdokumentation EXP 02):
 *   SPI1 SCK   → PA5  (D13)
 *   SPI1 MOSI  → PA7  (D11)
 *   SPI1 MISO  → PA6  (D12)
 *   CS/SHDN    → PB6  (D10)   ← Software-CS, GPIO Output
 *   CH0        → Potentiometer Abgriff (Wiper)
 *
 * MCP3208 Protokoll (3-Byte SPI-Transfer, MSB first):
 *   TX Byte 0: [0][0][0][0][0][1][SGL][D2]   Start=1, SGL=1 (Single-ended)
 *   TX Byte 1: [D1][D0][x][x][x][x][x][x]    Kanal-Bits D1..D0
 *   TX Byte 2: [0][0][0][0][0][0][0][0]       Dummy (taktet Antwort heraus)
 *
 *   RX Byte 1: [...][...][B11][B10][B9][B8]   Obere 4 Nutzbits (Bits 11..8)
 *   RX Byte 2: [B7][B6][B5][B4][B3][B2][B1][B0]  Untere 8 Bits
 *   Ergebnis:  ((rx[1] & 0x0F) << 8) | rx[2]  → 0 … 4095
 * =========================================================================== */

#ifndef INC_MCP3208_H_
#define INC_MCP3208_H_

/* ---- Includes ------------------------------------------------------------ */
#include "main.h"   /* HAL-Typen, SPI-Handle-Deklarationen                   */

/* ---- Konstanten ---------------------------------------------------------- */

/** Anzahl der Kanäle des MCP3208 */
#define MCP3208_NUM_CHANNELS    8U

/** Maximaler Rohwert (12-bit → 2^12 - 1) */
#define MCP3208_MAX_VALUE       4095U

/** Referenzspannung in Volt (VDD = VREF = 3,3 V gemäß Verdrahtungstabelle) */
#define MCP3208_VREF_V          3.3f

/* ---- CS-Pin Makros (CS = Chip Select, aktiv LOW) ------------------------ */
/*
 * [USER-CODE] Diese Makros steuern den CS-Pin (PB6 / D10).
 * CS LOW  → MCP3208 ausgewählt, Kommunikation aktiv
 * CS HIGH → MCP3208 deselektiert, SPI-Bus freigegeben
 */
#define MCP3208_CS_LOW()   HAL_GPIO_WritePin(MCP3208_CS_GPIO_Port, \
                                              MCP3208_CS_Pin,       \
                                              GPIO_PIN_RESET)

#define MCP3208_CS_HIGH()  HAL_GPIO_WritePin(MCP3208_CS_GPIO_Port, \
                                              MCP3208_CS_Pin,       \
                                              GPIO_PIN_SET)

/* ---- Rückgabetypen ------------------------------------------------------- */

/**
 * @brief Statuscode für MCP3208-Operationen
 */
typedef enum {
    MCP3208_OK    = 0,  /*!< Kommunikation erfolgreich          */
    MCP3208_ERROR = 1   /*!< SPI-Fehler oder ungültiger Kanal   */
} MCP3208_Status_t;

/**
 * @brief Messergebnis-Struktur (Rohwert + umgerechnete Spannung)
 */
typedef struct {
    uint16_t raw;       /*!< 12-bit Rohwert: 0 … 4095           */
    float    voltage;   /*!< Spannung in Volt: 0.0 … 3.3 V      */
} MCP3208_Result_t;

/* ---- Funktionsprototypen ------------------------------------------------- */

/**
 * @brief  Initialisiert die CS-Leitung (HIGH = inaktiv).
 *         Muss einmalig nach MX_GPIO_Init() aufgerufen werden.
 */
void MCP3208_Init(void);

/**
 * @brief  Liest einen einzelnen Kanal im Single-Ended-Modus.
 *
 * @param  channel  Kanalnummer 0 … 7
 * @return 12-bit Rohwert (0 … 4095), oder 0xFFFF bei Fehler
 */
uint16_t MCP3208_ReadRaw(uint8_t channel);

/**
 * @brief  Liest einen Kanal und berechnet die Spannung in Volt.
 *
 * @param  channel  Kanalnummer 0 … 7
 * @param  result   Zeiger auf MCP3208_Result_t (wird befüllt)
 * @return MCP3208_OK oder MCP3208_ERROR
 */
MCP3208_Status_t MCP3208_ReadVoltage(uint8_t channel, MCP3208_Result_t *result);

/**
 * @brief  Liest alle 8 Kanäle sequentiell.
 *
 * @param  results  Array mit 8 MCP3208_Result_t Einträgen (wird befüllt)
 * @return MCP3208_OK wenn alle Kanäle erfolgreich gelesen wurden
 */
MCP3208_Status_t MCP3208_ReadAllChannels(MCP3208_Result_t results[MCP3208_NUM_CHANNELS]);

#endif /* INC_MCP3208_H_ */
