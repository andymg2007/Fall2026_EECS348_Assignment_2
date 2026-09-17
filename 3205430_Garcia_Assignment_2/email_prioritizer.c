/* ============================================================
 * FILE NAME: main.c (CEO Email Priority System)
 *
 * DESCRIPTION:
 *   Implements a custom MaxHeap-based priority queue for managing
 *   a CEO's email inbox using dynamically allocated memory in C.
 *   Reads interactive or file-redirected commands from standard
 *   input (EMAIL, NEXT, READ, COUNT) and orders emails by sender
 *   category priority and date recency.
 *
 * INPUTS:
 *   Command inputs via stdin:
 *     - EMAIL <category>, <subject>, <MM-DD-YYYY>
 *     - NEXT
 *     - READ
 *     - COUNT
 *
 * OUTPUTS:
 *   Terminal output providing feedback on commands, queue status,
 *   and displayed email details.
 *
 * COLLABORATORS:
 *   Gemini assisted with comments and understanding Claude's code logic as well as assisting with the code analysis
 *
 * SOURCES:
 *   Claude for creating the original file structure and heap logic,
 *   ChatGPT for help creating date comparison optimization and string parsing
 *
 * AUTHOR:
 *   Andy Garcia
 *
 * CREATION DATE:
 *   September 12, 2026
 *
 * REVISION DATE:
 *   September 17, 2026
 *
 * REVISIONS:
 *   Updated date parsing to composite integer, reordered struct fields,
 *   added line-by-line comments, and adjusted output formatting/newlines.
 */


#include <stdio.h>   // include standard input output library
#include <stdlib.h>  // include standard library for dynamic memory management
#include <string.h>  // include string manipulation library


#define INITIAL_CAPACITY 8      // create constant for initial capacity of the heap array
#define MAX_LINE_LEN     512    // create constant for maximum input line buffer length
#define MAX_SENDER_LEN   64     // create constant for maximum sender category string length
#define MAX_SUBJECT_LEN  256    // create constant for maximum subject string length
#define MAX_DATE_LEN     11     // create constant for maximum date string length "MM-DD-YYYY"


typedef struct {
    int  dateValue;                     // create integer variable dateValue to store composite date YYYYMMDD
    int  priority;                      // create integer variable priority to store category rank
    char sender[MAX_SENDER_LEN];        // create character array sender to store category name
    char subject[MAX_SUBJECT_LEN];      // create character array subject to store email subject
    char dateStr[MAX_DATE_LEN];         // create character array dateStr to store formatted date string
} Email; // define Email struct type


typedef struct {
    Email *heap;     // create pointer heap to dynamic array of Email structs
    int size;        // create integer variable size to track current element count
    int capacity;    // create integer variable capacity to track allocated array capacity
} MaxHeap; // define MaxHeap struct type


// static function to remove leading and trailing whitespace from a string (authored by Claude)
static void trim(char *s) {
    int start = 0; // create integer variable start initialized to 0
   
    while (s[start] == ' ' || s[start] == '\t') { // loop while character is a space or tab
        start++; // increment start index by 1
    }
   
    if (start > 0) { // check if leading whitespace was found
        memmove(s, s + start, strlen(s + start) + 1); // shift string left to remove leading whitespace
    }


    int len = (int)strlen(s); // create integer variable len equal to string length
    while (len > 0 && (s[len - 1] == ' '  || s[len - 1] == '\t' ||
                       s[len - 1] == '\n' || s[len - 1] == '\r')) { // loop while trailing char is whitespace
        s[len - 1] = '\0'; // set trailing whitespace character to null terminator
        len--; // decrement len by 1
    }
}


// static function to convert category string to integer priority rank (authored by Claude)
static int categoryToPriority(const char *category) {
    if (strcmp(category, "Boss") == 0)            return 5; // checks if category is Boss, returns priority 5
    if (strcmp(category, "Subordinate") == 0)     return 4; // checks if category is Subordinate, returns priority 4
    if (strcmp(category, "Peer") == 0)            return 3; // checks if category is Peer, returns priority 3
    if (strcmp(category, "ImportantPerson") == 0) return 2; // checks if category is ImportantPerson, returns priority 2
    if (strcmp(category, "OtherPerson") == 0)     return 1; // checks if category is OtherPerson, returns priority 1
    return 0; // returns 0 if category is unrecognized
}


