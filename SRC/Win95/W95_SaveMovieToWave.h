//-----------------------------------------------------------------------------
// File: WaveSink.h
// Description: Archive sink for creating .wav files.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mferror.h>

#include <assert.h>

// utility functions

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]) )
#endif

#ifndef SAFE_RELEASE
template <class T>
inline void SAFE_RELEASE(T*& p) {
	if (p) {
		p->Release();
		p = NULL;
	}
}
#endif
#ifndef CheckPointer
#define CheckPointer(x, hr) if (x == NULL) { return hr; }
#endif

//using namespace MediaFoundationSamples;

// common.h BEGIN

//////////////////////////////////////////////////////////////////////////
//  CritSec
//  Description: Wraps a critical section.
//////////////////////////////////////////////////////////////////////////

class CritSec {
private:
	CRITICAL_SECTION m_criticalSection;
public:
	CritSec() {
		InitializeCriticalSection(&m_criticalSection);
	}

	~CritSec() {
		DeleteCriticalSection(&m_criticalSection);
	}

	void Lock() {
		EnterCriticalSection(&m_criticalSection);
	}

	void Unlock() {
		LeaveCriticalSection(&m_criticalSection);
	}
};


//////////////////////////////////////////////////////////////////////////
//  AsyncCallback [template]
//
//  Description: 
//  Helper class that routes IMFAsyncCallback::Invoke calls to a class
//  method on the parent class.
//
//  Usage:
//  Add this class as a member variable. In the parent class constructor,
//  initialize the AsyncCallback class like this:
//      m_cb(this, &CYourClass::OnInvoke)
//  where
//      m_cb       = AsyncCallback object
//      CYourClass = parent class
//      OnInvoke   = Method in the parent class to receive Invoke calls.
//
//  The parent's OnInvoke method (you can name it anything you like) must
//  have a signature that matches the InvokeFn typedef below.
//////////////////////////////////////////////////////////////////////////

// T: Type of the parent object
template<class T>
class AsyncCallback: public IMFAsyncCallback {
public:
	typedef HRESULT(T::*InvokeFn)(IMFAsyncResult *pAsyncResult);

	AsyncCallback(T *pParent, InvokeFn fn): m_pParent(pParent), m_pInvokeFn(fn) {
	}

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv) {
		if (!ppv) {
			return E_POINTER;
		}
		if (iid == __uuidof(IUnknown)) {
			*ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
		} else if (iid == __uuidof(IMFAsyncCallback)) {
			*ppv = static_cast<IMFAsyncCallback*>(this);
		} else {
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
	STDMETHODIMP_(ULONG) AddRef() {
		// Delegate to parent class.
		return m_pParent->AddRef();
	}
	STDMETHODIMP_(ULONG) Release() {
		// Delegate to parent class.
		return m_pParent->Release();
	}


	// IMFAsyncCallback methods
	STDMETHODIMP GetParameters(DWORD*, DWORD*) {
		// Implementation of this method is optional.
		return E_NOTIMPL;
	}

	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) {
		return (m_pParent->*m_pInvokeFn)(pAsyncResult);
	}

	T *m_pParent;
	InvokeFn m_pInvokeFn;
};

// The List class template implements a simple double-linked list. 
// It uses STL's copy semantics. 

// There are two versions of the Clear() method:
//  Clear(void) clears the list w/out cleaning up the object.
//  Clear(FN fn) takes a functor object that releases the objects, if they need cleanup.

// The List class supports enumeration. Example of usage:
//
// List<T>::POSIITON pos = list.GetFrontPosition();
// while (pos != list.GetEndPosition())
// {
//     T item;
//     hr = list.GetItemPos(&item);
//     pos = list.Next(pos);
// }

// The ComPtrList class template derives from List<> and implements a list of COM pointers.

template <class T>
struct NoOp {
	void operator()(T& t) {
	}
};

template <class T>
class List {
protected:

	// Nodes in the linked list
	struct Node {
		Node *prev;
		Node *next;
		T    item;

		Node(): prev(NULL), next(NULL) {
		}

		Node(T item): prev(NULL), next(NULL) {
			this->item = item;
		}

		T Item() const { return item; }
	};

public:

	// Object for enumerating the list.
	class POSITION {
		friend class List<T>;

	public:
		POSITION(): pNode(NULL) {
		}

		bool operator==(const POSITION &p) const {
			return pNode == p.pNode;
		}

