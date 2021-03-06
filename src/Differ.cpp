#include "Differ.h"

//TODO Замены, Маклорен, скобки в одинаковых операциях

int main () {
    TreeCtor(tree);
    TreeCtor(diffedTree);
    StackCtor(bigStack);

    Text mathPhrases  = {};
    Text greekSymbols = {};
    MakeText(&mathPhrases,  MATH_PHRASES);
    MakeText(&greekSymbols, GREEK_SYMBOLS);
    DiffContext diffContext = {&mathPhrases, &greekSymbols, &bigStack}; 

    ReadTreeFromFile(&tree, "Gachi.txt");
    MakeTreeGraph(&tree, G_STANDART_NAME);
    
    char* outputName = nullptr;
    FILE* output     = StartTex(&tree, &outputName, &diffContext);

    diffedTree.root  = Differentiate(tree.root, output, &diffContext, 1);

    MakeTreeGraph(&diffedTree, G_STANDART_NAME);
    OptimisationAfterDiff(&diffedTree);

    StopTex(output, outputName, tree.root, diffedTree.root, &diffContext);

    StackDtor(&bigStack);
    TreeDtor(&diffedTree);
    TreeDtor(&tree);
    
    printf("END\n");
}

void ScanBase(Text* input, Stack* stack) {
    assert(input  != nullptr);
    assert(stack  != nullptr);

    int8_t  trashBuff[MAX_TRASH_SIZE] = "";

    for (uint32_t curString = 0; curString < input->strAmount; curString++) {
        for (uint32_t curChar = 0; curChar < input->strings[curString].length - 1; curChar++) {
            int8_t* charPtr = input->strings[curString].value + curChar;

            if ((*charPtr != OPEN_BRACKET) && 
                (curChar == 0)) {
                StackPush(stack, charPtr);
            }
            else if ((*charPtr == OPEN_BRACKET) && 
                    (sscanf((const char*)charPtr + 1, " %1[^(]", trashBuff))) {
                for (uint32_t curIdx = curChar + 1; curIdx < input->strings[curString].length; curIdx++) {
                    if (input->strings[curString].value[curIdx] == OPEN_BRACKET) {
                        StackPush(stack, charPtr);
                        break;
                    }
                    if (input->strings[curString].value[curIdx] == CLOSE_BRACKET) {
                        break;
                    }
                }

                StackPush(stack, charPtr + 1);
            }
            else if (*charPtr == OPEN_BRACKET) {
                StackPush(stack, charPtr);
            }
            else if ((*charPtr == CLOSE_BRACKET) && 
                    sscanf((const char*)charPtr + 1, " %1[^)]", trashBuff)) {
                StackPush(stack, charPtr + 1);
            }
            else if (*charPtr == CLOSE_BRACKET) {
                StackPush(stack, charPtr);
            }
        }
    }
}

bool ReadTreeFromFile(Tree* tree, const char* inputFile) {
    assert(tree != nullptr);
    StackCtor(nodesStack);

    if (ReadTextFromFile(&tree->qbase, inputFile) == 0)
        return 0;

    MakeStrings(&tree->qbase);
    ProcessStrings(&tree->qbase);

    ScanBase(&tree->qbase, &nodesStack);
    tree->root = MakeTreeFromStack(&nodesStack);

    StackDtor(&nodesStack);
    return 1;
}

