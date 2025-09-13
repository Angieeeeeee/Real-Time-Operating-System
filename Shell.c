// Angelina Abuhilal
// RTOS: mini project

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a
//   virtual COM port. Configured to 115,200 baud, 8N1
// Fault triggers:
//   PBs

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

// Info that can be accepted
#define MAX_CHARS 80
#define MAX_FIELDS 5
#define longestCommand 7

// UI info structure
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

// Bitband aliases
  #define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
  #define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortF masks
  #define GREEN_LED_MASK 8
  #define RED_LED_MASK 2

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure LED pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs

    // Enable fault interrupts for mpu, bus, usage, and hard faults
    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE | NVIC_SYS_HND_CTRL_BUS | NVIC_SYS_HND_CTRL_MEM | NVIC_SYS_HND_CTRL_SVC;
}

void yield(void)
{
    // empty for now (?)
}

// Function that stores the inputed characters
void getsUart0(USER_DATA *data)
{
    int count = 0;
    while (count < MAX_CHARS)
    {
        while (!kbhitUart0())
        {
            yield();
        }

        // get the character input and store in variable
        char c = getcUart0();
        putcUart0(c);

        // backspace and not first char
        if ((c == 8 || c == 127) && count > 0)
        {
            count --;
        }

        // carriage return
        else if (c == 13)
        {
            break;
        }

        // space bar or printable characters
        else if (c >= 32)
        {
            data->buffer[count] = c;
            count ++;
        }

    }
    data->buffer[count] = 0;
    return;
}

void parseFields(USER_DATA *data)
{
    data->fieldCount = 0;

    char current;
    char prev = '\0';

    int i = 0;
    for (i = 0; i < MAX_CHARS && data->buffer[i] != '\0' && data->fieldCount < MAX_FIELDS; i ++)
    {
        bool alpha = (data->buffer[i] > 64 && data->buffer[i] < 91) || (data->buffer[i] > 96 && data->buffer[i] < 123);
        bool numeric = data->buffer[i] > 47 && data->buffer[i] < 58;

        current = data->buffer[i];

        if(prev == '\0' && current != '\0')
                {
                    if(alpha)
                    {
                        data->fieldPosition[data->fieldCount] = i;
                        data->fieldType[data->fieldCount] = 'a';
                        data->fieldCount++;
                    }
                    else if(numeric)
                    {
                        data->fieldPosition[data->fieldCount] = i;
                        data->fieldType[data->fieldCount] = 'n';
                        data->fieldCount++;
                    }
                }
                if(!(alpha || numeric))
                {
                    data->buffer[i] = '\0';
                    current = '\0';
                }
                prev = current;
    }
    return;
}

// function to return the value of a field requested if the field number is in range or NULL otherwise.
char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    if (fieldNumber <= data->fieldCount)
    {
        return &data->buffer[data->fieldPosition[fieldNumber]];
    }
    else
    {
        return NULL;
    }

}

// function to return the integer value of the field if the field number is in range and the field type is numeric or 0 otherwise.
int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    // Check if fieldNumber is within range and if the field type is numeric
        if (fieldNumber > data->fieldCount || data->fieldType[fieldNumber] != 'n')
        {
            return 0; // Return NULL if fieldNumber is out of range or non numeric
        }
        else
        {
            //char* str = &data->buffer[data->fieldPosition[fieldNumber]]; //get the field addy
            return atoi(&data->buffer[data->fieldPosition[fieldNumber]]);
        }
}

int myStrcmp(const char *a, const char *b)
{
    const unsigned char *s1 = (const unsigned char*)a;
    const unsigned char *s2 = (const unsigned char*)b;

    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return (int)(*s1) - (int)(*s2);  // <0 if a<b, 0 if equal, >0 if a>b
}

