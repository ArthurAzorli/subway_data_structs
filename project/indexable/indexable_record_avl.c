#include "indexable_record_avl.h"

#include <stdlib.h>
#include <string.h>

// ===== IndexableRecordAVL Private function ==== \\

/**
 * @brief Calculates the maximum height of a node's children.
 *
 * @param node Pointer to the current AVL node
 * @return Maximum height of left or right child
 */
uint32_t IndexableRecordAVL_heightMax(const struct IndexableRecordAVL* node) {
  const uint32_t a = node->left ? node->left->height : 0;
  const uint32_t b = node->right ? node->right->height : 0;
  return a > b ? a : b;
}

/**
 * @brief Calculates the total size (number of nodes) of a subtree.
 *
 * @param node Pointer to the current AVL node
 * @return Size of the subtree rooted at the node
 */
uint32_t IndexableRecordAVL_calculateSize(const struct IndexableRecordAVL* node) {
  const uint32_t a = node->left ? node->left->size : 0;
  const uint32_t b = node->right ? node->right->size: 0;
  return a + b + 1;
}

/**
 * @brief Initializes a new AVL node with a given record.
 *
 * Allocates memory for a new node and copies the record into it.
 *
 * @param record Pointer to the IndexableRecord to store
 * @return Pointer to the newly created node, or NULL if allocation fails
 */
struct IndexableRecordAVL* IndexableRecordAVL_initNode(const struct IndexableRecord *record) {
  struct IndexableRecordAVL *node = malloc(sizeof(struct IndexableRecordAVL));
  if (node == NULL) return NULL;
  node->record = malloc(sizeof(struct IndexableRecord));
  if (node->record == NULL) {
    free(node);
    return NULL;
  }
  memcpy(node->record, record, sizeof(struct IndexableRecord));
  node->left = NULL;
  node->right = NULL;
  node->height = 1;
  node->size = 1;
  return node;
}

/**
 * @brief Performs a right rotation on the given node.
 *
 * Used to rebalance the AVL tree when the left subtree is heavier.
 *
 * @param node Pointer to the root node of the subtree
 * @return New root after rotation
 */
struct IndexableRecordAVL* IndexableRecordAVL_rightRotation(struct IndexableRecordAVL *node) {
  struct IndexableRecordAVL *x = node->left;
  struct IndexableRecordAVL *y = x->right;
  x->right = node;
  node->left = y;
  node->height = IndexableRecordAVL_heightMax(node) + 1;
  node->size = IndexableRecordAVL_calculateSize(node);
  x->height = IndexableRecordAVL_heightMax(x) + 1;
  x->size = IndexableRecordAVL_calculateSize(x);
  return x;
}

/**
 * @brief Performs a left rotation on the given node.
 *
 * Used to rebalance the AVL tree when the right subtree is heavier.
 *
 * @param node Pointer to the root node of the subtree
 * @return New root after rotation
 */
struct IndexableRecordAVL* IndexableRecordAVL_leftRotation(struct IndexableRecordAVL *node) {
  struct IndexableRecordAVL *x = node->right;
  struct IndexableRecordAVL *y = x->left;
  x->left = node;
  node->right = y;
  node->height = IndexableRecordAVL_heightMax(node) + 1;
  node->size = IndexableRecordAVL_calculateSize(node) ;
  x->height = IndexableRecordAVL_heightMax(x) + 1;
  x->size = IndexableRecordAVL_calculateSize(x);
  return x;
}

/**
 * @brief Writes all nodes of the AVL tree to a file in-order.
 *
 * Traverses the tree recursively and writes each record to the file.
 *
 * @param file Pointer to the DataFile where records will be written
 * @param node Pointer to the current AVL node
 * @return true if all nodes were written successfully, false otherwise
 */
bool IndexableRecordAVL_writeNode(struct DataFile *file, struct IndexableRecordAVL *node) {
  if (file == NULL) return false;
  if (node == NULL) return true;
  if (!IndexableRecordAVL_writeNode(file, node->left)) return false;
  if (!IndexableRecordRepository_writeRecord(file, node->record)) return false;
  if (!IndexableRecordAVL_writeNode(file, node->right)) return false;
  return true;
}



// ===== IndexableRecordAVL Public function ==== \\

/**
 * @brief Inserts a new record into the AVL tree.
 *
 * Balances the tree after insertion to maintain AVL properties.
 *
 * @param root Pointer to the root of the tree
 * @param record Pointer to the record to insert
 * @return New root of the tree after insertion
 */
struct IndexableRecordAVL * IndexableRecordAVL_push(struct IndexableRecordAVL *root, struct IndexableRecord *record) {
  if (root == NULL) return IndexableRecordAVL_initNode(record);
  if (record->stationID < root->record->stationID) {
    root->left = IndexableRecordAVL_push(root->left, record);
  } else if (record->stationID > root->record->stationID) {
    root->right = IndexableRecordAVL_push(root->right, record);
  } else {
    return root;
  }

  root->height = IndexableRecordAVL_heightMax(root) + 1;
  root->size = IndexableRecordAVL_calculateSize(root);
  const uint32_t rightHeight = root->right ? root->right->height : 0;
  const uint32_t leftHeight = root->left ? root->left->height : 0;
  const int balance = leftHeight - rightHeight;

  if (balance > 1) {
    if (record->stationID > root->record->stationID) root->left = IndexableRecordAVL_leftRotation(root->left);
    return IndexableRecordAVL_rightRotation(root);
  }

  if (balance < -1) {
    if (record->stationID < root->record->stationID) root->right = IndexableRecordAVL_rightRotation(root->right);
    return IndexableRecordAVL_leftRotation(root);
  }

  return root;
}

