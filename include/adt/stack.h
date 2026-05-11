#ifndef STACK_H
#define STACK_H
/*
ADT STACK (ACTION HISTORY)

Represents a runtime-only stack used to
store user actions during program execution.

This structure is VOLATILE:
- not persisted to file
- exists only in RAM

Use case:
- tracking user operations
- simulating undo-like behavior
- logging system activity
*/

#define MAX_STACK 100
/*
The internal structure is hidden to enforce information hiding.
*/
typedef struct Stack* Stack; 
//STACK OPERATORS 
/*
create_stack -> Creates and initializes an empty stack.

Preconditions:
- none

Postconditions:
- returns pointer to Stack
- top initialized to -1
*/
Stack create_stack();
/*
push -> Pushes an action onto the stack.

Preconditions:
- stack must be valid (not NULL)
- stack must not be full

Postconditions:
- action is added to top
- top is incremented

Side effects:
- modifies stack state
*/
void push(Stack s, const char* action);
/*
pop -> Removes and returns last action.

Preconditions:
- stack must be valid
- stack must not be empty

Postconditions:
- top is decremented
- returns removed action string

Side effects:
- modifies stack state
*/
char* pop(Stack s);

#endif

/*
ACTION STRUCTURE -> Represents a single user action.
Invariants:
- action is a null-terminated string
- max length is 100 characters
*/
typedef struct {
    char action[100];
} Action;

/*
STACK STRUCTURE -> Represents a fixed-size stack (array-based).
Invariants:
- top >= -1 and top < MAX_STACK
- top == -1 → empty stack
- data[0..top] contains valid actions
*/
 struct Stack {
    Action data[MAX_STACK];
    int top;
};