# LIN_Test

LIN protocol test projects

Device: STM32F103C8T6

TASK:
Set LED output in LOW level. 
UART3 - master, UART2 - slave
Transmit data from LIN from UART3 to request data from slave. 
Slave get data.
If master receive valid data, then set LED ouput in HIGH level.
