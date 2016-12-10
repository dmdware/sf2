

#include "fibheap.h"
#include "../path/pathnode.h"


// byte-align structures
#pragma pack(push, 1)

//todo make generic

#if 0
void FibHeap::check(const char* comment, FibNode* x)
{
	//return;
	//FibNode *x = minn;

	static int total;

	if(x == minn)
		total = 0;

	int siblings = 0;

	if(!x)
		return;

	siblings++;

	FibNode *cur = x;
	while(ectrue)
	{
		/*std::cerr << "cur: " << cur << std::endl;
		std::cerr << "x: " << x << std::endl;*/
		if (cur->left && cur->left != x)
		{
			siblings++;
			//std::cerr << "cur left: " << cur->left << std::endl;
			FibNode *tmp = cur;
			cur = cur->left;
			//if (tmp->child)
			{
				//find(tmp->child);

				check(comment, tmp->child);
			}
			//delete tmp;
		}
		else
		{
			//if (cur->child)
			{
				//delete_fibnodes(cur->child);

				//FibNode* r = find(payload, cur->child);
			}
			//delete cur;

			check(comment, cur->child);

			break;
		}
	}

	if(x->p)
	{
		if(x->p->degree != siblings)
		{
			char m[123];
			sprintf(m, "%s: x->p->degree%d siblings%d", comment, x->p->degree, siblings);
			InfoMess(m,m);
		}
	}

	total+=siblings;

	if(x==minn)
	{
		if(total != n)
		{
			char m[123];
			sprintf(m, "%s: total%d n%d", comment, total, n);
			InfoMess(m,m);
		}
	}
}
#endif

//FibHeap_freenode
void FibHeap::delete_fibnodes(FibNode *x)
{
	if (!x)
		return;

	FibNode *cur = x;
	while(ectrue)
	{
		/*std::cerr << "cur: " << cur << std::endl;
		std::cerr << "x: " << x << std::endl;*/
		if (cur->left && cur->left != x)
		{
			//std::cerr << "cur left: " << cur->left << std::endl;
			FibNode *tmp = cur;
			cur = cur->left;

			
    std::map<void*, FibNode*>::iterator mit
      = find(cur->payload);
    fstore.erase(mit);

			//if (tmp->child)
			delete_fibnodes(tmp->child);
			delete tmp;
		}
		else
		{
			
    std::map<void*, FibNode*>::iterator mit
      = find(cur->payload);
    fstore.erase(mit);

			//if (cur->child)
			delete_fibnodes(cur->child);
			delete cur;
			break;
		}
	}
}

void FibHeap::heapify(void* payload)
{
	//FibNode* x = find(payload, minn);
	FibNode* x = findNode(payload);

	//check("heapify beg", minn);

	if(x)
	{
		//decrease_key(x, x->key);
		//decrease_key(x, ((PathNode*)payload)->cost);
		decrease_key(x);

		//check("heapify end1", minn);
		return;
	}

#if 0
	InfoMess("!f","!f");

	if(!payload)
	{
		InfoMess("!fp","!fp");
	}

	PathNode* n = (PathNode*)payload;

	char m[123];
	sprintf(m, "n cost%d", n->cost);
	InfoMess(m,m);
#endif

	add(payload);


		//check("heapify end2", minn);

	//InfoMess("a.","a.");
}

void* FibHeap::delmin()
{
	//Log("delmin beg");

	//check("delmin beg", minn);

	//Log("n=%d", n);
	//Log("minn=%d", (int)minn);

	FibNode* z = extract_min();

	//Log("z=%d", (int)z);

	//if(!z)
	//	InfoMess("!z","!z");
	//decrease_key(x,std::numeric_limits<T>::min());
	//decrease_key(z,std::numeric_limits<T>::min());
	void* pay = z->payload;

    std::map<void*, FibNode*>::iterator mit
      = find(z->payload);
    fstore.erase(mit);
	
	//Log("pay=%d", (int)pay);

	
	//Log("cost=%u age=%u", (int)HEAPKEY(pay)->cost, (int)HEAPKEY(pay)->age);

	//if(!pay)
	//	InfoMess("!pay","!pay");

	delete z;



	//check("delmin end", minn);
	
	//Log("delmin end");
	return pay;
}

