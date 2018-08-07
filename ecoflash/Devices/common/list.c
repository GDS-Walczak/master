/*
 * list.c
 *
 * List (thread safe) - source file
 *
 * Author: Filip Kinovic
 * Version: 1.6
 * Date: 26.05.2017
*/


#include "list.h"
#include "memwatch.h"

/* constants */


/* function definitions */

//--------------------------------------------------------------------
//*** function creates list (allocate memory)
//returns handle to LIST; it must be freed
HLIST LIST_Create(ITEMPROC iproc)
{
	LISTBODY *plist;

	//create structure
	plist = (LISTBODY *)malloc(sizeof(LISTBODY));
	if (plist == NULL)
		return(NULL);		//error out of memory
	memset(plist, 0, sizeof(LISTBODY));

	//initiate struct.
	plist->iproc = iproc;

	//init critical section
	InitializeCriticalSection(&plist->cs);

	return((HLIST)plist);
}

//*** function discards circular buffer (free memory)
//note: input handle should be set to NULL, after using this function
BOOL LIST_Discard(HLIST hList)
{
	LISTBODY *plist;

	if (hList == NULL)
		return(FALSE);
	plist = (LISTBODY *)hList;

	//remove all items
	LIST_RemoveAll(hList);

	//delete critical section
	DeleteCriticalSection(&plist->cs);

	//free structure
	free((void *)plist);
	return(TRUE);	//no error
}

//--------------------------------------------------------------------
//*** function searches LIST item by index
//note: searches from first or last item
HITEM LIST_SearchItemByIndex(HLIST hList, DWORD idx)
{
	LISTBODY *plist;
	LISTITEM *pitem = NULL;

	if (hList) {
		plist = (LISTBODY *)hList;

		EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

		//test index
		if (idx < plist->size) {
			//search for item
			if (idx <= plist->size/2) {
				//search from the beginning
				pitem = (LISTITEM *)plist->first;
				while (pitem && idx) {
					pitem = pitem->next;
					idx--;
				}
			}
			else {
				//search from the end
				pitem = (LISTITEM *)plist->last;
				idx = plist->size - idx - 1;
				while (pitem && idx) {
					pitem = pitem->previous;
					idx--;
				}
			}
		}

		LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
	}
	return(pitem);
}


//*** function inserts new item selected by index to the LIST
//idx: 0 -> as first, (DWORD)-1 -> as last
BOOL LIST_InsertItem(HLIST hList, DWORD idx, LPARAM lparam)
{
	LISTBODY *plist;
	LISTITEM *pitem, *pnew;

	if (hList == NULL)
		return(FALSE);
	plist = (LISTBODY *)hList;

	//allocation
	if ((pnew = (LISTITEM *)malloc(sizeof(LISTITEM))) == NULL)
		return(FALSE);
	memset(pnew, 0, sizeof(LISTITEM));
	pnew->lparam = lparam;		//set value

	EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

	//iproc
	if (plist->iproc)
		plist->iproc(&pnew->lparam, IPTYPE_CREATE, lparam);

	//search for item
	pitem = LIST_SearchItemByIndex(hList, idx);

	//insertion
	if (pitem) {
		if (pitem == plist->first) {
			//insert as first
			pnew->previous = NULL;
			pnew->next = plist->first;
			plist->first->previous = pnew;
			plist->first = pnew;
		}
		else {
			//insert inside
			pnew->previous = pitem->previous;
			pnew->next = pitem;
			((LISTITEM *)pitem->previous)->next = pnew;
			pitem->previous = pnew;
		}
	}
	else {
		if (plist->size) {
			//insert as last
			pnew->next = NULL;
			pnew->previous = plist->last;
			plist->last->next = pnew;
			plist->last = pnew;
		}
		else {
			//insert the 1st item
			pnew->previous = NULL;
			pnew->next = NULL;
			plist->first = pnew;
			plist->last = pnew;
		}
	}
	plist->size++;

	LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS

	return(TRUE);
}

//*** function appends new item to the LIST
BOOL LIST_AppendItem(HLIST hList, LPARAM lparam)
{
	return(LIST_InsertItem(hList, -1, lparam));
}

//*** function prepends (inserts in front of) new item to the LIST
BOOL LIST_PrependItem(HLIST hList, LPARAM lparam)
{
	return(LIST_InsertItem(hList, 0, lparam));
}