Node* MakeTreeFromStack(Stack* nodesStack) {
    assert(nodesStack != nullptr);
    assert(nodesStack->size > 0);

    StackCtor(treeStack);
    StackCtor(queueStack);

    Node* currentNode = 0;
    Node* leftOne     = 0;
    Node* rightOne    = 0;

    int32_t nodeConvertedData       = 0;
    NodeDataTypes nodeDataType      = TYPE_UNKNOWN;
    int32_t currentParenthesisDepth = 0;

    for (int32_t curIdx = 0; curIdx < nodesStack->size; curIdx++) {
        nodeConvertedData = ProcessNodeData(StackPopIndexDEVELOPERS_ONLY(nodesStack, curIdx), &nodeDataType);
        
        if (nodeDataType == TYPE_UNKNOWN) {
            if ((char)nodeConvertedData == OPEN_BRACKET) {
                currentParenthesisDepth += 1;
            }
            else if ((char)nodeConvertedData == CLOSE_BRACKET) {
                rightOne    = (Node*)StackPop(&queueStack);
                currentNode = (Node*)StackPop(&queueStack);

                currentNode->right = rightOne;
                currentNode->weight++;

                if (currentNode->type != TYPE_UNO) {
                    leftOne     = (Node*)StackPop(&queueStack);

                    currentNode->left  = leftOne;
                    currentNode->weight++;
                }
                
                currentParenthesisDepth -= 1;
                if (currentParenthesisDepth) {
                    StackPush(&queueStack, (StackElem)currentNode);
                }
                else {
                    StackPush(&treeStack,  (StackElem)currentNode);
                }
            }
            continue;
        }
        Node* newNode     = MakeNewNode(nodeConvertedData, nodeDataType);

        if (currentParenthesisDepth == 0) {
            StackPush(&treeStack, (StackElem)newNode);
        }
        else {
            StackPush(&queueStack, (StackElem)newNode);
        }   
    }

    rightOne     = (Node*)StackPop(&treeStack);
    currentNode  = (Node*)StackPop(&treeStack);
    currentNode->right = rightOne;

    if (currentNode->type != TYPE_UNO) {
        leftOne      = (Node*)StackPop(&treeStack);
        currentNode->left  = leftOne;
    }
    
    StackDtor(&queueStack);
    StackDtor(&treeStack);

    return currentNode;
}

int32_t Convert1251ToUtf8 (const char* input, char* output) {
    assert (input    != nullptr);
    assert (output   != nullptr);

    int32_t inputLength = 0;
    if ((inputLength = MultiByteToWideChar(1251, 0, input, -1, 0, 0)) == 0) {
        return 0;
    }

    wchar_t* buffer = (wchar_t*)calloc(inputLength + 10, sizeof(buffer[0]));
    if (MultiByteToWideChar(1251, 0, input, -1, buffer, inputLength) == 0) {
        free(buffer);
        return 0;
    }

    int32_t outputLength = 0;
    if ((outputLength = WideCharToMultiByte(65001, 0, buffer, -1, 0, 0, 0, 0)) == 0) {
        free(buffer);
        return 0;
    }

    if (WideCharToMultiByte(65001, 0, buffer, -1, output, outputLength, 0, 0) == 0) {
        free(buffer);

        return 0;
    }
    free(buffer);
    return outputLength;
}

int32_t ProcessNodeData(StackElem rawData, NodeDataTypes* type) {
    assert(rawData != nullptr);
    assert(type    != nullptr);
    
    int32_t convertedData = 0;

    int8_t buffer[MAX_TRASH_SIZE]      = "";
    
    if (sscanf((const char*)rawData, " %d", &convertedData)) {
        *type = TYPE_CONST;

        return convertedData;
    }
    else if (sscanf((const char*)rawData, " %1[sc]", buffer)) {
        convertedData = buffer[0];
        *type = TYPE_UNO;

        return convertedData;
    }
    else if (sscanf((const char*)rawData, " %1[+-*/^l]", buffer)) {
        convertedData = buffer[0];
        *type = TYPE_OP;

        return convertedData;
    }
    else if (sscanf((const char*)rawData, " %1[a-zA-Z]", buffer)) {
        convertedData = buffer[0];
        *type = TYPE_VAR;

        return convertedData;
    }
    else if (sscanf((const char*)rawData, " %1[()]", buffer)) {
        convertedData = buffer[0];
        *type = TYPE_UNKNOWN;

        return convertedData;
    }
    else {
        assert(FAIL && "UNKNOWN TYPE OF DATA IN EXPRESSION");
    }

    return -1;
}

