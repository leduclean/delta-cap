#include "magnet.h"
#include "main.h"

void magnet_take()
{
	HAL_GPIO_WritePin(MAGNET_SIG_GPIO_Port, MAGNET_SIG_Pin, GPIO_PIN_SET);
}
void magnet_release()
{
	HAL_GPIO_WritePin(MAGNET_SIG_GPIO_Port, MAGNET_SIG_Pin, GPIO_PIN_RESET);
}
