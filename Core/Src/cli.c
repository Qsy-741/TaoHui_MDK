/**
 ******************************************************************************
 * @file    cli.c
 * @brief   CLI (Command Line Interface) module implementation
 ******************************************************************************
 */

#include "cli.h"
#include "cmsis_os.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

extern UART_HandleTypeDef huart1;

#define CLI_UART_HANDLE        (&huart1)
#define CLI_UART_BAUD_RATE     115200
#define CLI_CMD_MAX_LEN        64
#define CLI_CMD_COUNT_MAX      20
#define CLI_PROMPT_STR         "$ "

typedef struct {
    char buffer[CLI_CMD_MAX_LEN];
    uint8_t position;
} cli_context_t;

static cli_command_t g_commands[CLI_CMD_COUNT_MAX];
static uint8_t g_cmd_count = 0;
static cli_context_t g_cli_ctx;

static UART_HandleTypeDef * const gp_cli_uart = CLI_UART_HANDLE;

static void CLI_SendChar(char ch);
static void CLI_SendString(const char *str);
static void CLI_PrintPrompt(void);
static void CLI_HandleBackspace(void);
static void CLI_ParseAndExecute(char *cmd);
static void CMD_Help(int argc, char *argv[]);
static void CMD_Clear(int argc, char *argv[]);
static void CMD_Info(int argc, char *argv[]);

/**
 * @brief CLI task entry function
 * @param argument: Task argument (unused)
 * @retval None
 * @note This function overrides the weak definition in freertos.c
 */
void Task_CLI_Handler(void *argument)
{
    uint8_t ch;

    memset(&g_cli_ctx, 0, sizeof(cli_context_t));
    memset(g_commands, 0, sizeof(g_commands));
    g_cmd_count = 0;

    CLI_RegisterBuiltInCommands();

    CLI_SendString("\r\n");
    CLI_SendString("========================================\r\n");
    CLI_SendString("     CLI System Initialized             \r\n");
    CLI_SendString("     Type 'help' for commands          \r\n");
    CLI_SendString("========================================\r\n");
    CLI_PrintPrompt();

    for (;;) {
        if (HAL_UART_Receive(gp_cli_uart, &ch, 1, 10) == HAL_OK) {
            CLI_ProcessChar(ch);
        }
        osDelay(10);
    }
}

/**
 * @brief Register a command to the CLI command table
 * @param name: Command name string
 * @param description: Command description for help
 * @param handler: Command handler function pointer
 * @retval None
 */
void CLI_RegisterCommand(const char *name, const char *description,
                        void (*handler)(int argc, char *argv[]))
{
    if (g_cmd_count >= CLI_CMD_COUNT_MAX) {
        CLI_Printf("[CLI] Command table full!\r\n");
        return;
    }

    g_commands[g_cmd_count].name = name;
    g_commands[g_cmd_count].description = description;
    g_commands[g_cmd_count].handler = handler;
    g_cmd_count++;
}

/**
 * @brief Print formatted string to CLI UART
 * @param fmt: Format string
 * @param ...: Variable arguments
 * @retval None
 */
void CLI_Printf(const char *fmt, ...)
{
    char msg[128];
    va_list args;

    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    HAL_UART_Transmit(gp_cli_uart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * @brief Process a single character input
 * @param ch: Input character
 * @retval None
 */
void CLI_ProcessChar(uint8_t ch)
{
    switch (ch) {
        case '\r':
        case '\n':
            CLI_SendString("\r\n");
            if (g_cli_ctx.position > 0) {
                g_cli_ctx.buffer[g_cli_ctx.position] = '\0';
                CLI_ParseAndExecute(g_cli_ctx.buffer);
                g_cli_ctx.position = 0;
            }
            CLI_PrintPrompt();
            break;

        case '\b':
        case 0x7F:
            CLI_HandleBackspace();
            break;

        default:
            if (ch >= 0x20 && ch <= 0x7E && g_cli_ctx.position < CLI_CMD_MAX_LEN - 1) {
                g_cli_ctx.buffer[g_cli_ctx.position++] = ch;
                CLI_SendChar(ch);
            }
            break;
    }
}

/**
 * @brief Register built-in commands
 * @retval None
 */
void CLI_RegisterBuiltInCommands(void)
{
    CLI_RegisterCommand("help",  "Show all available commands",           CMD_Help);
    CLI_RegisterCommand("clear", "Clear the screen",                      CMD_Clear);
    CLI_RegisterCommand("info",  "Show system information",                CMD_Info);
}

static void CLI_SendChar(char ch)
{
    HAL_UART_Transmit(gp_cli_uart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
}

static void CLI_SendString(const char *str)
{
    HAL_UART_Transmit(gp_cli_uart, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

static void CLI_PrintPrompt(void)
{
    CLI_SendString(CLI_PROMPT_STR);
}

static void CLI_HandleBackspace(void)
{
    if (g_cli_ctx.position > 0) {
        g_cli_ctx.position--;
        CLI_SendChar('\b');
        CLI_SendChar(' ');
        CLI_SendChar('\b');
    }
}

static void CLI_ParseAndExecute(char *cmd)
{
    char *argv[16];
    int argc = 0;

    while (*cmd == ' ') cmd++;

    if (*cmd == '\0') return;

    char *token = strtok(cmd, " ");
    while (token != NULL && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    for (uint8_t i = 0; i < g_cmd_count; i++) {
        if (strcmp(argv[0], g_commands[i].name) == 0) {
            CLI_SendString("\r\n");
            g_commands[i].handler(argc, argv);
            return;
        }
    }

    CLI_Printf("[CLI] Unknown command: %s\r\n", argv[0]);
    CLI_Printf("[CLI] Type 'help' for available commands.\r\n");
}

static void CMD_Help(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    CLI_Printf("========================================\r\n");
    CLI_Printf("       Available Commands (%d)           \r\n", g_cmd_count);
    CLI_Printf("========================================\r\n");

    for (uint8_t i = 0; i < g_cmd_count; i++) {
        CLI_Printf("  %-12s - %s\r\n", g_commands[i].name, g_commands[i].description);
    }

    CLI_Printf("========================================\r\n");
    CLI_Printf("  Press ENTER for prompt '$'\r\n");
}

static void CMD_Clear(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    CLI_SendString("\033[2J\033[H");
    CLI_Printf("[OK] Screen cleared.\r\n");
}

static void CMD_Info(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    CLI_Printf("========================================\r\n");
    CLI_Printf("       System Information               \r\n");
    CLI_Printf("========================================\r\n");
    CLI_Printf("  MCU:       STM32F103C8T6\r\n");
    CLI_Printf("  UART:      USART1 @ %d bps\r\n", CLI_UART_BAUD_RATE);
    CLI_Printf("  FreeRTOS:  v10.0.1\r\n");
    CLI_Printf("  CLI Ver:   1.0.0\r\n");
    CLI_Printf("========================================\r\n");
}