FibNode* FibHeap::extract_min()
{
	FibNode *z, *x, *next;
	FibNode ** childList;


	//check("extract_mind beg", minn);

	// 1
	z = minn;
	// 2
	if ( z != NULL )
	{
		// 3
		x = z->child;
		if ( x != NULL )
		{
			childList = new FibNode*[z->degree];
			next = x;
			for ( int i = 0; i < (int)z->degree; i++ )
			{
				childList[i] = next;
				next = next->right;
			}
			for ( int i = 0; i < (int)z->degree; i++ )
			{
				x = childList[i];
				// 4
				minn->left->right = x;
				x->left = minn->left;
				minn->left = x;
				x->right = minn;
				// 5
				x->p = NULL;
			}
			delete [] childList;
		}
		// 6
		z->left->right = z->right;
		z->right->left = z->left;
		// 7
		if ( z == z->right )
		{
			// 8
			minn = NULL;
		}
		else
		{
			// 9
			minn = z->right;
			// 10
			consolidate();
		}
		// 11
		--n;
	}


	//check("extract_mind end", minn);
	// 12
	return z;
}

FibNode* FibHeap::add(void* pl)
{
	FibNode *x = new FibNode(pl);

	//check("add beg", minn);

	// 1
	x->degree = 0;
	// 2
	x->p = NULL;
	// 3
	x->child = NULL;
	// 4
	x->mark = ecfalse;
	// 5
	if ( minn == NULL)
	{
		// 6, 7
		minn = x->left = x->right = x;
	}
	else
	{
		// 8
		minn->left->right = x;
		x->left = minn->left;
		minn->left = x;
		x->right = minn;
		// 9

		HeapKey* xpay = HEAPKEY(x->payload);
		HeapKey* mpay = HEAPKEY(minn->payload);

		//if ( x->key < minn->key )
		//if ( x->key <= minn->key )
		//if( HEAPCOMPARE( mpay, xpay ) )
		if( !HEAPCOMPARE( xpay, mpay ) )
		{
			// 10
			minn = x;
		}
	}
	// 11
	++n;
	
    fstore.insert(std::pair<void*, FibNode*>(pl,x));

	return x;
	//check("add end", minn);
}


FibNode* FibHeap::find(void* payload, FibNode* x)
{
	if(!x)
		return NULL;

	if(x->payload == payload)
		return x;

	FibNode *cur = x;
	while(ectrue)
	{
		/*std::cerr << "cur: " << cur << std::endl;
		std::cerr << "x: " << x << std::endl;*/
		if (cur->left && cur->left != x)
		{
			//std::cerr << "cur left: " << cur->left << std::endl;
			FibNode *tmp = cur;
			cur = cur->left;
			//if (tmp->child)
			{
				//find(tmp->child);

				FibNode* r = find(payload, tmp->child);

				if(r)
					return r;
			}
			//delete tmp;
		}
		else
		{
			//if (cur->child)
			{
				//delete_fibnodes(cur->child);

				FibNode* r = find(payload, cur->child);

				if(r)
					return r;
			}
			//delete cur;
			break;
		}
	}

	return NULL;
}


/*
* fib_heap_link(y,x)
* 1. remove y from the root list of heap
* 2. make y a child of x, incrementing x.degree
* 3. y.mark = FALSE
*/
void FibHeap::fib_heap_link( FibNode* y, FibNode* x )
{

	//check("fib_heap_link beg", minn);

	// 1
	y->left->right = y->right;
	y->right->left = y->left;
	// 2
	if ( x->child != NULL )
	{
		x->child->left->right = y;
		y->left = x->child->left;
		x->child->left = y;
		y->right = x->child;
	}
	else
	{
		x->child = y;
		y->right = y;
		y->left = y;
	}
	y->p = x;
	x->degree++;
	// 3
	y->mark = ecfalse;

	//check("fib_heap_link end", minn);
}