/**
 * @brief Removes a record from the AVL tree by station ID.
 *
 * Balances the tree after removal to maintain AVL properties.
 *
 * @param root Pointer to the root of the tree
 * @param stationID Station ID of the record to remove
 * @return New root of the tree after removal
 */
struct IndexableRecordAVL * IndexableRecordAVL_remove(struct IndexableRecordAVL *root, uint32_t stationID) {
  if (root == NULL) return NULL;
  if (root->record->stationID > stationID) {
    root->left = IndexableRecordAVL_remove(root->left, stationID);
  } else if (root->record->stationID < stationID) {
    root->right = IndexableRecordAVL_remove(root->right, stationID);
  } else {
    if (root->left==NULL || root->right==NULL) {
      struct IndexableRecordAVL *temp = root->left ? root->left : root->right;
      free(root->record);
      free(root);
      return temp;
    }

    const struct IndexableRecordAVL *temp = root->right;
    while (temp->left != NULL) temp = temp->left;
    memcpy(root->record, temp->record, sizeof(struct IndexableRecord));
    root->right = IndexableRecordAVL_remove(root->right, temp->record->stationID);
  }

  root->height = IndexableRecordAVL_heightMax(root) + 1;
  root->size = IndexableRecordAVL_calculateSize(root);
  const uint32_t rightHeight = root->right ? root->right->height : 0;
  const uint32_t leftHeight = root->left ? root->left->height : 0;
  const int balance = leftHeight - rightHeight;

  if (balance > 1) {
    if (root->left != NULL) {
      const uint32_t leftRightHeight = root->left->right ? root->left->right->height : 0;
      const uint32_t leftLeftHeight = root->left->left ? root->left->left->height : 0;
      const int leftBalance = leftLeftHeight - leftRightHeight;
      if (leftBalance < 0) root->left = IndexableRecordAVL_leftRotation(root->left);
    }
    return IndexableRecordAVL_rightRotation(root);
  }

  if (balance < -1) {
    if (root->right != NULL) {
      const uint32_t rightRightHeight = root->right->right ? root->right->right->height : 0;
      const uint32_t rightLeftHeight = root->right->left ? root->right->left->height : 0;
      const int rightBalance = rightLeftHeight - rightRightHeight;
      if (rightBalance > 0) root->right = IndexableRecordAVL_rightRotation(root->right);
    }
    return IndexableRecordAVL_leftRotation(root);
  }

  return root;
}

/**
 * @brief Retrieves a record by station ID.
 *
 * Searches the AVL tree recursively for the given station ID.
 *
 * @param root Pointer to the root of the tree
 * @param stationID Station ID to search for
 * @return Pointer to the matching record, or NULL if not found
 */
struct IndexableRecord * IndexableRecordAVL_getByStationID(const struct IndexableRecordAVL *root, const uint32_t stationID) {
  if (root == NULL) return NULL;
  if (root->record->stationID == stationID) return root->record;
  if (stationID < root->record->stationID) return IndexableRecordAVL_getByStationID(root->left, stationID);
  return IndexableRecordAVL_getByStationID(root->right, stationID);
}

/**
 * @brief Retrieves a record by index (in-order traversal).
 *
 * @param root Pointer to the root of the tree
 * @param index Zero-based index of the record
 * @return Pointer to the record at the given index, or NULL if out of bounds
 */
struct IndexableRecord * IndexableRecordAVL_getByIndex(const struct IndexableRecordAVL *root, const size_t index) {
  if (root == NULL) return NULL;
  const size_t leftSize = root->left != NULL ? root->left->size : 0;
  if (leftSize == index) return root->record;
  if (index < leftSize) return IndexableRecordAVL_getByIndex(root->left, index);
  return IndexableRecordAVL_getByIndex(root->right, index - leftSize - 1);
}

/**
 * @brief Frees all memory associated with the AVL tree.
 *
 * Recursively frees all nodes and their records.
 *
 * @param root Pointer to the root of the tree
 */
void IndexableRecordAVL_free(struct IndexableRecordAVL *root) {
  if (root == NULL) return;
  IndexableRecordAVL_free(root->left);
  IndexableRecordAVL_free(root->right);
  free(root->record);
  free(root);
}

/**
 * @brief Reads all indexable records from a file and builds an AVL tree.
 *
 * @param fileName Path to the file containing indexable records
 * @return Pointer to the root of the constructed AVL tree, or NULL if failed
 */
struct IndexableRecordAVL * IndexableRecordAVL_readFromFile(const char *fileName) {
  if (fileName == NULL) return NULL;

  //open file
  struct DataFile *file = FileRepository_openOrCreate(fileName, READ_ONLY);
  if (file == NULL) return NULL;

  //init params
  struct IndexableRecordAVL *avl = NULL;
  struct IndexableRecord *indexable = NULL;

  //read all index until end file
  while (true) {
    if (!IndexableRecordRepository_readRecord(file, &indexable)) break;
    avl = IndexableRecordAVL_push(avl, indexable);
    free(indexable);
  }

  //close file and return result
  FileRepository_close(file);
  return avl;
}

/**
 * @brief Writes all records from the AVL tree to a file.
 *
 * @param fileName Path to the output file
 * @param root Pointer to the root of the AVL tree
 * @return true if writing succeeded, false otherwise
 */
bool IndexableRecordAVL_writeOnFile(const char *fileName, struct IndexableRecordAVL *root) {
  //open output file
  struct DataFile *file = FileRepository_openOrCreate(fileName, WRITE_ONLY);
  if (file == NULL) return false;

  //write each AVL indexable record in file
  const bool result = IndexableRecordAVL_writeNode(file, root);

  // close file and resturn result
  FileRepository_close(file);
  return result;
}