// static function to parse date string into composite integer (concept from ChatGPT, validation added by author)
static int parseDate(const char *dateStr, int *outDateValue) {
    int month, day, year; // create integer variables for month, day, and year


    if (sscanf(dateStr, "%d-%d-%d", &month, &day, &year) != 3) { // attempts to parse MM-DD-YYYY from dateStr
        return 0; // returns 0 to indicate parsing error
    }


    if (month < 1 || month > 12) { // checks if month is outside range 1 to 12
        return 0; // returns 0 for invalid month
    }


    if (year < 1900 || year > 2100) { // checks if year is outside valid bounds
        return 0; // returns 0 for invalid year
    }


    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // create array of day limits
   
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) { // checks if year is a leap year
        daysInMonth[2] = 29; // updates February days limit to 29 for leap year
    }


    if (day < 1 || day > daysInMonth[month]) { // checks if day is within valid range for the month
        return 0; // returns 0 for invalid day
    }


    *outDateValue = (year * 10000) + (month * 100) + day; // calculates composite integer YYYYMMDD
    return 1; // returns 1 to indicate successful date parsing
}


// function to compare two emails based on priority and date (authored by Andy Garcia)
int compare(Email a, Email b) {
    if (a.priority != b.priority) { // checks if priority values are different
        return (a.priority > b.priority) ? 1 : -1; // returns 1 if a priority is higher, else -1
    }


    if (a.dateValue != b.dateValue) { // checks if date values are different when priorities tie
        return (a.dateValue > b.dateValue) ? 1 : -1; // returns 1 if a date is newer, else -1
    }


    return 0; // returns 0 if both priority and date are equal
}


// function to initialize heap memory structure (authored by Claude)
void initHeap(MaxHeap *hp) {
    hp->heap = (Email *)malloc(sizeof(Email) * INITIAL_CAPACITY); // allocates initial dynamic array buffer
    if (hp->heap == NULL) { // checks if memory allocation failed
        fprintf(stderr, "Fatal error: Allocation of heap memory failed.\n"); // prints error message to stderr
        exit(EXIT_FAILURE); // exits program with failure code
    }
    hp->size = 0; // sets heap size equal to 0
    hp->capacity = INITIAL_CAPACITY; // sets heap capacity equal to INITIAL_CAPACITY
}


// static function to resize heap array when capacity limit is reached (authored by Claude)
static void resizeHeap(MaxHeap *hp) {
    int newCapacity = hp->capacity * 2; // creates variable newCapacity equal to double the current capacity
    Email *newBlock = (Email *)realloc(hp->heap, sizeof(Email) * newCapacity); // reallocates dynamic array buffer
    if (newBlock == NULL) { // checks if memory reallocation failed
        fprintf(stderr, "Fatal error: Resizing heap memory failed.\n"); // prints error message to stderr
        free(hp->heap); // frees previously allocated heap array memory
        exit(EXIT_FAILURE); // exits program with failure code
    }
    hp->heap = newBlock; // assigns reallocated memory block to heap pointer
    hp->capacity = newCapacity; // updates capacity variable to newCapacity
}


// static function to swap two email structs in memory (authored by Claude)
static void swapEmail(Email *a, Email *b) {
    Email temp = *a; // creates struct variable temp equal to dereferenced a
    *a = *b;        // assigns dereferenced b to dereferenced a
    *b = temp;      // assigns temp struct value to dereferenced b
}


// static function to restore max heap property going upward (authored by Claude)
static void heapifyUp(MaxHeap *hp, int index) {
    while (index > 0) { // loops while current node is not the root
        int parent = (index - 1) / 2; // calculates parent node array index
       
        if (compare(hp->heap[index], hp->heap[parent]) > 0) { // checks if child outranks parent
            swapEmail(&hp->heap[index], &hp->heap[parent]); // swaps child and parent elements
            index = parent; // updates index variable to parent position
        } else { // handles case where parent outranks or equals child
            break; // breaks out of loop early
        }
    }
}


// static function to restore max heap property going downward (authored by Claude)
static void heapifyDown(MaxHeap *hp, int index) {
    while (1) { // loops indefinitely until heap property is restored
        int left = 2 * index + 1;  // calculates left child array index
        int right = 2 * index + 2; // calculates right child array index
        int largest = index;       // initializes variable largest equal to index


        if (left < hp->size && compare(hp->heap[left], hp->heap[largest]) > 0) { // checks if left child is larger
            largest = left; // sets largest index equal to left child index
        }


        if (right < hp->size && compare(hp->heap[right], hp->heap[largest]) > 0) { // checks if right child is larger
            largest = right; // sets largest index equal to right child index
        }


        if (largest == index) { // checks if current node is larger than both children
            break; // breaks out of loop as heap property holds
        }


        swapEmail(&hp->heap[index], &hp->heap[largest]); // swaps current node with largest child
        index = largest; // updates index variable to largest child position
    }
}