// function which returns true if the command matches the first field and the number of arguments (excluding the command field) is greater than or equal to the requested number of minimum arguments.
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    // check if command matches the first field
    char* firstField = getFieldString(data, 0);

    // to make not case sensitive
    int i;
    for (i = 0; i < longestCommand; i++)
    {
        // make lower case if upper case
        if (data->buffer[i] > 64 && data->buffer[i] < 91)
        {
            data->buffer[i] += 32;
        }
    }

    if (myStrcmp(strCommand, firstField) != 0)
    {
        return false;
    }

    // check if number of arguments (-command) is >= requested min arguments
    if ((data->fieldCount - 1) < minArguments)
    {
        return false;
    }

    return true; // if it hasn't returned false then it has a matching command and enough args

}

void ps(void)
{
    putsUart0("ps called");
}

void ipcs(void)
{
    putsUart0("ipcs called");
}

void kill(uint32_t pid)
{
    putsUart0("pid killed");
}
void pkill(char* pid)
{
    putsUart0("pid killed");
}
void pi(bool on)
{
    if (on)
    {
        putsUart0("pi on");
    }
    if (!on)
    {
        putsUart0("pi off");
    }
}
void preempt(bool on)
{
    if (on)
    {
        putsUart0("preempt on");
    }
    if (!on)
    {
        putsUart0("preempt off");
    }
}
void sched(bool prio_on)
{
    if (prio_on)
    {
        putsUart0("sched prio");
    }
    if (!prio_on)
    {
        putsUart0("sched rr");
    }
}
void pidof(const char name[])
{
    putsUart0("proc_name launched");
}
void run(char *name)
{
    // turn red led on
    RED_LED = 1;
}

void shell(void)
{
    USER_DATA data;
    while(true)
    {
        putsUart0("> ");
        //get string from user
        getsUart0(&data);
        //parse fields
        parseFields(&data);

        if(isCommand(&data,"reboot",0))
        {
            // reboot later
        }
        else if(isCommand(&data,"ps",0))
        {
            ps();
        }
        else if(isCommand(&data,"ipcs",0))
        {
            ipcs();
        }
        else if(isCommand(&data,"kill",1))
        {
            int32_t pid = getFieldInteger(&data, 1);
            kill(pid);
        }
        else if(isCommand(&data,"pkill",1))
        {
            char* nextField = getFieldString(&data, 1);
            pkill(nextField);
        }
        else if (isCommand(&data, "pi", 1))
        {
            // Turns priority inheritance on or off
            char* OnOff = getFieldString(&data, 1);
            bool on;

            if (myStrcmp(OnOff, "on") == 0)
            {
                on = true;
                pi(on);
            }
            else if (myStrcmp(OnOff, "off") == 0)
            {
                on = false;
                pi(on);
            }
            else
            {
                putsUart0("invalid on|off field");
            }
        }
        else if (isCommand(&data, "preempt", 1))
        {
            // Turns preemption on or off
            char* OnOff = getFieldString(&data, 1);
            bool on;

            if (myStrcmp(OnOff, "on") == 0)
            {
                on = true;
                preempt(on);
            }
            else if (myStrcmp(OnOff, "off") == 0)
            {
                on = false;
                preempt(on);
            }
            else
            {
                putsUart0("invalid on|off field");
            }
        }
        else if (isCommand(&data, "sched", 1))
        {
            // either priority or round robin scheduling
            char* prioRR = getFieldString(&data, 1);
            bool prio_on;

            if (myStrcmp(prioRR, "prio") == 0)
            {
                prio_on = true;
                sched(prio_on);
            }
            else if (myStrcmp(prioRR, "rr") == 0)
            {
                prio_on = false;
                sched(prio_on);
            }
            else
                putsUart0("invalid prio|rr field");
        }
        else if (isCommand(&data, "pidof", 1))
        {
            char* name = getFieldString(&data, 1);
            pidof(name);
        }
        else if (isCommand(&data, "run", 1))
        {
            char* name = getFieldString(&data, 1);
            run(name);
        }
        else
        {
            putsUart0("invalid command");
        }
        putcUart0('\n');
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);

    shell();
}