void FibHeap::consolidate()
{
	FibNode* w, * next, * x, * y, * temp;
	FibNode** A, ** rootList;
	// Max degree <= log base golden ratio of n
	int d, rootSize;
	//todo use something deterministic
	//int max_degree = static_cast<int>(floor(log(static_cast<double>(n))/log(static_cast<double>(1 + sqrt(static_cast<double>(5)))/2)));
	int max_degree = ilog2floor(n)/ilog2ceil(1 + (isqrt(5)>>1));
	//const int max_degree = ilog2floor(n);
	//const int max_degree = ilog2ceil(n+1)<<1;

	//check("consolidate beg", minn);

	// 1
	A = new FibNode*[max_degree+2]; // plus two both for indexing to max degree and so A[max_degree+1] == NIL
	// 2, 3
	std::fill_n(A, max_degree+2, (FibNode*)NULL);
	// 4
	w = minn;
	rootSize = 0;
	next = w;
	do
	{
		rootSize++;
		next = next->right;

		//if(!next)
		//	InfoMess("!next","!next");
	} while ( next != w );
	rootList = new FibNode*[rootSize];
	for ( int i = 0; i < rootSize; i++ )
	{
		rootList[i] = next;
		next = next->right;
	}
	for ( int i = 0; i < rootSize; i++ )
	{
		w = rootList[i];
		// 5
		x = w;
		// 6
		d = x->degree;

#if 0
			if(d > max_degree+2)
			{
				char m[123];
				sprintf(m, "maxd%d d%d !NULL=%d n=%d xdeg%d rs%d c%d", max_degree, d, (int)(A[d]), n, x->degree, rootSize, (int)x->child);
				InfoMess(m,m);
			}
#endif

		// 7
		while ( A[d] != NULL )
		{
#if 0
			if(d >= max_degree+1)
			{
				char m[123];
				sprintf(m, "maxd%d d%d !NULL=%d n=%d xdeg%d rs%d c%d", max_degree, d, (int)(A[d]), n, x->degree, rootSize, (int)x->child);
				InfoMess(m,m);
			}
#endif
			// 8
			y = A[d];
			// 9

			HeapKey* xpay = HEAPKEY(x->payload);
			HeapKey* ypay = HEAPKEY(y->payload);

			//if ( x->key > y->key )
			//if ( x->key >= y->key )
			//if( HEAPCOMPARE( xpay, ypay ) )
			if( !HEAPCOMPARE( ypay, xpay ) )
			{
				// 10
				//todo less vars?
				temp = x;
				x = y;
				y = temp;
			}
			// 11
			fib_heap_link(y,x);
			// 12
			A[d] = NULL;
			// 13
			d++;
		}
		// 14
		A[d] = x;
	}
	delete [] rootList;
	// 15
	minn = NULL;
	// 16
	for ( int i = 0; i < max_degree+2; i++ )
	{
		// 17
		if ( A[i] != NULL )
		{
			// 18
			if ( minn == NULL )
			{
				// 19, 20
				minn = A[i]->left = A[i]->right = A[i];
			}
			else
			{
				// 21
				minn->left->right = A[i];
				A[i]->left = minn->left;
				minn->left = A[i];
				A[i]->right = minn;
				// 22

				HeapKey* apay = HEAPKEY(A[i]->payload);
				HeapKey* mpay = HEAPKEY(minn->payload);

				//if ( A[i]->key < minn->key )
				//if ( A[i]->key <= minn->key )
				//if( HEAPCOMPARE( mpay, apay ) )
				if( !HEAPCOMPARE( apay, mpay ) )
				{
					// 23
					minn = A[i];
				}
			}
		}
	}
	delete [] A;


	//check("consolidate end", minn);
}


