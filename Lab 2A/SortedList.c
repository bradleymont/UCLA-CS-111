// NAME: Bradley Mont
// EMAIL: bradleymont@gmail.com
// ID: 804993030

#include "SortedList.h"
#include <string.h>
#include <sched.h>

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *    The specified element will be inserted in to
 *    the specified list, which will be kept sorted
 *    in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if (list == NULL) return;
    
    SortedListElement_t* nextElement = list->next;
    
    //iterate until nextElement is greater than element
    while (nextElement->key != NULL && strcmp(nextElement->key, element->key) < 0)
    {
        nextElement = nextElement->next;
    }
    
    if (opt_yield & INSERT_YIELD)
    {
        sched_yield();
    }
    
    SortedListElement_t* prevElement = nextElement->prev;
    prevElement->next = element;
    
    element->prev = prevElement;
    element->next = nextElement;
    
    nextElement->prev = element;
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *    The specified element will be removed from whatever
 *    list it is currently in.
 *
 *    Before doing the deletion, we check to make sure that
 *    next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element)
{
    if (element == NULL) return 1;
    
    SortedListElement_t* nextElement = element->next;
    SortedListElement_t* prevElement = element->prev;
    
    if (nextElement->prev != element || prevElement->next != element)    //check for corrupted pointers
    {
        return 1;
    }
    
    if (opt_yield & DELETE_YIELD)
    {
        sched_yield();
    }
    
    prevElement->next = nextElement;
    nextElement->prev = prevElement;
    
    return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *    The specified list will be searched for an
 *    element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t* currElement = list->next;
    
    if (opt_yield & LOOKUP_YIELD)
    {
        sched_yield();
    }
    
    while (currElement != list)
    {
        if (strcmp(currElement->key, key) == 0)
        {
            return currElement;
        }
        currElement = currElement->next;
    }
    
    return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *    While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *       -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list)
{
    if (list == NULL) return -1;
    
    int length = 0;
    
    SortedListElement_t* currElement = list->next;
    
    if (opt_yield & LOOKUP_YIELD)
    {
        sched_yield();
    }
    
    while (currElement != list)
    {
        //check for corruption
        if (currElement->prev->next != currElement || currElement->next->prev != currElement)
        {
            return -1;
        }
        
        length++;
        currElement = currElement->next;
    }
    
    return length;
}