#define N root
#define R root->right
#define L root->left

#define D(smth) Differentiate(smth, output, diffContext, logFlag)
#define C(smth) Copy(smth)
#define F(smth) MakeFactor(smth)

#define MUL(first, second) MakeNewNode((int32_t)(MUL_OP), TYPE_OP, first, second)
#define DIV(first, second) MakeNewNode((int32_t)(DIV_OP), TYPE_OP, first, second)
#define ADD(first, second) MakeNewNode((int32_t)(ADD_OP), TYPE_OP, first, second)
#define SUB(first, second) MakeNewNode((int32_t)(SUB_OP), TYPE_OP, first, second)
#define LOG(first, second) MakeNewNode((int32_t)(LOG_OP), TYPE_OP, first, second)
#define POW(first, second) MakeNewNode((int32_t)(POW_OP), TYPE_OP, first, second)

#define SIN(smth)          MakeNewNode((int32_t)SIN_OP, TYPE_UNO, nullptr, smth)
#define COS(smth)          MakeNewNode((int32_t)COS_OP, TYPE_UNO, nullptr, smth)

#define CONST_NODE(smth)   MakeNewNode(smth, TYPE_CONST)
#define VAR_NODE(smth)     MakeNewNode(smth, TYPE_VAR)     

Node* Differentiate (Node* root, FILE* output, DiffContext* diffContext, bool logFlag) {
    assert(root        != nullptr);
    assert(output      != nullptr);
    assert(diffContext != nullptr);

    Node* returningRoot = nullptr;

    switch (root->type) {
    case TYPE_VAR:
        if (root->data == (int32_t)E_CONST)
            returningRoot = VAR_NODE(E_CONST);
        else 
            returningRoot = CONST_NODE(1);
        break;
    case TYPE_CONST:
        returningRoot = CONST_NODE(0);
        break;
    case TYPE_OP:
        switch (root->data) {
            case (int32_t)(SUB_OP):
                returningRoot = SUB(D(L), D(R));
                break;
            case (int32_t)(ADD_OP): 
                returningRoot = ADD(D(L), D(R));
                break;

            case (int32_t)(MUL_OP):
                returningRoot = ADD(MUL(D(L), C(R)), MUL(D(R), C(L)));
                break;

            case (int32_t)(DIV_OP):
                returningRoot = DIV(SUB(MUL(D(L), C(R)), MUL(C(L), D(R))), MUL(C(R), C(R)));
                break;

            case (int32_t)LOG_OP: 
                returningRoot = DIV(D(R), MUL(LOG(VAR_NODE(E_CONST), C(L)), C(R)));
                break;

            case (int32_t)POW_OP: 
                if (CheckForVars(L) && CheckForVars(R))
                    returningRoot = MUL(C(N), D(MUL(LOG(VAR_NODE(E_CONST), C(L)), C(R))));

                else if (CheckForVars(L)) 
                    returningRoot = MUL(D(L), MUL(C(R), POW(C(L), SUB(C(R), CONST_NODE(1)))));

                else if (CheckForVars(R))
                    returningRoot = MUL(LOG(VAR_NODE(E_CONST), C(L)), MUL(D(R), POW(C(L), C(R))));

                else
                    returningRoot = CONST_NODE(0);
                break;
            
            default:
                assert(FAIL && "INVALID OPERATION");
        }
        break;
    case TYPE_UNO:
        switch (root->data) {
        case SIN_OP: 
            returningRoot = MUL(COS(C(R)), D(R));
            break;
        
        case COS_OP: 
            returningRoot = MUL(MUL(CONST_NODE(-1), SIN(C(R))), D(R));
            break;
        
        default:
            break;
        }
        break;
    case TYPE_UNKNOWN:
    default:
        assert(FAIL && "UNKNOWN DATA TYPE");
    }

    if (logFlag) {
        LogDiffProcessToTex(N, returningRoot, output, diffContext);
    }

    return returningRoot;
}