/*
* cut(x,y)
* 1. remove x from the child list of y, decrementing y.degree
* 2. add x to the root list of H
* 3. x.p = NIL
* 4. x.mark = FALSE
*/
void FibHeap::cut( FibNode* x, FibNode* y )
{

	//check("cut beg", minn);

	// 1
	if ( x->right == x )
	{
		y->child = NULL;
	}
	else
	{
		x->right->left = x->left;
		x->left->right = x->right;
		if ( y->child == x )
		{
			y->child = x->right;
		}
	}
	y->degree--;
	// 2
	minn->right->left = x;
	x->right = minn->right;
	minn->right = x;
	x->left = minn;
	// 3
	x->p = NULL;
	// 4
	x->mark = ecfalse;


	//check("cut end", minn);
}

/*
  * cascading_cut(y)
  * 1. z = y.p
  * 2. if z != NIL
  * 3. 	if y.mark == FALSE
  * 4. 		y.mark = TRUE
  * 5. 	else CUT(H,y,z)
  * 6. 		CASCADING-CUT(H,z)
  */
void FibHeap::cascading_cut( FibNode* y )
{
	FibNode* z;


	//check("cascading_cut beg", minn);

	// 1
	z = y->p;
	// 2
	if ( z != NULL )
	{
		// 3
		if ( y->mark == ecfalse )
		{
			// 4
			y->mark = ectrue;
		}
		else
		{
			// 5
			cut(y,z);
			// 6
			cascading_cut(z);
		}
	}


	//check("cascading_cut end", minn);
}

ecbool FibHeap::hasmore()
{
	//return (ecbool)n;
	return n > 0;
}

  /*
   * decrease_key(x,k)
   * 1. if k > x.key
   * 2. 	error "new key is greater than current key"
   * 3. x.key = k
   * 4. y = x.p
   * 5. if y != NIL and x.key < y.key
   * 6. 	CUT(H,x,y)
   * 7. 	CASCADING-CUT(H,y)
   * 8. if x.key < H.min.key
   * 9. 	H.min = x
   */
void FibHeap::decrease_key( FibNode* x )
{
#if 0
    std::unordered_map<void*, FibNode*>::iterator mit
      = find(x->payload);
    fstore.erase(mit);
    fstore.insert(std::pair<void*, FibNode*>(x->payload,x));
#endif

	FibNode* y;

	//check("decrease_key beg", minn);

	// 1
#if 0
	if ( k > x->key )
	{
		// 2
		// error( "new key is greater than current key" );
		//return;
	}
	// 3
	x->key = k;
#endif
	// 4
	y = x->p;
	// 5
	HeapKey* xpay = HEAPKEY(x->payload);

	if(y)
	{
		HeapKey* ypay = HEAPKEY(y->payload);

		//if ( y != NULL && x->key < y->key )
		//if ( y != NULL && x->key <= y->key )
		//if( HEAPCOMPARE( ypay, xpay ) )
		//if( !HEAPCOMPARE( xpay, ypay ) )
		{
			//InfoMess("x->key < y->key", "x->key < y->key");

			// 6
			cut(x,y);
			
			//check("decrease_key 1", minn);

			// 7
			cascading_cut(y);
		}
	}
	// 8

	
	//check("decrease_key 2", minn);

#if 0
	if(x->p)
	{
		InfoMess("x->p==ectrue", "x->p==ectrue");
	}
#endif 

	//xpay = x->payload;
	xpay = HEAPKEY(x->payload);
	HeapKey* mpay = HEAPKEY(minn->payload);

	//if ( x->key < minn->key )
	//if ( x->key <= minn->key )
	//if( HEAPCOMPARE( mpay, xpay ) )
	if( !HEAPCOMPARE( xpay, mpay ) )
	{
		// 9
		minn = x;
	}


#if 0
	if(minn->p)
	{
		InfoMess("minn->p==ectrue", "minn->p==ectrue");
	}
#endif

	//check("decrease_key end", minn);
}

//todo #define HEAPCOMPARE(a,b) macro
//todo #define HEAPKEY(a) macro

#pragma pack(pop)

