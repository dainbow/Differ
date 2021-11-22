#pragma once

#include <windows.h>
#include <stdlib.h>

#include "Tree.h"
#include "Graph.h"
#include "Text.h"
#include "Stack.h"
#include "Latex.h"

struct Context {
    Node* node;
    Node** prevNode;
};

const int32_t FAIL                 = 0;
const int32_t MAX_TRASH_SIZE       = 100;
const int32_t MAX_NODE_DATA_LENGTH = 20;

char const* TEX_NAME               = "latex";
const char MATH_PHRASES[]          = "MathPhrases.txt";

void ScanBase(Text* input, Stack* stack);
bool ReadTreeFromFile(Tree* tree, const char* inputFile);

int32_t ProcessNodeData(StackElem rawData, NodeDataTypes* type);
Node* MakeTreeFromStack(Stack* nodesStack);

int32_t Convert1251ToUtf8 (const char* input, char* output);

Node* Differentiate (Node* root, FILE* output, Text* mathPhrases);
Node* Copy (Node* root);
void OptimisationAfterDiff(Tree* tree);

int32_t FoldConst(Node* node);
int32_t CutEqualNodes(Context context);
int32_t CutNullNodes(Context context);
int32_t CutMinusOneNodes(Node* node);

int32_t CheckForVars(Node* node);

FILE* StartTex(Tree* tree, char** outputName);
void LogDiffProcessToTex(Node* curNode, Node* diffNode, FILE* output, Text* mathPhrases);
void StopTex(FILE* output, char* outputName, Node* node);