Node* Copy (Node* root) {
    assert(root  != nullptr);

    if ((root->left  != nullptr) &&
        (root->right != nullptr)) {
        return MakeNewNode(root->data, root->type, Copy(root->left), Copy(root->right));
    }
    else if ((root->left == nullptr) &&
             (root->right != nullptr)) {
        return MakeNewNode(root->data, root->type, 0, Copy(root->right));
    }
    else {
        return MakeNewNode(root->data, root->type);
    }

    return nullptr;
}

Node* MakeFactor(int32_t factor) {
    if ((factor == 1) || (factor == 0)) {
        return CONST_NODE(1);
    }
    else {
        return MUL(CONST_NODE(factor), F(factor - 1));
    }
}

int32_t FoldConst(Node* node) {
    assert(node != nullptr);
    int32_t returnValue = 0;

    if ((node->type == TYPE_OP) || (node->type == TYPE_UNO)) {
        if  (((node->type) == TYPE_OP)        &&
            ((node->data) != LOG_OP)             &&
            (node->left->type  == TYPE_CONST) &&
            (node->right->type == TYPE_CONST)) {
        switch (node->data) {
            case (int32_t)(ADD_OP):
                node->data = node->left->data + node->right->data;
                node->type = TYPE_CONST;
                break;
            case (int32_t)(SUB_OP):
                node->data = node->left->data - node->right->data;
                node->type = TYPE_CONST;
                break;
            case (int32_t)(MUL_OP):
                node->data = node->left->data * node->right->data;
                node->type = TYPE_CONST;
                break;
            case (int32_t)(DIV_OP):
                if ((node->left->data % node->right->data) == 0) {
                    node->data = node->left->data / node->right->data;
                    node->type = TYPE_CONST;
                    break;
                }
                else {
                    goto elseSection;
                }
            case (int32_t)(POW_OP):
                if (node->right->data > 0) {
                    node->data = (int32_t)pow(node->left->data, node->right->data);
                    node->type = TYPE_CONST;
                    break;
                }
                else 
                    goto elseSection;
            default:
                assert(FAIL && "UNKNOWN OPERATION");
            }
        }
        else if (((node->data) == LOG_OP) &&
                (node->left->type == node->right->type) && (node->left->data == node->right->data) &&
                ((node->left->type == TYPE_CONST) || (node->left->type == TYPE_VAR))) {
            node->data = 1;
            node->type = TYPE_CONST;
        }
        else if (((node->type) == TYPE_OP) &&
                 (node->left->type  == TYPE_VAR)    &&
                 (node->right->type == TYPE_CONST)  &&
                 (node->right->data == 0)) {
            switch (node->data) {
                case POW_OP:
                    node->data = 1;
                    node->type = TYPE_CONST;
                    break;
                default:
                    goto elseSection;
            }
        }
        else if ((node->type       == TYPE_UNO) &&
                (node->right->data == 0)) {
            switch (node->data) {
            case SIN_OP:
                node->data = 0;
                node->type = TYPE_CONST;
                break;
            case COS_OP:
                node->data = 1;
                node->type = TYPE_CONST;
                break;
            default:
                break;
            }
        }
        else 
            goto elseSection;        

        if (node->left  != nullptr) NodeDtor(node->left);
        if (node->right != nullptr) NodeDtor(node->right);

        node->left  = nullptr;
        node->right = nullptr; 
        return 1;
    }
    else {
        elseSection:
        if (node->left  != nullptr)
            returnValue += FoldConst(node->left);
        if (node->right != nullptr) 
            returnValue += FoldConst(node->right);
    }

    return returnValue;
}

