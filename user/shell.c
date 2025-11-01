#include "ulibc.h"
#include "uprintf.h"

static uint32_t parse_buf();

char *prompt = "risky321:%s>";
char current_directory[101] = "/\0";

#define MAX_CMD_ARGS  10
#define MAX_CMD_LEN  101
#define BUF_SIZE     101

enum CMD_TYPE {EXEC_CMD, BUILTIN_CMD};

struct parsed_buf
{
    enum CMD_TYPE type;
    uint32_t argc;
    char *argv[MAX_CMD_ARGS + 1]; /* last one is for NULL ptr that terminates argv array */
    char commands[MAX_CMD_ARGS][MAX_CMD_LEN];
} current_cmd;

char buf[BUF_SIZE];

void main(void)
{

    buf[BUF_SIZE - 1] = '\0';
    current_cmd.argv[MAX_CMD_ARGS] = NULL;


    uprintf("Hello from Risky Shell!\n");

    for(;;)
    {
        uprintf(prompt, current_directory);
        /* if buffer has at leas one valid character and parsed buffer has at least one valid element */
        if( (usys_read(0, (uint8_t *)buf, BUF_SIZE - 1) > 0) && (parse_buf() > 0) )
        {
            //char **iter = current_cmd.argv;

            if(usys_fork() == 0)
            {
                usys_exec(current_cmd.argv[0], current_cmd.argv);
            }
            else
            {
                usys_sleep();
            }
            

        }
    }
    
}

static uint32_t parse_buf()
{
    current_cmd.argc = 0;
    uint32_t argc = 0;
    uint32_t argv_pos = 0;

    if(buf[0] == '\0')
    {
        current_cmd.argc = 0;
        return 0;
    }

    for(int i = 0; i < BUF_SIZE; i++)
    {
        if(buf[i] == ' ')
        {
            if(argv_pos == 0)
            {
                continue;
            }
            current_cmd.commands[argc][argv_pos] = '\0';
            current_cmd.argc++;
            argc++;
            argv_pos = 0;
            continue;
        }

        if(argc >= MAX_CMD_ARGS)
        {
            uprintf("Shell Command Parse Error: Too many arguments.\n");
            return 0;
        }

        current_cmd.commands[argc][argv_pos] = buf[i];
        argv_pos++;

        if(buf[i] == '\0')
        {   
            if(argv_pos == 1)
            {
                break;
            }
            current_cmd.argc++;
            break;
        }

    }

    /* initialize argv with pointers to char arrays where command parts will be stored, to not pass char (*)[MAX_CMD_LEN] as arguments*/
    for(int i = 0; i <  current_cmd.argc; i++)
    {
        current_cmd.argv[i] = current_cmd.commands[i];
    }

    current_cmd.argv[current_cmd.argc] = NULL;

    return current_cmd.argc;
}
