#include "uart.h"
#include "cmsis_os.h"
#include "main.h"
#include "stm32f4xx.h"

#define UART_BUFFER_SIZE 128
#define timeout (portMAX_DELAY)
#define USE_INTERRUPTS 0

#if USE_INTERRUPTS

typedef struct {
	uint8_t buffer[UART_BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
} ring_buffer_t;

static UART_HandleTypeDef UartHandle;

static ring_buffer_t rxBuffer = {0};
static ring_buffer_t txBuffer = {0};

static uint8_t rxByte;
static uint8_t txByte;

/* Semaphores */
static xSemaphoreHandle rxSem;     // counts available bytes
static xSemaphoreHandle txSem;     // counts free space
static xSemaphoreHandle txDoneSem; // signals TX completion/idle

app_err_t uart_init() {
	/* Create semaphores */
	rxSem = xSemaphoreCreateCounting(UART_BUFFER_SIZE, 0);
	if (rxSem == NULL) {
		APP_FAIL;
	}
	txSem = xSemaphoreCreateCounting(UART_BUFFER_SIZE, UART_BUFFER_SIZE);
	if (txSem == NULL) {
		APP_FAIL;
	}
	txDoneSem = xSemaphoreCreateCounting(1, 1);
	if (txDoneSem == NULL) {
		APP_FAIL;
	}

	if (HAL_OK != HAL_UART_Receive_IT(&UartHandle, &rxByte, 1)) {
		APP_FAIL;
	}
	return APP_OK;
}

static void ringPush(ring_buffer_t *rb, uint8_t data) {
	rb->buffer[rb->head] = data;
	rb->head = (rb->head + 1) % UART_BUFFER_SIZE;
}

static uint8_t ringPop(ring_buffer_t *rb) {
	uint8_t data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % UART_BUFFER_SIZE;
	return data;
}

app_err_t uart_send(char* pstring, uint16_t size) {
	for (uint16_t i = 0; i < size; i++) {
		if (pstring[i] == 0x00) break;

		if (xSemaphoreTake(txSem, timeout) != pdPASS) {
			return APP_ERR_INTERNAL; // timeout waiting for space
		}

		ringPush(&txBuffer, pstring[i]);
	}

	/* Trigger transmission if idle */
	if (xSemaphoreTake(txDoneSem, 0) == pdPASS) {
		txByte = ringPop(&txBuffer);
		HAL_UART_Transmit_IT(&UartHandle, &txByte, 1);
	}

	return APP_OK;
}

app_err_t uart_receive(char* pstring, uint16_t size) {
	for (uint16_t i = 0; i < size; i++) {

		if (xSemaphoreTake(rxSem, timeout) != pdPASS) {
			return APP_ERR_INTERNAL; // timeout
		}

		pstring[i] = ringPop(&rxBuffer);
	}

	return APP_OK;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {

		ringPush(&rxBuffer, rxByte);

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(rxSem, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		HAL_UART_Receive_IT(&UartHandle, &rxByte, 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {

		if (txBuffer.head != txBuffer.tail) {

			txByte = ringPop(&txBuffer);
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;

			xSemaphoreGiveFromISR(txSem, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

			HAL_UART_Transmit_IT(&UartHandle, &txByte, 1);

		} else {
			/* Transmission finished */
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;

			xSemaphoreGiveFromISR(txDoneSem, &xHigherPriorityTaskWoken);
			xSemaphoreGiveFromISR(txSem, &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			// last byte freed
		}
	}
}

#else
#include "stm32f4xx_hal.h"
#include "string.h"
#include <stdio.h>
#include <stdbool.h>
static UART_HandleTypeDef UartHandle;
const uint32_t TIMEOUT = 3000;
static void errorIfNeeded(void* delay) {
	if (delay == NULL) {
		Error_Handler();
	}
}
static uint16_t findCharIndex(char *pstring, char charToFind, uint16_t max) {
	uint16_t i = 0;
	uint8_t actual = pstring[i];
	while (actual != charToFind) {
		i++;
		actual = pstring[i];
		if (i >= max) {
			Error_Handler();
		}
	}
	return i;
}

app_err_t uart_send(char* pstring, uint16_t size) {
	errorIfNeeded(pstring);
	uint16_t i = findCharIndex(pstring, '\0', size + 5);
	return (HAL_OK == HAL_UART_Transmit(&UartHandle, pstring, i < size ? i : size, TIMEOUT)) ? APP_OK : APP_ERR_INTERNAL;
}

app_err_t uart_receive(char* pstring, uint16_t size) {
	errorIfNeeded(pstring);
	return (HAL_OK == HAL_UART_Receive(&UartHandle, pstring, size, TIMEOUT)) ? APP_OK : APP_ERR_INTERNAL;
}

app_err_t uart_init() {
	UartHandle.Instance = USART2;
	UartHandle.Init.BaudRate = 115200;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	bool status = HAL_UART_Init(&UartHandle) == HAL_OK;
	if (status) {
		char uart_json[256];

		snprintf(uart_json, sizeof(uart_json),
			"\r\033[2J{"
			"\"BaudRate\":%ld,"
			"\"WordLength\":\"%ld\","
			"\"StopBits\":\"%ld\","
			"\"Parity\":\"%ld\","
			"\"HwFlowCtl\":\"%ld\","
			"\"Mode\":\"%ld\","
			"\"Instance\":\"%s\""
			"}\n",
			UartHandle.Init.BaudRate,
			UartHandle.Init.WordLength,
			UartHandle.Init.StopBits,
			UartHandle.Init.Parity,
			UartHandle.Init.HwFlowCtl,
			UartHandle.Init.Mode,
			"USART2"
		);
		uint8_t i = findCharIndex(uart_json, '\n', 256);
		uart_send(uart_json, i+1);
	};
	return status ? APP_OK : APP_ERR_INTERNAL;
}
#endif