#define CUT_EQUAL_NODES(direction1, direction2, value)                          \
    if ((context.node-> direction1 ->type == TYPE_CONST) &&                     \
        (context.node-> direction1 ->data == value)) {                          \
        if (*context.prevNode == context.node) {                                \
            Node* saveNode = *context.prevNode;                                 \
            *context.prevNode = context.node-> direction2;                      \
                                                                                \
            free(saveNode-> direction1);                                        \
            free(saveNode);                                                     \
        }                                                                       \
        else {                                                                  \
            if ((*context.prevNode)->left == context.node) {                    \
                (*context.prevNode)->left = context.node->direction2;           \
            }                                                                   \
            else {                                                              \
                (*context.prevNode)->right = context.node->direction2;          \
            }                                                                   \
            free(context.node-> direction1);                                    \
            free(context.node);                                                 \
        }                                                                       \
                                                                                \
        return 1;                                                               \
    }

#define NEXT_CUT_FUNC_ITERATION(direction, function)                \
    if (context.node-> direction ->type == TYPE_OP) {               \
        Context next = {context.node->direction, &context.node};    \
                                                                    \
        returnValue += function (next);                             \
    }   

int32_t CutEqualNodes(Context context) {
    int32_t returnValue = 0;

    if (context.node->data == (int32_t)ADD_OP) {
        CUT_EQUAL_NODES(left, right, 0)
        CUT_EQUAL_NODES(right, left, 0)
    }
    else if (context.node->data == (int32_t)MUL_OP) {
        CUT_EQUAL_NODES(left, right, 1)
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)SUB_OP) {
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)POW_OP) {
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)DIV_OP) {
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)LOG_OP) {
        CUT_EQUAL_NODES(left, right, 1)
    }

    if (context.node->type == TYPE_OP) {
        NEXT_CUT_FUNC_ITERATION(left,  CutEqualNodes)
        NEXT_CUT_FUNC_ITERATION(right, CutEqualNodes)
    }
    else if (context.node->type == TYPE_UNO) {
        NEXT_CUT_FUNC_ITERATION(right, CutEqualNodes)
    }
    
    return returnValue;
}

#undef CUT_EQUAL_NODES

#define CUT_NULL_NODES(direction1, direction2, value)                           \
    if ((context.node-> direction1 ->type == TYPE_CONST) &&                     \
        (context.node-> direction1 ->data == value)) {                          \
        if (*context.prevNode == context.node) {                                \
            Node* saveNode = *context.prevNode;                                 \
            *context.prevNode = context.node-> direction1;                      \
                                                                                \
            NodeDtor(saveNode-> direction2);                                    \
            free(saveNode);                                                     \
        }                                                                       \
        else {                                                                  \
            if ((*context.prevNode)->left == context.node) {                    \
                (*context.prevNode)->left = context.node->direction1;           \
            }                                                                   \
            else {                                                              \
                (*context.prevNode)->right = context.node->direction1;          \
            }                                                                   \
            NodeDtor(context.node-> direction2);                                \
            free(context.node);                                                 \
        }                                                                       \
                                                                                \
        return 1;                                                               \
    }

int32_t CutNullNodes(Context context) {
    int32_t returnValue = 0;

    if (context.node->type == TYPE_OP) {
        if (context.node->data == (int32_t)MUL_OP) {
            CUT_NULL_NODES(left, right, 0)
            CUT_NULL_NODES(right, left, 0)
        }
        else if (context.node->data == (int32_t)POW_OP) {
            CUT_NULL_NODES(left, right, 0)
        }
        else if (context.node->data == (int32_t)DIV_OP) {
            CUT_NULL_NODES(left, right, 0)
            if (!context.node->right->data) {
                assert(FAIL && "ZERO DIVISION ERROR");
            }
        }
        else if (context.node->data == (int32_t)LOG_OP) {
            CUT_NULL_NODES(right, left, 1)
        }
    }

    if (context.node->type == TYPE_OP) {
        NEXT_CUT_FUNC_ITERATION(left,  CutNullNodes)
        NEXT_CUT_FUNC_ITERATION(right, CutNullNodes)
    }
    else if (context.node->type == TYPE_UNO) {
        NEXT_CUT_FUNC_ITERATION(right, CutNullNodes)
    }
    
    return returnValue;
}

