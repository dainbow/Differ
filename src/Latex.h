#pragma once

#include "Graph.h"

const char TEX_PATH[]             = "texs\\";

const char TEX_OUTPUT_FORMAT[]    = ".pdf";
const char LOG_FORMAT[]           = ".log";
const char AUX_FORMAT[]           = ".aux";

const int32_t TEX_MAX_NAME_LENGTH = 100;

char* MakeLatexTitle(Tree* tree, char const* name);

void PrintTexTree(Node* node, FILE* output);

int32_t CompareOperations(int8_t firstOper, int8_t secondOper);
int32_t GiveOperationPriority(int8_t operation);