//*** function removes item selected by index from the LIST
BOOL LIST_RemoveItem(HLIST hList, DWORD idx)
{
	LISTBODY *plist;
	LISTITEM *pitem;

	if (hList == NULL)
		return(FALSE);
	plist = (LISTBODY *)hList;

	EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

	//test list size
	if (plist->size) {
		//test a correct index
		if (idx >= plist->size)
			idx = plist->size - 1;

		//search for item
		pitem = LIST_SearchItemByIndex(hList, idx);

		//test item
		if (pitem) {
			//iproc
			if (plist->iproc)
				plist->iproc(&pitem->lparam, IPTYPE_DELETE, 0);

			//removing
			if (plist->size == 1) {
				plist->first = NULL;
				plist->last = NULL;

			}
			else if (pitem == plist->first) {
				((LISTITEM *)pitem->next)->previous = NULL;
				plist->first = pitem->next;
			}
			else if (pitem == plist->last) {
				((LISTITEM *)pitem->previous)->next = NULL;
				plist->last = pitem->previous;
			}
			else {
				//edit previous & next
				((LISTITEM *)pitem->previous)->next = pitem->next;
				((LISTITEM *)pitem->next)->previous = pitem->previous;
			}
			//free item
			free((void *)pitem);
			plist->size--;

			LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
			return(TRUE);		//done
		}
	}

	LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
	return(FALSE);		//notning to remove
}

//*** function removes item selected by item handle
BOOL LIST_RemoveItemByHandle(HLIST hList, HITEM hItem)
{
	LISTBODY *plist;
	LISTITEM *pitem;

	if (hList == NULL || hItem == NULL)
		return(FALSE);
	plist = (LISTBODY *)hList;
	pitem = (LISTITEM *)hItem;

	EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

	//test list size
	if (plist->size) {
		//iproc
		if (plist->iproc)
			plist->iproc(&pitem->lparam, IPTYPE_DELETE, 0);
		//removing
		if (plist->size == 1) {
			plist->first = NULL;
			plist->last = NULL;
		}
		else if (pitem == plist->first) {
			((LISTITEM *)pitem->next)->previous = NULL;
			plist->first = pitem->next;
		}
		else if (pitem == plist->last) {
			((LISTITEM *)pitem->previous)->next = NULL;
			plist->last = pitem->previous;
		}
		else {
			//edit previous & next
			((LISTITEM *)pitem->previous)->next = pitem->next;
			((LISTITEM *)pitem->next)->previous = pitem->previous;
		}
		//free item
		free((void *)pitem);
		plist->size--;
    LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
		return(TRUE);		//done
	}

	LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
	return(FALSE);		//notning to remove
}

//*** function removes the first item from the LIST
BOOL LIST_RemoveFirstItem(HLIST hList)
{
	return(LIST_RemoveItem(hList, 0));
}

//*** function removes the last item from the LIST
BOOL LIST_RemoveLastItem(HLIST hList)
{
	return(LIST_RemoveItem(hList, (DWORD)-1));
}

//*** function removes all items from the LIST
BOOL LIST_RemoveAll(HLIST hList)
{
	LISTBODY *plist;
	LISTITEM *pitem;

	if (hList == NULL)
		return(FALSE);		//invalid handle
	plist = (LISTBODY *)hList;

	EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

	//test for iproc function
	if (plist->iproc) {
		//go through list -> process user's idelete, free items
		pitem = (LISTITEM *)plist->first;
		while (pitem) {
			plist->first = (LISTITEM *)pitem->next;
			//process user's iproc function
			plist->iproc(&pitem->lparam, IPTYPE_DELETE, 0);
			free((void *)pitem);
			pitem = plist->first;
		}
	}
	else {
		//go through list -> free items only
		pitem = (LISTITEM *)plist->first;
		while (pitem) {
			plist->first = (LISTITEM *)pitem->next;
			free((void *)pitem);
			pitem = plist->first;
		}
	}
	plist->size = 0;
	plist->first = NULL;
	plist->last = NULL;

	LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS

	return(TRUE);		//done
}

//----------------------------------------------------------------
//*** function returns list size
//returns: (DWORD)-1 if error
DWORD LIST_GetSize(HLIST hList)
{
	if (hList == NULL)
		return((DWORD)0);
	return(((LISTBODY *)hList)->size);
}