#undef CUT_NULL_NODES
#define CUT_MINUS_ONE_NODES(operator, value, swapping)                              \
    if ((node->data == (int32_t)operator) && (node->left->data == value)) {         \
        node->data = (int32_t)swapping;                                             \
        NodeDtor(node->left);                                                       \
                                                                                    \
        node->left          = Copy(node->right);                                    \
                                                                                    \
        node->right->data   = -1;                                                   \
        node->right->left   = nullptr;                                              \
        node->right->right  = nullptr;                                              \
        node->right->type   = TYPE_CONST;                                           \
    }                                           


int32_t CutMinusOneNodes(Node* node) {
    int32_t returnValue = 0;

    if ((node->type == TYPE_OP) &&
        (node->left->type  == TYPE_CONST)) {
        CUT_MINUS_ONE_NODES(DIV_OP, 1, POW_OP)
        CUT_MINUS_ONE_NODES(SUB_OP, 0, MUL_OP)
    }

    if (node->type == TYPE_OP) {
        if (node->left->type  == TYPE_OP) {
            returnValue += CutMinusOneNodes(node->left);
        }
        if (node->right->type == TYPE_OP) {
            returnValue += CutMinusOneNodes(node->right);
        }
    }
    else if (node->type == TYPE_UNO) {
        if ((node->right->type == TYPE_OP) ||
            (node->right->type == TYPE_UNO)) {
            returnValue += CutMinusOneNodes(node->right);
        }
    }
    
    return returnValue;
}

#undef CUT_MINUS_ONE_NODES

int32_t CheckForVars(Node* node) {
    assert(node != nullptr);
    int32_t returnValue = 0;

    if ((node->type == TYPE_VAR) &&
        (node->data != E_CONST)) {
        returnValue++;
    }

    if (node->left != nullptr) {
        returnValue += CheckForVars(node->left);
    }
    if (node->right != nullptr) {
        returnValue += CheckForVars(node->right);
    }

    return returnValue;
}

void OptimisationAfterDiff(Tree* tree) {
    assert(tree != nullptr);

    int32_t optimisationCounter = 0;
    do {
        optimisationCounter  = 0;

        optimisationCounter += FoldConst(tree->root);
        optimisationCounter += CutEqualNodes({tree->root, &tree->root});
        optimisationCounter += CutNullNodes({tree->root, &tree->root});
        optimisationCounter += CutMinusOneNodes(tree->root);

        //MakeTreeGraph(tree, G_STANDART_NAME);
    } while (optimisationCounter);
}

FILE* StartTex(Tree* tree, char** outputName, DiffContext* diffContext) {
    assert(tree != nullptr);
    
    *outputName = MakeLatexTitle(tree, TEX_NAME, diffContext);
    assert(*outputName != nullptr);

    FILE* output = fopen(*outputName, "a");
    assert(output != nullptr);

    PrintBigNodes(output, diffContext);

    return output;
}

void LogDiffProcessToTex(Node* curNode, Node* diffNode, FILE* output, DiffContext* diffContext) {
    assert(curNode     != nullptr);
    assert(diffNode    != nullptr);
    assert(output      != nullptr);
    assert(diffContext != nullptr);

    fprintf(output, "%s\n\n", diffContext->mathPhrases->strings
                              [rand() % diffContext->mathPhrases->strAmount].value);
    fprintf(output, "$(");
        PrintTexTree(1, curNode, output, diffContext);
    fprintf(output, ")'$ = $");
        PrintTexTree(1, diffNode, output, diffContext);
    fprintf(output, "$\n\n");
        PrintBigNodes(output, diffContext);
}

