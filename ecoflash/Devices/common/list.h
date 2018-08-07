/*
 * list.h
 *
 * List (thread safe) - header file
 *
 * Author: Filip Kinovic
 * Version: 1.6
 * Date: 26.06.2017
*/

#include <windows.h>
#include <stdio.h>

#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif


/* type definitions */
typedef HANDLE HLIST;
typedef HANDLE HITEM;

typedef enum {
	IPTYPE_CREATE,
	IPTYPE_DELETE,
	IPTYPE_PRINT,
	IPTYPE_COMPARE,
} IPROCTYPE;

typedef BOOL (*ITEMPROC)(LPARAM *iparam, IPROCTYPE type, LPARAM lparam);

typedef struct {
	LPARAM lparam;		//data
	void *previous;
	void *next;
} LISTITEM;

typedef struct {
	DWORD size;
	LISTITEM *first;
	LISTITEM *last;
	ITEMPROC iproc;		//process item
	CRITICAL_SECTION cs;
} LISTBODY;

typedef enum {
	LPOS_FIRST,		//hObj=hlist, idx=ignored
	LPOS_LAST,		//hObj=hlist, idx=ignored
	LPOS_NEXT,		//hObj=hitem, idx=ignored
	LPOS_PREV,		//hObj=hitem, idx=ignored
	LPOS_INDEX,		//hObj=hlist, idx=index
} LISTPOS;

typedef enum {
	LIST_SORTMODE_ASCEND = 0,
	LIST_SORTMODE_DESCEND = 1,
} LIST_SORTMODE;

/* constants */


/* function declarations */
HLIST LIST_Create(ITEMPROC iproc);
BOOL LIST_Discard(HLIST hList);
//
HITEM LIST_SearchItemByIndex(HLIST hList, DWORD idx);
BOOL LIST_InsertItem(HLIST hList, DWORD idx, LPARAM lparam);
BOOL LIST_AppendItem(HLIST hList, LPARAM lparam);
BOOL LIST_PrependItem(HLIST hList, LPARAM lparam);
BOOL LIST_RemoveItem(HLIST hList, DWORD idx);
BOOL LIST_RemoveItemByHandle(HLIST hList, HITEM hItem);
BOOL LIST_RemoveFirstItem(HLIST hList);
BOOL LIST_RemoveLastItem(HLIST hList);
BOOL LIST_RemoveAll(HLIST hList);
//
DWORD LIST_GetSize(HLIST hList);
//
HITEM LIST_GetItem(HANDLE hObj, LISTPOS pos, DWORD idx);
BOOL LIST_GetItemValue(HITEM hItem, LPARAM *lparam);
BOOL LIST_SetItemValue(HLIST hList, HITEM hItem, LPARAM lparam);
//
DWORD LIST_Copy(HLIST des_hList, HLIST src_hList);
BOOL LIST_Compare(HLIST hList1, HLIST hList2, int (*compar)(LPARAM, LPARAM));
//
BOOL LIST_Printf(HLIST hList, FILE *f);
//
BOOL LIST_BubbleSort(HLIST hList, LIST_SORTMODE mode);
//
BOOL LIST_IProcATextParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam);
BOOL LIST_IProcTextParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam);
//can be used as sub function
BOOL LIST_IProcMemoryParam(unsigned int size, LPARAM *iparam, IPROCTYPE type, LPARAM lparam);
//
BOOL LIST__Test(void);


#ifdef __cplusplus
}
#endif


#endif		/* _LIST_H_ */

