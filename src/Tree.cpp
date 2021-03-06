#include "Tree.h"
#include "Graph.h"
#include "Differ.h"

void TreeCtor_(Tree* tree) {
    assert(tree != nullptr);

    tree->qbase = {};
    tree->root = (Node*)calloc(1, sizeof(tree->root[0]));

    tree->unsavedQuestions = (int8_t*)calloc(MAX_BUFFER_SIZE, sizeof(tree->unsavedQuestions[0]));
    tree->bufLen           = 0;
}

void TreeDtor(Tree* tree) {
    assert(tree != nullptr);

    free(tree->unsavedQuestions);

    tree->qbase  = {};
    tree->bufLen = 0;

    NodeDtor(tree->root);
}

void NodeDtor(Node* root) {
    assert(root != nullptr);

    if (root->left  != nullptr) NodeDtor(root->left);
    if (root->right != nullptr) NodeDtor(root->right);

    root->data  = (TreeElem)13;
    root->type  = TYPE_UNKNOWN;
    root->left  = (Node*)13;
    root->right = (Node*)13;
    free(root);
}

Node* MakeNewNode(TreeElem data, NodeDataTypes type, Node* left, Node* right) {
    Node* newNode = (Node*)calloc(1, sizeof(newNode[0]));

    uint32_t nodeWeight = 1;

    if (left != nullptr) {
        nodeWeight += left->weight;
    }
    if (right != nullptr) {
        nodeWeight += right->weight;
    }
    *newNode = {data, type, nodeWeight, left, right};

    return newNode;
}

void PrintTreeNodes(Tree* tree, Node* root, FILE* output) {
    assert(root != nullptr);

    static int64_t rankArray[MAX_RECURSION_DEPTH] = {};
    static int32_t curRecursionDepth = 1;
    static int32_t curNodeNumber = 0;
    static Node* pointerAnchor = root;

    if (root == tree->root) {
        printf("Dropped\n");
        curRecursionDepth = 1;
        curNodeNumber     = 0;
        pointerAnchor     = root;

        for (uint32_t curIdx = 0; curIdx < MAX_RECURSION_DEPTH; curIdx++) {
            rankArray[curIdx] = 0;
        }
    }

    char nodeData[MAX_NODE_DATA_LENGTH] = "";

    switch (root->type) {
    case TYPE_CONST:
        itoa(root->data, nodeData, 10);
        break;
    case TYPE_OP:
        if (root->data == 'l') {
            strcat(nodeData, "log");
        }
        else {
            nodeData[0] = (char)root->data;
        }
        break;
    case TYPE_VAR:
        nodeData[0] = (char)root->data;
        break;
    case TYPE_UNO:
        switch (root->data) {
            case (int32_t)'s':
                strcat(nodeData, "sin");
                break;
            case (int32_t)'c':
                strcat(nodeData, "cos");
                break;
            default:
                assert(FAIL && "UNKNOWN UNO OPERAND");
                break;
        }
        break;
    case TYPE_UNKNOWN:
        assert(FAIL && "UNKNOWN OPERAND TYPE");
        break;
    default:
        assert(FAIL && "UNKNOWN OPERAND TYPE");
        break;
    }

    char dataConverted[MAX_NODE_DATA_LENGTH] = "";
    Convert1251ToUtf8((const char*)nodeData, dataConverted);

    curNodeNumber++;    
    
    if (rankArray[curRecursionDepth] == 0) {
        rankArray[curRecursionDepth] = root - pointerAnchor;
    }
    else {
        fprintf(output, "\t{rank = same; %lld; %lld;}\n", rankArray[curRecursionDepth], root - pointerAnchor);
    }

    #if (_ENABLE_PTR_ADRESSES_VIEW_ == 1)
        fprintf(output, "\tsubgraph cluster%d {\n", curNodeNumber);
        fprintf(output, "\t\tstyle = filled;\n"
                        "\t\tcolor = lightgreen;\n"
                        "\t\t%lld;\n"
                        "\t\tlabel = \"%lld\";\n", root - pointerAnchor, root - pointerAnchor);
        fprintf(output, "\t}\n");
    #endif

    if ((root->left != nullptr) && (root->right != nullptr))  {
        fprintf(output, 
            "\t%lld[shape=plaintext, label = <"
            "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING= \"0\" CELLPADDING=\"4\">"
            "<TR>"
                "<TD COLSPAN=\"2\">" TREE_TYPE "[%u]</TD>"
            "</TR>"
            "<TR>"
                "<TD PORT = \"l%d\">" LEFT_BRANCH "</TD>"
                "<TD PORT = \"r%d\">" RIGHT_BRANCH "</TD>"
            "</TR>"
            "</TABLE>>];\n",
            root - pointerAnchor, dataConverted, root->weight,
            curNodeNumber, LEFT_BRANCH_VALUE, 
            curNodeNumber, RIGHT_BRANCH_VALUE);

        curRecursionDepth += 1;

        fprintf(output, "\t\t%lld: <l%d> -> %lld;\n", 
                root - pointerAnchor, curNodeNumber, root->left  - pointerAnchor);
        fprintf(output, "\t\t%lld: <r%d> -> %lld;\n", 
                root - pointerAnchor, curNodeNumber, root->right - pointerAnchor);

        PrintTreeNodes(tree, root->left, output);
        PrintTreeNodes(tree, root->right, output);
        curRecursionDepth--;
    }
    else if ((root->left == nullptr) && (root->right != nullptr)) {
        fprintf(output, 
            "\t%lld[shape=plaintext, label = <"
            "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING= \"0\" CELLPADDING=\"4\">"
            "<TR>"
                "<TD COLSPAN=\"2\">" TREE_TYPE "[%u]</TD>"
            "</TR>"
            "<TR>"
                "<TD PORT = \"r%d\">" RIGHT_BRANCH "</TD>"
            "</TR>"
            "</TABLE>>];\n",
            root - pointerAnchor, dataConverted, root->weight,
            curNodeNumber, RIGHT_BRANCH_VALUE);

        curRecursionDepth += 1;

        fprintf(output, "\t\t%lld: <r%d> -> %lld;\n", 
                root - pointerAnchor, curNodeNumber, root->right - pointerAnchor);

        PrintTreeNodes(tree, root->right, output);
        curRecursionDepth--;
    }
    else {
        fprintf(output, 
            "\t%lld[shape = plaintext, label = <"
            "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING= \"0\" CELLPADDING=\"4\">"
            "<TR>"
                "<TD COLSPAN=\"2\">" TREE_TYPE "</TD>"
            "</TR>"
            "<TR>"
                "<TD> 0 </TD>"
                "<TD> 0 </TD>"
            "</TR>"
            "</TABLE>>];\n", 
            root - pointerAnchor, dataConverted);
    }
}

bool VerifyTree(Node* root) {
    assert(root != nullptr);

    if ((root->left == nullptr) != (root->right == nullptr)) {
        return 1;
    }

    if (root->left  != nullptr) return VerifyTree(root->left);
    if (root->right != nullptr) return VerifyTree(root->right);
    return 0;
}