void StopTex(FILE* output, char* outputName, Node* beginNode, Node* node, DiffContext* diffContext) {
    assert(output != nullptr);
    assert(node   != nullptr);

    fprintf(output, "После некоторых наипростейших упрощений, я получил ответ:\n\n");
    
    fprintf(output, "$");
        PrintTexTree(1, node, output, diffContext);
    fprintf(output, "$\n\n");
        PrintBigNodes(output, diffContext);
    
    #if(MAKLOREN)
        fprintf(output, "А также, хотелось бы дать читателю занимательное упражнение: разложить данное выражение в ряд Маклорена, "
                    "а когда вы вернётесь, то можете сверить ответ\n\n");

        MakeMakloren(output, beginNode, 5, 'x', diffContext);
    #endif
    
    fprintf(output, "\\end{document}\n");
    fclose(output);

    char pdflatex[MAX_COMMAND_NAME] = "";
    char del[MAX_COMMAND_NAME]      = "";
    char delLog[MAX_COMMAND_NAME]   = "";
    char delAux[MAX_COMMAND_NAME]   = "";
    char start[MAX_COMMAND_NAME]    = "";

    sprintf(pdflatex, "pdflatex %s -output-directory=%s", outputName, TEX_PATH);
    
    sprintf(del,    "del \"%s\"", outputName);
    sprintf(delLog, "del \"%s%s\"", outputName, LOG_FORMAT);
    sprintf(delAux, "del \"%s%s\"", outputName, AUX_FORMAT);

    sprintf(start, "start %s%s", outputName, TEX_OUTPUT_FORMAT);

    system(pdflatex);

    system(del);
    system(delLog);
    system(delAux);
    
    system(start);
}

void SubstituteVars(Node* node, int32_t varData, int32_t value) {
    assert(node != 0);

    if (node->left != nullptr) {
        SubstituteVars(node->left, varData, value);
    }

    if ((node->type == TYPE_VAR) &&
        (node->data == varData)) {
        node->data = value;
        node->type = TYPE_CONST;
    }

    if (node->right != nullptr) {
        SubstituteVars(node->right, varData, value);
    }
}

void MakeMakloren(FILE* output, Node* node, int32_t accuracy, int32_t variable, DiffContext* diffContext) {
    assert(output != nullptr);
    assert(node   != nullptr);
    TreeCtor(maklorenTree);
    TreeCtor(maklorenSubTree);

    maklorenTree.root    = node;
    maklorenSubTree.root = node;
    Node* finalNode      = nullptr;

    fprintf(output, "f(%c) = $", variable);
    for (int32_t curSum = 0; curSum <= accuracy; curSum++) {
        if (curSum > 0) {
            maklorenTree.root    = Differentiate(maklorenTree.root, output, diffContext, 0);
            OptimisationAfterDiff(&maklorenTree);
        }

        maklorenSubTree.root = Copy(maklorenTree.root);
        SubstituteVars(maklorenSubTree.root, variable, 0);
        OptimisationAfterDiff(&maklorenSubTree);

        maklorenSubTree.root = MUL(DIV(maklorenSubTree.root, F(curSum)), POW(VAR_NODE('x'), CONST_NODE(curSum)));

        if (finalNode == nullptr)
            finalNode = maklorenSubTree.root;
        else
            finalNode = ADD(finalNode, maklorenSubTree.root);
    }

    maklorenSubTree.root = finalNode;
    MakeTreeGraph(&maklorenSubTree, G_STANDART_NAME);
    OptimisationAfterDiff(&maklorenSubTree);
    PrintTexTree(0, maklorenSubTree.root, output, diffContext);
    fprintf(output, "+ o(%c^{%d}), %c \\to 0$\n\n", variable, accuracy, variable);
    //PrintBigNodes(output, diffContext);

    //TreeDtor(&maklorenTree);
}
