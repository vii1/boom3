//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   MapItem.h - class MapItem interface                             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _BOOM_MAPITEM_H
#define _BOOM_MAPITEM_H

#include "types.h"

// if EDITOR is defined, the engine uses linked lists instead of arrays
// lists are slower than the arrays, but allow inserting and deleting the elements

#ifndef EDITOR
// creates an array with __size elements of __type
// loads each of them from the current file
// returns the address in __base
#define CONSTRUCT_AND_LOAD_ARRAY( __base, __type, __size )      \
{                                                               \
  __base = new __type[__size];                                  \
  if (!__base) return false; /* not enough memory */            \
  __type *__p;                                                  \
  int __i;                                                      \
  for ( __i=0,__p=__base; __i<__size; __i++,__p++ )             \
    if (!__p->load()) {                                         \
      for (;__p>=__base;__p--)                                  \
        __p->unload();                                          \
      delete __base;                                            \
      __base=NULL;                                              \
      return false;                                             \
    }                                                           \
}

// calls the destructor for each element of the array in __base and frees the memory
#define DESTRUCT_ARRAY( __base )                                \
{                                                               \
  delete[] __base;                                              \
  __base=NULL;                                                  \
}

#else
// creates a linked list with __size elements of __type
// loads each of them from the current file
// returns the address of the first in __base
#define CONSTRUCT_AND_LOAD_ARRAY( __base, __type, __size )      \
{                                                               \
  __type **l=&__base;                                           \
  for (int i=0;i<__size;i++) {                                  \
    *l=new __type();                                            \
    if (!*l) return false; /* not enough memory */              \
    (*l)->next=NULL;                                            \
    if (!(*l)->load()) {                                        \
      __type *q,*n;                                             \
      for (q=__base;q;q=n) {                                    \
        n=q->next;                                              \
        delete q;                                               \
      }                                                         \
      __base=NULL;                                              \
      return false;                                             \
    }                                                           \
    l=&(*l)->next;                                              \
  }                                                             \
}

// calls the destructor for each element of the linked list in __base and frees the memory
#define DESTRUCT_ARRAY( __base )                                \
{                                                               \
  void *n;                                                      \
  for (;__base;*(void **)&__base=n) {                           \
    n=__base->next;                                             \
    delete __base;                                              \
  }                                                             \
  __base=NULL;                                                  \
}
#endif

namespace boom {

	// MapItem is the root of the class hierarchy
	class MapItem
	{
	public:
		static const int kOptionDirty = 0x80; //The item has changed and should be saved

		byte options = 0;

		MapItem() = default;

		// saves the map item to the current file
		void save();
		// loads the map item from the current file
		bool load();

		//marks the map item as 'dirty'
		void dirty();
	};
} // namespace
#endif // _BOOM_MAPITEM_H_