//----------------------------------------------------------------
//*** function gets item handle by LISTPOS
//hObj=HLIST: first, last, by index
//hObj=HITEM: previous, next
HITEM LIST_GetItem(HANDLE hObj, LISTPOS pos, DWORD idx)
{

	if (hObj) {
		switch (pos) {
			case LPOS_FIRST:
				return(((LISTBODY *)hObj)->first);
			case LPOS_LAST:
				return(((LISTBODY *)hObj)->last);
			case LPOS_INDEX:
				return(LIST_SearchItemByIndex(hObj, idx));
			case LPOS_NEXT:
				return(((LISTITEM *)hObj)->next);
			case LPOS_PREV:
				return(((LISTITEM *)hObj)->previous);
		}
	}
	return(NULL);
}

//*** function gets item value
//note: none iproc function is processed, take care!
BOOL LIST_GetItemValue(HITEM hItem, LPARAM *lparam)
{
	if (hItem == NULL || lparam == NULL)
		return(FALSE);

	*lparam = ((LISTITEM *)hItem)->lparam;
	return(TRUE);
}

//*** function sets item value
//note: IPTYPE_DELETE/IPTYPE_CREATE iproc functions are processed if hList != NULL, take care!
BOOL LIST_SetItemValue(HLIST hList, HITEM hItem, LPARAM lparam)
{
	LISTBODY *plist;

	if (hItem == NULL)
		return(FALSE);
	plist = (LISTBODY *)hList;

	//set item LPARAM
	if (plist && plist->iproc) {
		EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

		//by iproc IPTYPE_DELETE
		plist->iproc(&((LISTITEM *)hItem)->lparam, IPTYPE_DELETE, 0);
		//by iproc IPTYPE_CREATE
		plist->iproc(&((LISTITEM *)hItem)->lparam, IPTYPE_CREATE, lparam);

		LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
	}
	else {
		//only copy LPARAMs
		((LISTITEM *)hItem)->lparam = lparam;
	}
	return(TRUE);
}

//----------------------------------------------------------------
//*** function copies source list to destionation one (destination list will be removed)
//notes: lists should be the same type (with the same IProc)
DWORD LIST_Copy(HLIST des_hList, HLIST src_hList)
{
	DWORD n;
	HITEM hitem;
	LPARAM val;

	if (des_hList == NULL || des_hList == NULL)
		return(ERROR_INVALID_HANDLE);

	//remove all items
	LIST_RemoveAll(des_hList);
	//get size of source
	n = LIST_GetSize(src_hList);
	if (n) {
		//get first from source
		hitem = LIST_GetItem(src_hList, LPOS_FIRST, 0);
		while (hitem && LIST_GetItemValue(hitem, &val)) {
			//append value
			LIST_AppendItem(des_hList, val);
			//next item
			hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
		}
	}
	return(NO_ERROR);
}

//----------------------------------------------------------------
//*** function compares two list (define size of data)
//notes: compre function should return zero for the same values else non-zero
//returns: 0=the same, 1=different
BOOL LIST_Compare(HLIST hList1, HLIST hList2, int (*compar)(LPARAM, LPARAM))
{
	DWORD n;
	HITEM hi1, hi2;
	LPARAM val1, val2;

	if (hList1 == NULL || hList2 == NULL) {
		if (hList1 == hList2)
			return(0);		//both null
		return(1);
	}
	//compare size
	n = LIST_GetSize(hList1);
	if (LIST_GetSize(hList2) != n)
		return(1);		//different

	//compare data
	if (compar) {
		//get first items
		hi1 = LIST_GetItem(hList1, LPOS_FIRST, 0);
		hi2 = LIST_GetItem(hList2, LPOS_FIRST, 0);
		//go through items
		while (hi1 && hi2) {
			//get values
			if (!LIST_GetItemValue(hi1, &val1) ||
					!LIST_GetItemValue(hi2, &val2) ||
					compar(val1, val2) != 0) {
				return(1);		//different
			}
			//next items
			hi1 = LIST_GetItem(hi1, LPOS_NEXT, 0);
			hi2 = LIST_GetItem(hi2, LPOS_NEXT, 0);
		}
	}
	return(0);		//are the same
}

