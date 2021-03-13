#include "stm32f10x.h"


#define APB1CLOCK               (36000000)
#define LIN_BAUD_RATE           (9600)
#define LIN_WORLD_LENGTH        (8U)
#define LIN_RX_EN               ((uint16_t)0x04)
#define LIN_TX_EN               ((uint16_t)0x08)

#define USART2ID                (0x01)
#define DATAQTY                 (5)


#define LIN_STOP_BITS_MASK      ((uint16_t)0x3000)



static void My_LIN_Response(void);
static void GPIO_Init_UARTs(void);
static void My_LIN_Init(void);
static void My_LIN_Write(void);

static uint8_t pDaraTX[DATAQTY] = {1U, 2U, 3U, 4U, 5U};

static void GPIO_Init_UARTs(void)
{
    // Set PORT A clock enable
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    // Set PORT B clock enable
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    // Set both UARTs TX in ouput push-pull
    // USART 3    
    GPIOB->CRH |= GPIO_CRH_MODE10_0;
    // USART 2
    GPIOA->CRL |= GPIO_CRL_MODE2_0;
    
    // Set both UARTs RX in input mode
    // USART 3
    GPIOB->CRH &= ~GPIO_CRH_MODE11;
    // USART 2
    GPIOA->CRL &= ~GPIO_CRL_MODE3;
    
    // Enable altrenative function Rx in floating input
    // USART 3
    GPIOB->CRH |= (GPIO_CRH_CNF11_0);
    // USART 2
    GPIOA->CRL |= (GPIO_CRL_CNF3_0);
    
    // Enable alt func TX in alternative push-pull
    // USART 3
    GPIOB->CRH |= GPIO_CRH_CNF10_1;
    GPIOB->CRH &= ~(GPIO_CRH_CNF10_0);
    // USART 2
    GPIOA->CRL |= GPIO_CRL_CNF2_1;
    GPIOA->CRL &= ~(GPIO_CRL_CNF2_0);
    
    // Set clock
    // USART 3
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    // USART 2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
}

// UART3 - master, UART2 - slave
// USART3 RX - PB11 , TX - PB10
// USART2 RX - PA3  , TX - PA2
static void My_LIN_Init(void)
{
    // init GPIO ports and clocks USART1, USART2
    GPIO_Init_UARTs();
    
    // Set 1 stop bit
    USART3->CR2 &= ~LIN_STOP_BITS_MASK; 
    USART2->CR2 &= ~LIN_STOP_BITS_MASK;
    
    // Transmitter and reciver enable
    // Parity off and World Length is 8 bit (reset state)
    USART3->CR1 |= LIN_RX_EN | LIN_TX_EN;
    USART2->CR1 |= LIN_RX_EN | LIN_TX_EN;
    
    // CR3 reg while not change. Reset state is 0x0000
   
    // BRR = 1D4C
    /*
    1:
        BRR = (Fck + baudrate/2) / baudrate
        
    2:
        Mantissa BRR = Fck/(16 * baudrate)
        Dev BRR: 16 * Mantissa BRR
        Itog BRR: Mantissa BRR Dev BRR;
    */

    uint16_t tmpreg = ((APB1CLOCK + (LIN_BAUD_RATE/2)) / LIN_BAUD_RATE);
   
    /* Write to USART BRR */
    USART3->BRR = (uint16_t)tmpreg;
    USART2->BRR = (uint16_t)tmpreg;
    
    // Reset CLKEN
    // UART 3
    USART3->CR2 &= ~USART_CR2_CLKEN;
    // UART 2
    USART2->CR2 &= ~USART_CR2_CLKEN;
    
    // Enable LIN
    USART3->CR2 |= USART_CR2_LINEN;
    USART2->CR2 |= USART_CR2_LINEN;
    
    // Reset SCEN, HDSEL, IREN
    USART3->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);
    USART2->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);
    
    USART3->CR1 |= USART_CR1_UE;
    USART2->CR1 |= USART_CR1_UE;
}



static void My_LIN_Write(void)
{
    // Set break
    USART3->CR1 |= USART_CR1_SBK;
    
    // Send 0x55
    USART3->DR = 0x55;
    
    // Send ID
    USART3->DR = USART2ID;
    
    
    
}

static void My_LIN_Response(void)
{
    uint16_t data = 0;
    
    data = USART2->DR;
    data = USART2->DR;
    
    if (data == USART2ID)
    {
        for (int i = 0; i < DATAQTY; i++)
        {
            USART2->DR = pDaraTX[i];
        }
    }
}


int main()
{
    My_LIN_Init();
    
    // Lin write: Lin breake, data 0x55, id, data 8 byte
    My_LIN_Write();
    My_LIN_Response();
    return 0;
}