		bool operator!=(const POSITION &p) const {
			return pNode != p.pNode;
		}

	private:
		const Node *pNode;

		POSITION(Node *p): pNode(p) {
		}
	};

protected:
	Node    m_anchor;  // Anchor node for the linked list.
	DWORD   m_count;   // Number of items in the list.

	Node* Front() const {
		return m_anchor.next;
	}

	Node* Back() const {
		return m_anchor.prev;
	}

	virtual HRESULT InsertAfter(T item, Node *pBefore) {
		if (pBefore == NULL) {
			return E_POINTER;
		}

		Node *pNode = new Node(item);
		if (pNode == NULL) {
			return E_OUTOFMEMORY;
		}

		Node *pAfter = pBefore->next;

		pBefore->next = pNode;
		pAfter->prev = pNode;

		pNode->prev = pBefore;
		pNode->next = pAfter;

		m_count++;

		return S_OK;
	}

	virtual HRESULT GetItem(const Node *pNode, T* ppItem) {
		if (pNode == NULL || ppItem == NULL) {
			return E_POINTER;
		}

		*ppItem = pNode->item;
		return S_OK;
	}

	// RemoveItem:
	// Removes a node and optionally returns the item.
	// ppItem can be NULL.
	virtual HRESULT RemoveItem(Node *pNode, T *ppItem) {
		if (pNode == NULL) {
			return E_POINTER;
		}

		assert(pNode != &m_anchor); // We should never try to remove the anchor node.
		if (pNode == &m_anchor) {
			return E_INVALIDARG;
		}


		T item;

		// The next node's previous is this node's previous.
		pNode->next->prev = pNode->prev;

		// The previous node's next is this node's next.
		pNode->prev->next = pNode->next;

		item = pNode->item;
		delete pNode;

		m_count--;

		if (ppItem) {
			*ppItem = item;
		}

		return S_OK;
	}

public:

	List() {
		m_anchor.next = &m_anchor;
		m_anchor.prev = &m_anchor;

		m_count = 0;
	}

	virtual ~List() {
		Clear();
	}

	// Insertion functions
	HRESULT InsertBack(T item) {
		return InsertAfter(item, m_anchor.prev);
	}


	HRESULT InsertFront(T item) {
		return InsertAfter(item, &m_anchor);
	}

	// RemoveBack: Removes the tail of the list and returns the value.
	// ppItem can be NULL if you don't want the item back. (But the method does not release the item.)
	HRESULT RemoveBack(T *ppItem) {
		if (IsEmpty()) {
			return E_FAIL;
		} else {
			return RemoveItem(Back(), ppItem);
		}
	}

	// RemoveFront: Removes the head of the list and returns the value.
	// ppItem can be NULL if you don't want the item back. (But the method does not release the item.)
	HRESULT RemoveFront(T *ppItem) {
		if (IsEmpty()) {
			return E_FAIL;
		} else {
			return RemoveItem(Front(), ppItem);
		}
	}

	// GetBack: Gets the tail item.
	HRESULT GetBack(T *ppItem) {
		if (IsEmpty()) {
			return E_FAIL;
		} else {
			return GetItem(Back(), ppItem);
		}
	}

	// GetFront: Gets the front item.
	HRESULT GetFront(T *ppItem) {
		if (IsEmpty()) {
			return E_FAIL;
		} else {
			return GetItem(Front(), ppItem);
		}
	}


	// GetCount: Returns the number of items in the list.
	DWORD GetCount() const { return m_count; }

	bool IsEmpty() const {
		return (GetCount() == 0);
	}

	// Clear: Takes a functor object whose operator()
	// frees the object on the list.
	template <class FN>
	void Clear(FN& clear_fn) {
		Node *n = m_anchor.next;

		// Delete the nodes
		while (n != &m_anchor) {
			clear_fn(n->item);

			Node *tmp = n->next;
			delete n;
			n = tmp;
		}

		// Reset the anchor to point at itself
		m_anchor.next = &m_anchor;
		m_anchor.prev = &m_anchor;

		m_count = 0;
	}

	// Clear: Clears the list. (Does not delete or release the list items.)
	virtual void Clear() {
		Clear<NoOp<T>>(NoOp<T>());
	}


	// Enumerator functions

	POSITION FrontPosition() {
		if (IsEmpty()) {
			return POSITION(NULL);
		} else {
			return POSITION(Front());
		}
	}

