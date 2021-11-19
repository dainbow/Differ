#include "Differ.h"

int main () {
    TreeCtor(tree);
    TreeCtor(diffedTree);

    ReadTreeFromFile(&tree, "Gachi.txt");
    MakeTreeGraph(&tree, G_STANDART_NAME);

    Node* diffed = Differentiate(tree.root);
    diffedTree.root = diffed;
    printf("!!!%d!!!%d %p\n", diffedTree.root->left->type, diffedTree.root->left->right->type, diffedTree.root->left->right->right);
    MakeTreeGraph(&diffedTree, G_STANDART_NAME);

    int32_t optimisationCounter = 0;
    do {
        optimisationCounter  = 0;

        optimisationCounter += FoldConst(diffedTree.root);
        optimisationCounter += CutEqualNodes({diffedTree.root, &diffedTree.root});
        optimisationCounter += CutNullNodes({diffedTree.root, &diffedTree.root});
        optimisationCounter += CutMinusOneNodes(diffedTree.root);

        MakeTreeGraph(&diffedTree, G_STANDART_NAME);
    } while (optimisationCounter);

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

            if ((*charPtr != '(') && (curChar == 0)) {
                StackPush(stack, charPtr);
            }
            else if ((*charPtr == '(') && (sscanf((const char*)charPtr + 1, " %1[^(]", trashBuff))) {
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
            else if ((*charPtr == ')') && sscanf((const char*)charPtr + 1, " %1[^)]", trashBuff)) {
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
        printf("%d iter.\n", curIdx);
        nodeConvertedData = ProcessNodeData(StackPopIndexDEVELOPERS_ONLY(nodesStack, curIdx), &nodeDataType);
        
        if (nodeDataType == TYPE_UNKNOWN) {
            printf("%d iter.\n", curIdx);
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
                printf("Popped %c and %c and %c[%d]\n", (char)rightOne->data, (char)currentNode->data, (char)leftOne->data, leftOne->data);
                
                currentParenthesisDepth -= 1;
                if (currentParenthesisDepth) {
                    StackPush(&queueStack, (StackElem)currentNode);
                }
                else {
                    StackPush(&treeStack, (StackElem)currentNode);
                    printf("TreeStack size is %d\n", treeStack.size);
                }
            }
            continue;
        }
        Node* newNode     = MakeNewNode(nodeConvertedData, nodeDataType);

        if (currentParenthesisDepth == 0) {
            StackPush(&treeStack, (StackElem)newNode);
            printf("TreeStack size is %d\n", treeStack.size);
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
    
    printf("First node char is %c\n", (char)currentNode->data);

    printf("Tree stack size is %d\n", treeStack.size);

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
    printf("rawData: %s\n", rawData);

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

Node* Differentiate (Node* root) {
    assert(root  != nullptr);

    Node* returningRoot = 0;

    switch (root->type) {
    case TYPE_VAR:
        if (root->data == (int32_t)'e')
            returningRoot = MakeNewNode('e', TYPE_CONST);
        else 
            returningRoot = MakeNewNode(1, TYPE_CONST);
        return returningRoot;
    case TYPE_CONST:
        returningRoot = MakeNewNode(0, TYPE_CONST);
        return returningRoot;
    case TYPE_OP:
        switch (root->data) {
            case (int32_t)('-'):
            case (int32_t)('+'): {
                Node* leftSummand  = Differentiate(root->left);
                Node* rightSummand = Differentiate(root->right);

                return MakeNewNode(root->data, TYPE_OP, leftSummand, rightSummand);
            }
            case (int32_t)('*'): {
                Node* leftDiff  = Differentiate(root->left);
                Node* rightDiff = Differentiate(root->right);

                Node* leftCopy  = Copy(root->left);
                Node* rightCopy = Copy(root->right);

                Node* leftSummand  = MakeNewNode((int32_t)('*'), TYPE_OP, leftDiff, rightCopy);
                Node* rightSummand = MakeNewNode((int32_t)('*'), TYPE_OP, rightDiff, leftCopy);

                return MakeNewNode((int32_t)('+'), TYPE_OP, leftSummand, rightSummand);
            }
            case (int32_t)('/'): {
                Node* leftDiff  = Differentiate(root->left);
                Node* rightDiff = Differentiate(root->right);

                Node* leftCopy  = Copy(root->left);
                Node* rightCopy = Copy(root->right);

                Node* leftSummand  = MakeNewNode((int32_t)('*'), TYPE_OP, leftDiff, rightCopy);
                Node* rightSummand = MakeNewNode((int32_t)('*'), TYPE_OP, rightDiff, leftCopy);

                Node* numerator    = MakeNewNode((int32_t)('-'), TYPE_OP, leftSummand, rightSummand);
                Node* denominator  = MakeNewNode((int32_t)('*'), TYPE_OP, Copy(root->right), Copy(root->right));

                return  MakeNewNode((int32_t)('/'), TYPE_OP, numerator, denominator);
            }
            case (int32_t)'l': {
                Node* base              = MakeNewNode('e', TYPE_VAR);
                Node* stepen            = Copy(root->left);
                Node* leftMultiplier    = MakeNewNode((int32_t)'l', TYPE_OP, base, stepen);

                Node* denominator = MakeNewNode((int32_t)'*', TYPE_OP, leftMultiplier, Copy(root->right));

                return MakeNewNode((int32_t)'/', TYPE_OP, Differentiate(root->right), denominator);
            }
            default:
                assert(FAIL && "INVALID OPERATION");
        }
    case TYPE_UNO:
        switch (root->data)
        {
        case 's': {
            Node* diffArgument = Differentiate(root->right);

            Node* diffSin      = MakeNewNode('c', TYPE_UNO, nullptr, Copy(root->right));

            return MakeNewNode((int32_t)'*', TYPE_OP, diffSin, diffArgument);
        }
        case 'c': {
            Node* diffArgument = Differentiate(root->right);
            Node* sin          = MakeNewNode('s', TYPE_UNO, nullptr, Copy(root->right));
            Node* minusOne     = MakeNewNode(-1, TYPE_CONST);
            Node* diffCos      = MakeNewNode((int32_t)'*', TYPE_OP, minusOne, sin);

            return MakeNewNode((int32_t)'*', TYPE_OP, diffCos, diffArgument);
        }
        default:
            break;
        }
        break;
    case TYPE_UNKNOWN:
    default:
        assert(FAIL && "UNKNOWN DATA TYPE");
    }

    return nullptr;
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

    printf("WARNING RETURN NULLPTR\n");
    return nullptr;
}

int32_t FoldConst(Node* node) {
    assert(node != nullptr);
    int32_t returnValue = 0;

    if ((node->type == TYPE_OP) &&
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
        case (int32_t)('l'):
            return 0;
            break;
        default:
            assert(FAIL && "UNKNOWN OPERATION");
        }

        NodeDtor(node->left);
        NodeDtor(node->right);

        node->left  = nullptr;
        node->right = nullptr; 
        return 1;
    }
    else {
        printf("Trying to check node type\n");
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

//TODO Степени, tex, до конца DSL
