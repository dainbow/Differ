#pragma once

//#include "TXLib.h"
#include <windows.h>

#include "Tree.h"
#include "Graph.h"
#include "Text.h"
#include "Stack.h"

struct Context {
    Node* node;
    Node** prevNode;
};

const int32_t FAIL                 = 0;
const int32_t MAX_TRASH_SIZE       = 100;
const int32_t MAX_NODE_DATA_LENGTH = 20;

void ScanBase(Text* input, Stack* stack);
bool ReadTreeFromFile(Tree* tree, const char* inputFile);

int32_t ProcessNodeData(StackElem rawData, NodeDataTypes* type);
Node* MakeTreeFromStack(Stack* nodesStack);

int32_t Convert1251ToUtf8 (const char* input, char* output);

Node* Differentiate (Node* root);
Node* Copy (Node* root);

int32_t FoldConst(Node* node);
int32_t CutEqualNodes(Context context);
int32_t CutNullNodes(Context context);
int32_t CutMinusOneNodes(Node* node);