// function to insert email item into the max heap (authored by Claude)
void insert(MaxHeap *hp, Email email) {
    if (hp->size == hp->capacity) { // checks if heap size has reached total capacity
        resizeHeap(hp); // calls resizeHeap to expand dynamic array capacity
    }
    hp->heap[hp->size] = email; // places new email struct at trailing index of array
    heapifyUp(hp, hp->size);     // calls heapifyUp to restore max heap ordering
    hp->size++;                 // increments total heap element count size by 1
}


// function to return highest priority email without removing it (authored by Claude)
Email peek(MaxHeap *hp) {
    if (hp->size == 0) { // checks if heap contains no elements
        Email empty = {0, 0, "", "", ""}; // creates empty fallback struct instance
        return empty; // returns dummy empty struct
    }
    return hp->heap[0]; // returns highest priority email located at array index 0
}


// function to remove and return highest priority email (authored by Claude)
Email extractMax(MaxHeap *hp) {
    Email top = peek(hp); // calls peek function to store current highest priority email


    if (hp->size == 0) { // checks if heap is empty
        return top; // returns dummy email struct
    }


    hp->heap[0] = hp->heap[hp->size - 1]; // replaces root element with last element in heap
    hp->size--; // decrements total heap element count size by 1
   
    if (hp->size > 0) { // checks if elements remain in heap
        heapifyDown(hp, 0); // calls heapifyDown starting at index 0 to restore max heap
    }


    return top; // returns extracted highest priority email struct
}


// function to free dynamic memory used by heap array (authored by Claude)
void freeHeap(MaxHeap *hp) {
    if (hp->heap != NULL) { // checks if dynamic heap buffer pointer is not NULL
        free(hp->heap);   // frees dynamically allocated memory buffer
        hp->heap = NULL;  // resets heap pointer variable to NULL
    }
    hp->size = 0;       // resets size counter equal to 0
    hp->capacity = 0;   // resets capacity tracking variable equal to 0
}


// static function to process EMAIL input command line (authored by Claude and modified by Gemini and Andy Garcia)
static void handleEmailCommand(MaxHeap *hp, char *args) {
    char category[MAX_SENDER_LEN]; // create character array category to hold parsed category string
    char subject[MAX_SUBJECT_LEN]; // create character array subject to hold parsed subject string
    char dateStr[MAX_DATE_LEN];    // create character array dateStr to hold parsed date string


    char *part1 = strtok(args, ","); // parses string up to first comma delimiter
    char *part2 = strtok(NULL, ","); // parses string up to second comma delimiter
    char *part3 = strtok(NULL, ","); // parses string up to third comma delimiter


    if (part1 == NULL || part2 == NULL || part3 == NULL) { // checks if any of the three tokens are missing
        printf("Error: malformed EMAIL command. Expected format:\n"); // prints error format message to user
        printf("  EMAIL <category>, <subject>, <MM-DD-YYYY>\n"); // prints expected syntax structure
        return; // exits command handler early
    }


    trim(part1); // calls trim to strip whitespace from category string
    trim(part2); // calls trim to strip whitespace from subject string
    trim(part3); // calls trim to strip whitespace from date string


    strncpy(category, part1, MAX_SENDER_LEN - 1); // copies category token into category array
    category[MAX_SENDER_LEN - 1] = '\0'; // sets null terminator at last element of category array
   
    strncpy(subject, part2, MAX_SUBJECT_LEN - 1); // copies subject token into subject array
    subject[MAX_SUBJECT_LEN - 1] = '\0'; // sets null terminator at last element of subject array
   
    strncpy(dateStr, part3, MAX_DATE_LEN - 1); // copies date token into dateStr array
    dateStr[MAX_DATE_LEN - 1] = '\0'; // sets null terminator at last element of dateStr array


    int priority = categoryToPriority(category); // calls categoryToPriority to get numeric priority integer
    if (priority == 0) { // checks if input category string was invalid
        printf("Error: unknown sender category '%s'. Valid categories are:\n", category); // prints unknown error message
        printf("  Boss, Subordinate, Peer, ImportantPerson, OtherPerson\n"); // prints valid options
        return; // exits command handler early
    }


    int dateValue; // create integer variable dateValue to store composite date integer
    if (!parseDate(dateStr, &dateValue)) { // calls parseDate and checks if date string is invalid
        printf("Error: invalid date or format '%s'. Expected valid MM-DD-YYYY.\n", dateStr); // prints invalid date error
        return; // exits command handler early
    }


    Email e; // create Email struct variable e
    e.priority = priority; // sets struct priority field equal to priority variable
    e.dateValue = dateValue; // sets struct dateValue field equal to dateValue variable
   
    strncpy(e.sender, category, MAX_SENDER_LEN - 1); // copies category string into struct sender array
    e.sender[MAX_SENDER_LEN - 1] = '\0'; // sets null terminator at end of sender field
   
    strncpy(e.subject, subject, MAX_SUBJECT_LEN - 1); // copies subject string into struct subject array
    e.subject[MAX_SUBJECT_LEN - 1] = '\0'; // sets null terminator at end of subject field
   
    strncpy(e.dateStr, dateStr, MAX_DATE_LEN - 1); // copies dateStr string into struct dateStr array
    e.dateStr[MAX_DATE_LEN - 1] = '\0'; // sets null terminator at end of dateStr field


    insert(hp, e); // inserts initialized email struct e into max heap
}