	POSITION EndPosition() const {
		return POSITION();
	}

	HRESULT GetItemPos(POSITION pos, T *ppItem) {
		if (pos.pNode) {
			return GetItem(pos.pNode, ppItem);
		} else {
			return E_FAIL;
		}
	}

	POSITION Next(const POSITION pos) {
		if (pos.pNode && (pos.pNode->next != &m_anchor)) {
			return POSITION(pos.pNode->next);
		} else {
			return POSITION(NULL);
		}
	}

	// Remove an item at a position. 
	// The item is returns in ppItem, unless ppItem is NULL.
	// NOTE: This method invalidates the POSITION object.
	HRESULT Remove(POSITION& pos, T *ppItem) {
		if (pos.pNode) {
			// Remove const-ness temporarily...
			Node *pNode = const_cast<Node*>(pos.pNode);

			pos = POSITION();

			return RemoveItem(pNode, ppItem);
		} else {
			return E_INVALIDARG;
		}
	}

};

// Typical functors for Clear method.

// ComAutoRelease: Releases COM pointers.
// MemDelete: Deletes pointers to new'd memory.

class ComAutoRelease {
public:
	void operator()(IUnknown *p) {
		if (p) {
			p->Release();
		}
	}
};

class MemDelete {
public:
	void operator()(void *p) {
		if (p) {
			delete p;
		}
	}
};

// ComPtrList class
// Derived class that makes it safer to store COM pointers in the List<> class.
// It automatically AddRef's the pointers that are inserted onto the list
// (unless the insertion method fails). 
//
// T must be a COM interface type. 
// example: ComPtrList<IUnknown>
//
// NULLABLE: If true, client can insert NULL pointers. This means GetItem can
// succeed but return a NULL pointer. By default, the list does not allow NULL
// pointers.

template <class T, bool NULLABLE = FALSE>
class ComPtrList: public List<T*> {
public:

	typedef T* Ptr;

	void Clear() {
		List<Ptr>::Clear(ComAutoRelease());
	}

	~ComPtrList() {
		Clear();
	}

protected:
	HRESULT InsertAfter(Ptr item, Node *pBefore) {
		// Do not allow NULL item pointers unless NULLABLE is true.
		if (!item && !NULLABLE) {
			return E_POINTER;
		}

		if (item) {
			item->AddRef();
		}

		HRESULT hr = List<Ptr>::InsertAfter(item, pBefore);
		if (FAILED(hr)) {
			SAFE_RELEASE(item);
		}
		return hr;
	}

	HRESULT GetItem(const Node *pNode, Ptr* ppItem) {
		Ptr pItem = NULL;

		// The base class gives us the pointer without AddRef'ing it.
		// If we return the pointer to the caller, we must AddRef().
		HRESULT hr = List<Ptr>::GetItem(pNode, &pItem);
		if (SUCCEEDED(hr)) {
			assert(pItem || NULLABLE);
			if (pItem) {
				*ppItem = pItem;
				(*ppItem)->AddRef();
			}
		}
		return hr;
	}

	HRESULT RemoveItem(Node *pNode, Ptr *ppItem) {
		// ppItem can be NULL, but we need to get the
		// item so that we can release it. 

		// If ppItem is not NULL, we will AddRef it on the way out.

		Ptr pItem = NULL;

		HRESULT hr = List<Ptr>::RemoveItem(pNode, &pItem);

		if (SUCCEEDED(hr)) {
			assert(pItem || NULLABLE);
			if (ppItem && pItem) {
				*ppItem = pItem;
				(*ppItem)->AddRef();
			}

			SAFE_RELEASE(pItem);
		}

		return hr;
	}
};

//////////////////////////////////////////////////////////////////////////
//  AutoLock
//  Description: Provides automatic locking and unlocking of a 
//               of a critical section.
//
//  Note: The AutoLock object must go out of scope before the CritSec.
//////////////////////////////////////////////////////////////////////////

class AutoLock {
private:
	CritSec *m_pCriticalSection;
public:
	AutoLock(CritSec& crit) {
		m_pCriticalSection = &crit;
		m_pCriticalSection->Lock();
	}
	~AutoLock() {
		m_pCriticalSection->Unlock();
	}
};

/*
#define TRACE_INIT() 
#define TRACE(x) 
#define TRACE_CLOSE()
#define LOG_MSG_IF_FAILED(x, hr)
#define LOG_HRESULT(hr)
#define LOG_MSG_IF_FAILED(msg, hr)
*/

