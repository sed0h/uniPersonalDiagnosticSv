#ifndef __LIST_H__
#define __LIST_H__

struct ListItem {
	volatile uint32_t item_value;
	struct ListItem * volatile next;
	struct ListItem * volatile previous;
	void * owner;	 			//point to the object (normally a TCB) that contains the list item.
	void * volatile container;	//point to which this list item is placed
};
typedef struct ListItem ListItem_t;

struct MINIListItem {
	volatile uint32_t item_value;
	struct ListItem * next;
	struct ListItem * previous;
};
typedef struct MINIListItem MINIListItem_t;

typedef  struct  LIST {
	volatile uint32_t number_of_items;
	ListItem_t * volatile index;		//Used to walk through the list
	MINIListItem_t list_end;
} List_t;

#define listLIST_IS_EMPTY( pxList )	( ( uint32_t ) ( ( pxList )->number_of_items == ( uint32_t ) 0 ) )
#define listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxList )	( ( ( pxList )->list_end ).next->item_value )
#define listGET_OWNER_OF_HEAD_ENTRY( pxList )  ( (&( ( pxList )->list_end ))->next->owner )
#define listIS_CONTAINED_WITHIN(pxList, pxListItem ) ( ( uint32_t ) ( ( pxListItem )->container == ( void * ) ( pxList ) ) )
#define listSET_LIST_ITEM_VALUE( pxListItem, xValue )	( ( pxListItem )->item_value = ( xValue ) )
#define listSET_LIST_ITEM_OWNER( pxListItem, pxOwner )		( ( pxListItem )->owner = ( void * ) ( pxOwner ) )

class ListInterface {
public:
	ListInterface();
	~ListInterface();
	void InitialiseList(List_t* const list);
	void InitialiseListItem(ListItem_t* const pxItem);
	uint32_t RemoveItem(ListItem_t* const pxItemToRemove);
	void InsertItem(List_t *const pxList, ListItem_t *const pxNewListItem);

private:

};


#endif