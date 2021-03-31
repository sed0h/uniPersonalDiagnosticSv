#include <cstdint>
#include "List.h"
#include "portmacro.h"

void ListInterface::InitialiseListItem(ListItem_t* const pxItem) {
	pxItem->container = nullptr;
}

void ListInterface::InitialiseList(List_t* const list) {
	list->index = (ListItem_t *)& (list->list_end);
	
	list->list_end.item_value = portMAX_DELAY;

	list->list_end.previous = (ListItem_t*)&(list->list_end);
	list->list_end.next = (ListItem_t*)&(list->list_end);

	list->number_of_items = 0;
}

uint32_t ListInterface::RemoveItem(ListItem_t* const pxItemToRemove) {
	List_t* const px_list = (List_t*)pxItemToRemove->container;
	
	pxItemToRemove->previous->next = pxItemToRemove->next;
	pxItemToRemove->next->previous = pxItemToRemove->previous;

	if (px_list->index == pxItemToRemove) {
		px_list->index = pxItemToRemove->previous;
	}

	pxItemToRemove->container = NULL;
	(px_list->number_of_items)--;

	return px_list->number_of_items;
}

void ListInterface::InsertItem(List_t* const pxList, ListItem_t* const pxNewListItem) {
	ListItem_t* px_iterator;
	const TickType_t value_of_insertion = pxNewListItem->item_value;

	if (value_of_insertion == portMAX_DELAY) {
		px_iterator = pxList->list_end.previous;
	} else {
		for ( px_iterator = (ListItem_t *) &(pxList->list_end); value_of_insertion > (px_iterator->next->item_value); px_iterator = px_iterator->next) {

		}
	}

	pxNewListItem->previous = px_iterator;
	pxNewListItem->next = px_iterator->next;
	pxNewListItem->next->previous = pxNewListItem;
	px_iterator->next = pxNewListItem;
	//同一线的操作完成后再另一条线，不是同一个节点操作完之后再另一个节点

	pxNewListItem->container = (void*)pxList;

	(pxList->number_of_items)++;
}


ListInterface::ListInterface() {
}

ListInterface::~ListInterface() {
}