//----------------------------------------------------------------
//*** function prints LIST
BOOL LIST_Printf(HLIST hList, FILE *f)
{
	LISTBODY *plist;
	LISTITEM *pitem;
	DWORD i;

	if (f == NULL)
		return(FALSE);

	if (hList == NULL) {
		fprintf(f, "LIST (NULL)\n");
	}
	else {
		plist = (LISTBODY *)hList;
		fprintf(f, "LIST (%lu)\n", plist->size);

		if (plist->size) {
			fprintf(f, "  |\n");

			EnterCriticalSection(&plist->cs);		//enter CS, other threads are locked out

			pitem = plist->first;
			i = 0;

			if (plist->iproc) {
				while (pitem) {
					fprintf(f, "  +- %lu: ", i);

					//print by iproc
					plist->iproc(&pitem->lparam, IPTYPE_PRINT, (LPARAM)f);

					fputc('\n', f);
					i++;
					pitem = pitem->next;
				}
			}
			else {
				while (pitem) {
					fprintf(f, "  +- %lu: %lu (%08X)\n", i, pitem->lparam, (unsigned)pitem);
					i++;
					pitem = pitem->next;
				}
			}

			LeaveCriticalSection(&plist->cs);		//leave CS, other thread can enter CS
		}
	}
	fputc('\n', f);
	return(TRUE);
}

//----------------------------------------------------------------
//*** function process defalu sorting by lparam value
BOOL LIST_IProcSortParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	if (type == IPTYPE_COMPARE && iparam) {
		if (*iparam > lparam)
			return(1);
		else if (*iparam < lparam)
			return(-1);
		else
			return(0);
	}
	return(0);
}

//*** function sorts LIST (bubble sort)
BOOL LIST_BubbleSort(HLIST hList, LIST_SORTMODE mode)
{
	ITEMPROC iproc;
	LISTITEM *pitem1, *pitem2;
	int done = 0, cmp;
	LPARAM lparam;

	if (hList == NULL)
		return(FALSE);
	iproc = ((LISTBODY *)hList)->iproc;
	if (iproc == NULL)
		iproc = LIST_IProcSortParam;		//default sort function

	while (!done) {
		done = 1;
		//through list
		pitem1 = LIST_GetItem(hList, LPOS_FIRST, 0);		//first
		pitem2 = LIST_GetItem(pitem1, LPOS_NEXT, 0);		//second
		while (pitem1 && pitem2) {
			//compare by proc. function
			cmp = iproc(&pitem1->lparam, IPTYPE_COMPARE, pitem2->lparam);

			//swap item values
			if ((mode == LIST_SORTMODE_ASCEND && cmp > 0) || (mode == LIST_SORTMODE_DESCEND && cmp < 0)) {
				lparam = pitem1->lparam;
				pitem1->lparam = pitem2->lparam;
				pitem2->lparam = lparam;
				done = 0;		//value changed
			}

			//next items
			pitem1 = pitem2;
			pitem2 = LIST_GetItem(pitem1, LPOS_NEXT, 0);
		}
	}
	return(TRUE);
}


//----------------------------------------------------------------
//*** function processes IPROC function for working with strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
//IPTYPE_COMPARE: iparam -> pointer to lparam of item, lparam of other item -> LPARAM
BOOL LIST_IProcATextParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					*iparam = (LPARAM)malloc((strlen((char *)lparam)+1)*sizeof(char));
					if (*iparam == 0)
						return(FALSE);
					strcpy((char *)*iparam, (char *)lparam);
				}
				else
					*iparam = 0;
				break;
			//--- processed before ITEM deletion
			case IPTYPE_DELETE:
				if (*iparam)
					free((void *)*iparam);
				*iparam = 0;
				break;
			//--- processed at ITEM print
			case IPTYPE_PRINT:
				if (lparam) {
					if (*iparam)
						fprintf((FILE *)lparam, "\"%s\"", (char *)*iparam);
					else
						fprintf((FILE *)lparam, "(null)");
				}
				break;
			//--- processed comparing items
			case IPTYPE_COMPARE:
				if (*iparam && lparam)
					return(strcmp((char *)*iparam, (char *)lparam));
				return(0);
		}
	}
	return(TRUE);
}