// common.h END

// Forward declares
class CWavStream;
class CMarker;

enum FlushState
{
    DropSamples = 0,
    WriteSamples 
};


// IMarker:
// Custom interface for handling IMFStreamSink::PlaceMarker calls asynchronously.

// A marker consists of a marker type, marker data, and context data. 
// By defining this interface, we can store the marker data inside an IUnknown object
// and keep that object on the same queue that holds the media samples. This is 
// useful because samples and markers must be serialized. That is, we cannot respond
// to a marker until we have processed all of the samples that came before it.

// Note that IMarker is not a standard Media Foundation interface.
MIDL_INTERFACE("3AC82233-933C-43a9-AF3D-ADC94EABF406")
IMarker : public IUnknown
{
    virtual STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType) = 0;
    virtual STDMETHODIMP GetMarkerValue(PROPVARIANT *pvar) = 0;
    virtual STDMETHODIMP GetContext(PROPVARIANT *pvar) = 0;
};



class CWavSink : public IMFFinalizableMediaSink,   // Note: IMFFinalizableMediaSink inherits IMFMediaSink
                 public IMFClockStateSink
{

    friend class CWavStream;

public:
    // Static method to create the object.
    static HRESULT CreateInstance(IMFByteStream *pStream, REFIID iid, void **ppSink);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaSink methods
    STDMETHODIMP GetCharacteristics(DWORD *pdwCharacteristics);

    STDMETHODIMP AddStreamSink( 
        /* [in] */ DWORD dwStreamSinkIdentifier,
        /* [in] */ IMFMediaType *pMediaType,
        /* [out] */ IMFStreamSink **ppStreamSink);

    STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier);
    STDMETHODIMP GetStreamSinkCount(DWORD *pcStreamSinkCount);
    STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink **ppStreamSink);
    STDMETHODIMP GetStreamSinkById(DWORD dwIdentifier, IMFStreamSink **ppStreamSink);
    STDMETHODIMP SetPresentationClock(IMFPresentationClock *pPresentationClock);
    STDMETHODIMP GetPresentationClock(IMFPresentationClock **ppPresentationClock);
    STDMETHODIMP Shutdown();

    // IMFFinalizableMediaSink methods
    STDMETHODIMP BeginFinalize(IMFAsyncCallback *pCallback, IUnknown *punkState);
    STDMETHODIMP EndFinalize(IMFAsyncResult *pResult);

    // IMFClockStateSink methods
    STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

private:

    CWavSink();
    virtual ~CWavSink();

    HRESULT Initialize(IMFByteStream *pByteStream);

    HRESULT CheckShutdown() const
    {
        if (m_IsShutdown)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }


    long                        m_nRefCount;                // reference count
    CritSec                     m_critSec;                  // critical section for thread safety

    BOOL                        m_IsShutdown;               // Flag to indicate if Shutdown() method was called.

    CWavStream                  *m_pStream;                 // Byte stream
    IMFPresentationClock        *m_pClock;                  // Presentation clock.
};


class CWavStream : public IMFStreamSink, public IMFMediaTypeHandler
{
    friend class CWavSink;

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFStreamSink
    STDMETHODIMP GetMediaSink(IMFMediaSink **ppMediaSink);
    STDMETHODIMP GetIdentifier(DWORD *pdwIdentifier);
    STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler **ppHandler);
    STDMETHODIMP ProcessSample(IMFSample *pSample);
    
    STDMETHODIMP PlaceMarker( 
        /* [in] */ MFSTREAMSINK_MARKER_TYPE eMarkerType,
        /* [in] */ const PROPVARIANT *pvarMarkerValue,
        /* [in] */ const PROPVARIANT *pvarContextValue);
    
    STDMETHODIMP Flush();

    // IMFMediaTypeHandler
    STDMETHODIMP IsMediaTypeSupported(IMFMediaType *pMediaType, IMFMediaType **ppMediaType);
    STDMETHODIMP GetMediaTypeCount(DWORD *pdwTypeCount);
    STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType **ppType);
    STDMETHODIMP SetCurrentMediaType(IMFMediaType *pMediaType);
    STDMETHODIMP GetCurrentMediaType(IMFMediaType **ppMediaType);
    STDMETHODIMP GetMajorType(GUID *pguidMajorType);


