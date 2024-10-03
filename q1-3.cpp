// Q1
// void deleteNodeWithoutHead(Node *ptr) {
//   Node *prevPtr = ptr;
//   while (ptr->next) {
//     ptr->val = ptr->next->val;
//     prevPtr = ptr;
//     ptr = ptr->next;
//   }

//   prevPtr->next = nullptr;
// }

// void deleteNode(T val) {
//   Node *ptr = head;
//   while (ptr) {
//     if (ptr->data == val) {
//       deleteParticular(ptr);
//       return;
//     }
//     ptr = ptr->next;
//   }
// }

// void deleteParticular(Node *ptr) {
//   if (ptr->prev)
//     ptr->prev->next = ptr->next;
//   if (ptr->next)
//     ptr->next->prev = ptr->prev;
// }

// Q2

//  vector<int> nextLargerNodes(ListNode *head) {
//    vector<int> arr = {};
//    for (ListNode *ptr = head; ptr; ptr = ptr->next) {
//      int val = 0;
//      ListNode *innerPtr = ptr->next;
//      while (innerPtr && innerPtr->val <= ptr->val) {
//        innerPtr = innerPtr->next;
//      }
//      if (innerPtr)
//        val = innerPtr->val;

//     arr.emplace_back(val);
//   }
//   return arr;
// }

// Q3
// ListNode *removeNthFromEnd(ListNode *head, int n) {
//   int size = 0;
//   ListNode *ptr = head;
//   while (ptr) {
//     ptr = ptr->next;
//     size++;
//   }

//   if ((size == 1 && n >= 1) || size == 0)
//     return nullptr;

//   if (size == n)
//     return head->next;

//   int count = 0;
//   ptr = head;
//   while (size - n != count + 1) {
//     ptr = ptr->next;
//     count++;
//   }

//   ptr->next = ptr->next->next;
//   return head;
// }