//----------------------------------------------------------------
//*** function processes IPROC function for working with TCHAR strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
//IPTYPE_COMPARE: iparam -> pointer to lparam of item, lparam of other item -> LPARAM
BOOL LIST_IProcTextParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					*iparam = (LPARAM)malloc((lstrlen((TCHAR *)lparam)+1)*sizeof(TCHAR));
					if (*iparam == 0)
						return(FALSE);
					lstrcpy((TCHAR *)*iparam, (TCHAR *)lparam);
				}
				else
					*iparam = 0;
				break;
			//--- processed before ITEM deletion
			case IPTYPE_DELETE:
				if (*iparam)
					free((void *)*iparam);
				*iparam = 0;
				break;
			//--- processed at ITEM print
			case IPTYPE_PRINT:
				if (lparam) {
					if (*iparam)
#ifdef UNICODE
						fprintf((FILE *)lparam, "\"%ls\"", (TCHAR *)*iparam);
#else
						fprintf((FILE *)lparam, "\"%s\"", (TCHAR *)*iparam);
#endif
					else
						fprintf((FILE *)lparam, "(null)");
				}
				break;
			//--- processed comparing items
			case IPTYPE_COMPARE:
				if (*iparam && lparam)
					return(lstrcmp((TCHAR *)*iparam, (TCHAR *)lparam));
				return(0);
		}
	}
	return(TRUE);
}

//----------------------------------------------------------------
//*** function processes IPROC function for working with TCHAR strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
//IPTYPE_COMPARE: iparam -> pointer to lparam of item, lparam of other item -> LPARAM
BOOL LIST_IProcMemoryParam(unsigned int size, LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					*iparam = (LPARAM)malloc(size);
					if (*iparam == 0)
						return(FALSE);
					memcpy((TCHAR *)*iparam, (TCHAR *)lparam, size);
				}
				else
					*iparam = 0;
				break;
			//--- processed before ITEM deletion
			case IPTYPE_DELETE:
				if (*iparam)
					free((void *)*iparam);
				*iparam = 0;
				break;
			//--- processed at ITEM print
			case IPTYPE_PRINT:
				if (lparam) {
					if (*iparam) {
						fprintf((FILE *)lparam, "\"");
						unsigned char *buf = (unsigned char*)*iparam;
						int i;
						for (i=0; i<size; i++) {
							fprintf((FILE *)lparam, "%02X", buf[i]);
						}
						fprintf((FILE *)lparam, "\"");
					}
					else
						fprintf((FILE *)lparam, "(null)");
				}
				break;
			//--- processed comparing items
			case IPTYPE_COMPARE:
				if (*iparam && lparam)
					return(memcmp((TCHAR *)*iparam, (TCHAR *)lparam, size));
				return(0);
		}
	}
	return(TRUE);
}

//--------------------------------------------------------------------
//*** test function
BOOL LIST__Test(void)
{
	HLIST hlist;
	HITEM hitem;
	BOOL ret;
	FILE *fw;
	char *pstr;

	fw = fopen("list.log", "w");
	hlist = LIST_Create(LIST_IProcATextParam);

	ret = LIST_InsertItem(hlist, 0, (LPARAM)"item 1");
	ret = LIST_InsertItem(hlist, 0, (LPARAM)"item 2");
	ret = LIST_InsertItem(hlist, 0, (LPARAM)"item 3");
	ret = LIST_InsertItem(hlist, -1, (LPARAM)"");
	ret = LIST_InsertItem(hlist, 4, (LPARAM)0);
	ret = LIST_InsertItem(hlist, 4, (LPARAM)0);
	ret = LIST_AppendItem(hlist, (LPARAM)"item 7");
	ret = LIST_AppendItem(hlist, (LPARAM)"item 8");
	LIST_Printf(hlist, fw);

	hitem = LIST_GetItem(hlist, LPOS_INDEX, 2);
	ret = LIST_GetItemValue(hitem, (void *)&pstr);
	ret = LIST_SetItemValue(hlist, hitem, (LPARAM)"change of item 2");

	ret = LIST_RemoveItem(hlist, 5);
	LIST_Printf(hlist, fw);

	while ((ret = LIST_RemoveItem(hlist, -1)))
		;
	LIST_Printf(hlist, fw);

	LIST_Discard(hlist);
	fclose(fw);

	return(ret);
}