private:

    // State enum: Defines the current state of the stream.
    enum State
    {
        State_TypeNotSet = 0,    // No media type is set
        State_Ready,             // Media type is set, Start has never been called.
        State_Started,  
        State_Stopped,
        State_Paused,
        State_Finalized,

        State_Count = State_Finalized + 1    // Number of states
    };

    // StreamOperation: Defines various operations that can be performed on the stream.
    enum StreamOperation
    {
        OpSetMediaType = 0,
        OpStart,
        OpRestart,
        OpPause,
        OpStop,
        OpProcessSample,
        OpPlaceMarker,
        OpFinalize,

        Op_Count = OpFinalize + 1  // Number of operations
    };

    // CAsyncOperation:
    // Used to queue asynchronous operations. When we call MFPutWorkItem, we use this
    // object for the callback state (pState). Then, when the callback is invoked,
    // we can use the object to determine which asynchronous operation to perform.
 
    class CAsyncOperation : public IUnknown
    {
    public:
        CAsyncOperation(StreamOperation op);

        StreamOperation m_op;   // The operation to perform.  

        // IUnknown methods.
        STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

    private:
        long    m_nRefCount;
        virtual ~CAsyncOperation();
    };

    
    // ValidStateMatrix: Defines a look-up table that says which operations
    // are valid from which states.
    static BOOL ValidStateMatrix[State_Count][Op_Count];


    CWavStream();
    virtual ~CWavStream();

    HRESULT Initialize(CWavSink *pParent, IMFByteStream *pStream);

    HRESULT CheckShutdown() const
    {
        if (m_IsShutdown)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }


    HRESULT     Start(MFTIME start);
    HRESULT     Restart();
    HRESULT     Stop();
    HRESULT     Pause();
    HRESULT     Finalize(IMFAsyncCallback *pCallback, IUnknown *punkState);
    HRESULT     Shutdown();

    HRESULT     ValidateOperation(StreamOperation op);

    HRESULT     QueueAsyncOperation(StreamOperation op);
    
    HRESULT     OnDispatchWorkItem(IMFAsyncResult* pAsyncResult);
    HRESULT     DispatchProcessSample(CAsyncOperation* pOp);
    HRESULT     DispatchFinalize(CAsyncOperation* pOp);

    HRESULT     ProcessSamplesFromQueue(FlushState bFlushData);
    HRESULT     WriteSampleToFile(IMFSample *pSample);
    HRESULT     SendMarkerEvent(IMarker *pMarker, FlushState bFlushData);


    long                        m_nRefCount;                // reference count
    CritSec                     m_critSec;                  // critical section for thread safety

    State                       m_state;
    BOOL                        m_IsShutdown;               // Flag to indicate if Shutdown() method was called.
    
    DWORD                       m_WorkQueueId;              // ID of the work queue for asynchronous operations.
    AsyncCallback<CWavStream>   m_WorkQueueCB;              // Callback for the work queue.

    MFTIME                      m_StartTime;                // Presentation time when the clock started.
    DWORD                       m_cbDataWritten;            // How many bytes we have written so far.

    CWavSink                    *m_pSink;                   // Parent media sink

    IMFMediaEventQueue          *m_pEventQueue;             // Event queue
    IMFByteStream               *m_pByteStream;             // Bytestream where we write the data.
    IMFMediaType                *m_pCurrentType;

    ComPtrList<IUnknown>        m_SampleQueue;              // Queue to hold samples and markers.
                                                            // Applies to: ProcessSample, PlaceMarker, BeginFinalize

    IMFAsyncResult              *m_pFinalizeResult;         // Result object for Finalize operation.

};




// Holds marker information for IMFStreamSink::PlaceMarker 

class CMarker : public IMarker
{
public:
    static HRESULT Create(
        MFSTREAMSINK_MARKER_TYPE eMarkerType,
        const PROPVARIANT* pvarMarkerValue,
        const PROPVARIANT* pvarContextValue,
        IMarker **ppMarker
        );

    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType);
    STDMETHODIMP GetMarkerValue(PROPVARIANT *pvar);
    STDMETHODIMP GetContext(PROPVARIANT *pvar);

protected:
    MFSTREAMSINK_MARKER_TYPE m_eMarkerType;
    PROPVARIANT m_varMarkerValue;
    PROPVARIANT m_varContextValue;

private:
    long    m_nRefCount;

    CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType);
    virtual ~CMarker();
};
