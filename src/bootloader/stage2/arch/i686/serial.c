#include "../../drivers/serial.h"
#include "../../stdint.h"
#include "io.h"

#define COM1_BASE_PORT 0x3F8
#define UART_REG_RCVR_BUF (COM1_BASE_PORT)
#define UART_REG_TRANS_HOLDING (COM1_BASE_PORT)
#define UART_REG_INT_ENABLE (COM1_BASE_PORT+0x1)
#define UART_REG_INT_ID (COM1_BASE_PORT+0x2)
#define UART_REG_FIFO_CTL (COM1_BASE_PORT+0x2)
#define UART_REG_LINE_CTL (COM1_BASE_PORT+0x3)
#define UART_REG_MODEM_CTL (COM1_BASE_PORT+0x4)
#define UART_REG_LINE_STS (COM1_BASE_PORT+0x5)
#define UART_REG_MODEM_STS (COM1_BASE_PORT+0x6)
#define UART_REG_SCRATCH (COM1_BASE_PORT+0x7)

void serial_init()
{
    // Set up BAUD generator

    // Turn on DLAB (Divisor Latch Access Bit)
    u8 old_val = inb(UART_REG_LINE_CTL);
    outb(UART_REG_LINE_CTL, (old_val | (1<<7))); // Write bit 7 to LCR to set DLAB on
    
    // After turning DLAB on, RBR (Receiver Buffer) and THR (Transmitter Holding) will be DLL (Divisor Latch LSB) and DLM (Divisor Latch MSB), respectively.
    outb(UART_REG_RCVR_BUF, 0x1); // Divisor = 1 => Baud Rate = 128000
    outb(UART_REG_TRANS_HOLDING, 0); // Set MSB to 0 just to be sure

    // Set up 8N1 configuration
    
    // Turn off DLAB
    old_val = inb(UART_REG_LINE_CTL);
    outb(UART_REG_LINE_CTL, (old_val & ~(1<<7)));

    // 8 is for 8 bits of data (1 character)
    // N is for No Parity
    // 1 is for 1 stop bit
    
    // In LCR:
    // Bit 0 and 1 controls the number of bits you can send.
    // Bit 2 controls the number of stop bits.
    // Bit 3 is the Parity Enable bit.
    old_val = inb(UART_REG_LINE_CTL);
    old_val |= 0b1100; // Bit 0 and 1 = 0b11 -> 8 bits, Bit 2 = 0 -> 1 stop, Bit 3 = 0 -> No Parity
    outb(UART_REG_LINE_CTL, old_val);

    // Enable FIFO
    outb(UART_REG_FIFO_CTL, 0x1);
}

void serial_putch(char ch)
{
    // Poll until the THR is empty
    while (inb(UART_REG_TRANS_HOLDING) & (1<<5));

    // Write char
    outb(UART_REG_RCVR_BUF, ch);
}