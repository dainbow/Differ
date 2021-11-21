#include "Differ.h"

int main () {
    TreeCtor(tree);
    TreeCtor(diffedTree);

    ReadTreeFromFile(&tree, "Gachi.txt");

    Text mathPhrases = {};
    MakeText(&mathPhrases, MATH_PHRASES);
    
    MakeTreeGraph(&tree, G_STANDART_NAME);
    
    char* outputName = nullptr;
    FILE* output = StartTex(&tree, &outputName);
    diffedTree.root = Differentiate(tree.root, output, &mathPhrases);

    MakeTreeGraph(&diffedTree, G_STANDART_NAME);
    OptimisationAfterDiff(&diffedTree);

    StopTex(output, outputName, diffedTree.root);
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

            if ((*charPtr != '(') && 
                (curChar == 0)) {
                StackPush(stack, charPtr);
            }
            else if ((*charPtr == '(') && 
                    (sscanf((const char*)charPtr + 1, " %1[^(]", trashBuff))) {
                for (uint32_t curIdx = curChar + 1; curIdx < input->strings[curString].length; curIdx++) {
                    if (input->strings[curString].value[curIdx] == '(') {
                        StackPush(stack, charPtr);
                        break;
                    }
                    if (input->strings[curString].value[curIdx] == ')') {
                        break;
                    }
                }

                StackPush(stack, charPtr + 1);
            }
            else if (*charPtr == '(') {
                StackPush(stack, charPtr);
            }
            else if ((*charPtr == ')') && 
                    sscanf((const char*)charPtr + 1, " %1[^)]", trashBuff)) {
                StackPush(stack, charPtr + 1);
            }
            else if (*charPtr == ')') {
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
            if ((char)nodeConvertedData == '(') {
                currentParenthesisDepth += 1;
            }
            else if ((char)nodeConvertedData == ')') {
                rightOne    = (Node*)StackPop(&queueStack);
                currentNode = (Node*)StackPop(&queueStack);
                currentNode->right = rightOne;

                if (currentNode->type != TYPE_UNO) {
                    leftOne     = (Node*)StackPop(&queueStack);
                    currentNode->left  = leftOne;
                }
                
                currentParenthesisDepth -= 1;
                if (currentParenthesisDepth) {
                    StackPush(&queueStack, (StackElem)currentNode);
                }
                else {
                    StackPush(&treeStack, (StackElem)currentNode);
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

#define D(smth) Differentiate(smth, output, mathPhrases)
#define C(smth) Copy(smth)

#define MUL(first, second) MakeNewNode((int32_t)('*'), TYPE_OP, first, second)
#define DIV(first, second) MakeNewNode((int32_t)('/'), TYPE_OP, first, second)
#define ADD(first, second) MakeNewNode((int32_t)('+'), TYPE_OP, first, second)
#define SUB(first, second) MakeNewNode((int32_t)('-'), TYPE_OP, first, second)
#define LOG(first, second) MakeNewNode((int32_t)('l'), TYPE_OP, first, second)
#define POW(first, second) MakeNewNode((int32_t)('^'), TYPE_OP, first, second)

#define SIN(smth)          MakeNewNode((int32_t)'s', TYPE_UNO, nullptr, smth)
#define COS(smth)          MakeNewNode((int32_t)'c', TYPE_UNO, nullptr, smth)

#define CONST_NODE(smth)        MakeNewNode(smth, TYPE_CONST)
#define VAR_NODE(smth)          MakeNewNode(smth, TYPE_VAR)     

Node* Differentiate (Node* root, FILE* output, Text* mathPhrases) {
    assert(root        != nullptr);
    assert(output      != nullptr);
    assert(mathPhrases != nullptr);

    Node* returningRoot = nullptr;

    switch (root->type) {
    case TYPE_VAR:
        if (root->data == (int32_t)'e')
            returningRoot = CONST_NODE('e');
        else 
            returningRoot = CONST_NODE(1);
        break;
    case TYPE_CONST:
        returningRoot = CONST_NODE(0);
        break;
    case TYPE_OP:
        switch (root->data) {
            case (int32_t)('-'):
            case (int32_t)('+'): 
                returningRoot = ADD(D(L), D(R));
                break;

            case (int32_t)('*'):
                returningRoot = ADD(MUL(D(L), C(R)), MUL(D(R), C(L)));
                break;

            case (int32_t)('/'):
                returningRoot = DIV(SUB(MUL(D(L), C(R)), MUL(C(L), D(R))), MUL(C(R), C(R)));
                break;

            case (int32_t)'l': 
                returningRoot = DIV(D(R), MUL(LOG(VAR_NODE('e'), C(L)), C(R)));
                break;

            case (int32_t)'^': 
                if (CheckForVars(L) && CheckForVars(R))
                    returningRoot = MUL(C(N), D(MUL(LOG(VAR_NODE('e'), C(L)), C(R))));

                else if (CheckForVars(L)) 
                    returningRoot = MUL(D(L), MUL(C(R), POW(C(L), SUB(C(R), CONST_NODE(1)))));

                else if (CheckForVars(R))
                    returningRoot = MUL(LOG(VAR_NODE('e'), C(L)), MUL(D(R), POW(C(L), C(R))));

                else
                    returningRoot = CONST_NODE(0);
                break;
            
            default:
                assert(FAIL && "INVALID OPERATION");
        }
        break;
    case TYPE_UNO:
        switch (root->data) {
        case 's': 
            returningRoot = MUL(COS(C(R)), D(R));
            break;
        
        case 'c': 
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

    LogDiffProcessToTex(N, returningRoot, output, mathPhrases);
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

int32_t FoldConst(Node* node) {
    assert(node != nullptr);
    int32_t returnValue = 0;

    if (node->type == TYPE_OP) {
        if ((node->data != (int32_t)'l') &&
            (node->left->type  == TYPE_CONST) &&
            (node->right->type == TYPE_CONST)) {
        switch (node->data) {
            case (int32_t)('+'):
                node->data = node->left->data + node->right->data;
                node->type = TYPE_CONST;
                break;
            case (int32_t)('-'):
                node->data = node->left->data - node->right->data;
                node->type = TYPE_CONST;
                break;
            case (int32_t)('*'):
                node->data = node->left->data * node->right->data;
                node->type = TYPE_CONST;
                break;
            case (int32_t)('/'):
                node->data = node->left->data / node->right->data;
                node->type = TYPE_CONST;
                break;
            default:
                assert(FAIL && "UNKNOWN OPERATION");
            }
        }
        else if (((node->data) == 'l') &&
                (node->left->type == node->right->type) && (node->left->data == node->right->data) &&
                ((node->left->type == TYPE_CONST) || (node->left->type == TYPE_VAR))) {
            node->data = 1;
            node->type = TYPE_CONST;
        }
        else 
            goto elseSection;        

        NodeDtor(node->left);
        NodeDtor(node->right);

        node->left  = nullptr;
        node->right = nullptr; 
        return 1;
    }
    else {
        elseSection:
        if (node->type != TYPE_CONST) {
            if (node->left->type  == TYPE_OP) {
                returnValue += FoldConst(node->left);
            }
            if (node->right->type == TYPE_OP) {
                returnValue += FoldConst(node->right);
            }
        }
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

    if (context.node->data == (int32_t)'+') {
        CUT_EQUAL_NODES(left, right, 0)
        CUT_EQUAL_NODES(right, left, 0)
    }
    else if (context.node->data == (int32_t)'*') {
        CUT_EQUAL_NODES(left, right, 1)
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)'-') {
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)'^') {
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)'/') {
        CUT_EQUAL_NODES(right, left, 1)
    }
    else if (context.node->data == (int32_t)'l') {
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
        if (context.node->data == (int32_t)'*') {
            CUT_NULL_NODES(left, right, 0)
            CUT_NULL_NODES(right, left, 0)
        }
        else if (context.node->data == (int32_t)'^') {
            CUT_NULL_NODES(left, right, 0)
        }
        else if (context.node->data == (int32_t)'/') {
            CUT_NULL_NODES(left, right, 0)
            if (!context.node->right->data) {
                assert(FAIL && "ZERO DIVISION ERROR");
            }
        }
        else if (context.node->data == (int32_t)'l') {
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
        CUT_MINUS_ONE_NODES('/', 1, '^')
        CUT_MINUS_ONE_NODES('-', 0, '*')
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

    if (node->type == TYPE_VAR) {
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

        MakeTreeGraph(tree, G_STANDART_NAME);
    } while (optimisationCounter);
}

FILE* StartTex(Tree* tree, char** outputName) {
    assert(tree != nullptr);

    *outputName = MakeLatexTitle(tree, TEX_NAME);
    assert(*outputName != nullptr);

    FILE* output = fopen(*outputName, "a");
    assert(output != nullptr);

    return output;
}

void LogDiffProcessToTex(Node* curNode, Node* diffNode, FILE* output, Text* mathPhrases) {
    assert(curNode     != nullptr);
    assert(diffNode    != nullptr);
    assert(output      != nullptr);
    assert(mathPhrases != nullptr);

    fprintf(output, "%s\n\n", mathPhrases->strings[rand() % mathPhrases->strAmount].value);
    fprintf(output, "$(");
        PrintTexTree(curNode, output);
    fprintf(output, ")'$ = $");
        PrintTexTree(diffNode, output);
    fprintf(output, "$\n\n");
}

void StopTex(FILE* output, char* outputName, Node* node) {
    assert(output != nullptr);
    assert(node   != nullptr);

    fprintf(output, "После некоторых наипростейших упрощений, я получил ответ:\n\n");
    
    fprintf(output, "$");
        PrintTexTree(node, output);
    fprintf(output, "$\n\n");

    fprintf(output, "\\end{document}\n");
    fclose(output);

    char pdflatex[MAX_COMMAND_NAME] = "";
    char del[MAX_COMMAND_NAME]      = "";
    char start[MAX_COMMAND_NAME]    = "";

    sprintf(pdflatex, "pdflatex %s -output-directory=%s", outputName, TEX_PATH);
    sprintf(start, "start %s%s", outputName, TEX_OUTPUT_FORMAT);

    system(pdflatex);
    system(start);
}