// static function to execute NEXT command (authored by Claude)
static void handleNextCommand(MaxHeap *hp) {
    if (hp->size == 0) { // checks if heap contains no email elements
        printf("No emails in the queue.\n"); // prints queue empty message to user
        return; // exits function early
    }
    Email top = peek(hp); // retrieves top email struct using peek function
    printf("Next email:\n"); // prints next email header label
    printf("\tSender: %s\n", top.sender); // prints email sender category with leading tab
    printf("\tSubject: %s\n", top.subject); // prints email subject line with leading tab
    printf("\tDate: %s\n", top.dateStr); // prints email formatted date string with leading tab
}


// static function to execute READ command (authored by Claude)
static void handleReadCommand(MaxHeap *hp) {
    if (hp->size == 0) { // checks if heap contains no email elements
        printf("No emails to read.\n"); // prints empty queue message to user
        return; // exits function early
    }
    extractMax(hp); // removes top priority email element from max heap
}


// static function to execute COUNT command (authored by Claude)
static void handleCountCommand(MaxHeap *hp) {
    printf("\nThere are %d emails to read.\n", hp->size); // prints total number of unread emails in heap followed by two newlines
}


// initialize main execution entry point (authored by Claude and Gemini and Andy Garcia)
int main(void) {
    MaxHeap hp; // create MaxHeap struct variable hp
    initHeap(&hp); // calls initHeap to initialize heap memory structure


    char line[MAX_LINE_LEN]; // create character array line buffer to store standard input lines


    while (fgets(line, sizeof(line), stdin) != NULL) { // loops while reading standard input line-by-line
       
        size_t len = strlen(line); // creates size_t variable len equal to input line string length
        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) { // checks if trailing newline exists
            line[len - 1] = '\0'; // strips trailing newline character from end of string
        }


        char trimmedLine[MAX_LINE_LEN]; // create character array buffer trimmedLine
        strncpy(trimmedLine, line, MAX_LINE_LEN - 1); // copies input line into trimmedLine buffer
        trimmedLine[MAX_LINE_LEN - 1] = '\0'; // sets null terminator at last array element
        trim(trimmedLine); // calls trim to clean whitespace surrounding command string


        if (strlen(trimmedLine) == 0) { // checks if trimmed command line is empty
            continue; // skips rest of loop for blank lines
        }


        if (strncmp(trimmedLine, "EMAIL ", 6) == 0) { // checks if command line begins with EMAIL
            handleEmailCommand(&hp, trimmedLine + 6); // calls handleEmailCommand passing string arguments
        } else if (strcmp(trimmedLine, "NEXT") == 0) { // checks if command line is equal to NEXT
            handleNextCommand(&hp); // calls handleNextCommand function
        } else if (strcmp(trimmedLine, "READ") == 0) { // checks if command line is equal to READ
            handleReadCommand(&hp); // calls handleReadCommand function
        } else if (strcmp(trimmedLine, "COUNT") == 0) { // checks if command line is equal to COUNT
            handleCountCommand(&hp); // calls handleCountCommand function
        } else { // handles case where input command is unrecognized
            printf("Unknown command: %s\n", trimmedLine); // prints unknown command error message
        }
    }


    freeHeap(&hp); // calls freeHeap to release all allocated heap array memory
    return 0; // returns 0 to terminate program successfully
}

