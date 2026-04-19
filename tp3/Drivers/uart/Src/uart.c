#include "uart.h"
#include "cmsis_os.h"
#include "main.h"
#include "stm32f4xx.h"

#define UART_BUFFER_SIZE 128
#define timeout (portMAX_DELAY)

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