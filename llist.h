/* libuev - (Doubly) Linked lists anchored at both ends
 *
 * Copyright (c) 2012  Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013  Joachim Nilsson <troglobit()gmail!com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Struct mix-in. Use this inside your struct declaration */
#define LListField(tn) tn * _llNext; tn * _llPrev

/* Anchor Declaration. Use like: LList(typename) varname; */
#define LList(tn) struct { tn *head; tn *tail; int numElm; }

/* List init */
#define lListInit(a) ((a)->head = (a)->tail = NULL, (a)->numElm = 0, (a))

/* List empty */
#define lListEmpty(a) ((a)->head == NULL)

/* List head */
#define lListHead(a) ((a)->head)

/* List tail */
#define lListTail(a) ((a)->tail)

/* List length */
#define lListLength(a) ((a)->numElm)

/* List element next */
#define lListNext(e) ((e)->_llNext)

/* List element prev */
#define lListPrev(e) ((e)->_llPrev)

/* Append element e to list e */
#define lListAppend(a, e)				\
	{						\
		if (lListEmpty(a))			\
			(a)->head = (e);		\
		else					\
			(a)->tail->_llNext = (e);	\
		(e)->_llPrev = (a)->tail;		\
		(a)->tail = (e);			\
		(e)->_llNext = NULL;			\
		(a)->numElm++;				\
	}

/* Insert element n before element e in list a */
#define lListInsertFirst(a, n) lListInsert(a, n, (a)->head)

/* Insert element n before element e in list a */
#define lListInsert(a, n, e)					\
	{							\
		if (!(e) || (a)->head == (e)) {			\
			n->_llNext = (a)->head;			\
			n->_llPrev = NULL;			\
			(a)->head = (n);			\
		} else {					\
			n->_llNext = (e);			\
			n->_llPrev = (e)->_llPrev;		\
			(e)->_llPrev->_llNext = n;		\
		}						\
		if (n->_llNext) n->_llNext->_llPrev = n;	\
		(a)->numElm++;					\
	}

/* Remove element e from list a. Caller must free element. */
#define lListRemove(a, e)						\
	{								\
		if ((a)->head) {					\
			if ((e) == (a)->head)				\
				(a)->head = (e)->_llNext;		\
			else						\
				(e)->_llPrev->_llNext = (e)->_llNext;	\
			if ((e) == (a)->tail)				\
				(a)->tail = (e)->_llPrev;		\
			else						\
				(e)->_llNext->_llPrev = (e)->_llPrev;	\
			(a)->numElm--;					\
		}							\
	}

/* Purge list a elements with free() */
#define lListPurgeT(a, T)				\
	{						\
		T p1, p2;				\
		for (p1 = (a)->head; p1; ) {		\
			p2 = p1;			\
			p1 = p1->_llNext; free(p2);	\
		}					\
		lListInit(a);				\
	}

/* Remove last element from list a */
#define lListRemoveTailT(a, T)					\
	{							\
		T p;						\
		if ((a)->tail) {				\
			p = (a)->tail;				\
			(a)->tail = (a)->tail->_llPrev;		\
			if ((a)->tail)				\
				(a)->tail->_llNext = NULL;	\
			else					\
				(a)->head = NULL;		\
			free(p);				\
			(a)->numElm--;				\
		}						\
	}

/* Remove first element from list a */
#define lListRemoveHeadT(a, T)					\
	{							\
		T p;						\
		if ((a)->head) {				\
			p = (a)->head;				\
			(a)->head = (a)->head->_llNext;		\
			if ((a)->head)				\
				(a)->head->_llPrev = NULL;	\
			else					\
				(a)->tail = NULL;		\
			free(p);				\
			(a)->numElm--;				\
		}						\
	}

/* Convenience
 * Only use if you have typeof() eg. gcc */
#define lListPurge(a) lListPurgeT(a, typeof((a)->head))
#define lListRemoveTail(a) lListRemoveTailT(a, typeof((a)->head))
#define lListRemoveHead(a) lListRemoveHeadT(a, typeof((a)->head))

/* Traverse the list foreach e in a */
#define lListForeachIn(e, a) for (e = (a)->head; e; e = e->_llNext)

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
