/**
 ******************************************************************************
 * @file    cli.h
 * @brief   CLI (Command Line Interface) module header
 ******************************************************************************
 */

#ifndef __CLI_H__
#define __CLI_H__

#include "main.h"
#include <stdint.h>

#define CLI_CMD_MAX_LEN     64
#define CLI_CMD_COUNT_MAX   20

typedef struct {
    const char *name;
    const char *description;
    void (*handler)(int argc, char *argv[]);
} cli_command_t;

void CLI_RegisterCommand(const char *name, const char *description,
                        void (*handler)(int argc, char *argv[]));
void CLI_Printf(const char *fmt, ...);
void CLI_ProcessChar(uint8_t ch);
void CLI_RegisterBuiltInCommands(void);

#endif
