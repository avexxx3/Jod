template <typename T> class DoublyLinkedList {
  struct Node {
    T data;
    Node *prev;
    Node *next;

    Node(T _data, Node *_prev, Node *_next)
        : data(_data), prev(_prev), next(_next) {}

    Node() {
      prev = next = 0;
    }
  };

  Node *head;
  Node *tail;

public:
  DoublyLinkedList() { head = tail = new Node(); }

  void insertAtHead(T data) {
    Node *newNode = new Node(data, head, head->next);
  }
};