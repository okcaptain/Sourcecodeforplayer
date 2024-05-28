/*

a. Derive your COM object from CUnknown

b. Make a static CreateInstance function that takes an LPUNKNOWN, an HRESULT *
   and a TCHAR *. The LPUNKNOWN defines the object to delegate IUnknown calls
   to. The HRESULT * allows error codes to be passed around constructors and
   the TCHAR * is a descriptive name that can be printed on the debugger.

   It is important that constructors only change the HRESULT * if they have
   to set an ERROR code, if it was successful then leave it alone or you may
   overwrite an error code from an object previously created.

   When you call a constructor the descriptive name should be in static store
   as we do not copy the string. To stop large amounts of memory being used
   in retail builds by all these static strings use the NAME macro,

   CMyFilter = new CImplFilter(NAME("My filter"),pUnknown,phr);
   if (FAILED(hr)) {
       return hr;
   }

   In retail builds NAME(_x_) compiles to NULL, the base CBaseObject class
   knows not to do anything with objects that don't have a name.

c. Have a constructor for your object that passes the LPUNKNOWN, HRESULT * and
   TCHAR * to the CUnknown constructor. You can set the HRESULT if you have an
   error, or just simply pass it through to the constructor.

   The object creation will fail in the class factory if the HRESULT indicates
   an error (ie FAILED(HRESULT) == TRUE)

d. Create a FactoryTemplate with your object's class id and CreateInstance
   function.

Then (for each interface) either

Multiple inheritance

1. Also derive it from ISomeInterface
2. Include DECLARE_IUNKNOWN in your class definition to declare
   implementations of QueryInterface, AddRef and Release that
   call the outer unknown
3. Override NonDelegatingQueryInterface to expose ISomeInterface by
   code something like

     if (riid == IID_ISomeInterface) {
         return GetInterface((ISomeInterface *) this, ppv);
     } else {
         return CUnknown::NonDelegatingQueryInterface(riid, ppv);
     }

4. Declare and implement the member functions of ISomeInterface.

or: Nested interfaces

1. Declare a class derived from CUnknown
2. Include DECLARE_IUNKNOWN in your class definition
3. Override NonDelegatingQueryInterface to expose ISomeInterface by
   code something like

     if (riid == IID_ISomeInterface) {
         return GetInterface((ISomeInterface *) this, ppv);
     } else {
         return CUnknown::NonDelegatingQueryInterface(riid, ppv);
     }

4. Implement the member functions of ISomeInterface. Use GetOwner() to
   access the COM object class.

And in your COM object class:

5. Make the nested class a friend of the COM object class, and declare
   an instance of the nested class as a member of the COM object class.

   NOTE that because you must always pass the outer unknown and an hResult
   to the CUnknown constructor you cannot use a default constructor, in
   other words you will have to make the member variable a pointer to the
   class and make a NEW call in your constructor to actually create it.

6. override the NonDelegatingQueryInterface with code like this:

     if (riid == IID_ISomeInterface) {
         return m_pImplFilter->
            NonDelegatingQueryInterface(IID_ISomeInterface, ppv);
     } else {
         return CUnknown::NonDelegatingQueryInterface(riid, ppv);
     }

You can have mixed classes which support some interfaces via multiple
inheritance and some via nested classes

*/

#ifndef __COMBASE_H__
#define __COMBASE_H__

#ifdef DEBUG
#define NAME(x) TEXT(x)
#else
#define NAME(__x__) ((TCHAR*)NULL)
#endif

#define VMFDAPI_OUTPORT                  extern "C" __declspec(dllexport) HRESULT  
#define VMFDAPI_OUTPORT_(type)           extern "C" __declspec(dllexport) type  

#ifndef __strmif_h__
// Filter Setup data structures no defined in axextend.idl
typedef /* [public][public][public][public][public][public][public] */ struct __MIDL___MIDL_itf_strmif_0138_0001
{
	const CLSID *clsMajorType;
	const CLSID *clsMinorType;
} 	REGPINTYPES;

typedef /* [public][public][public] */ struct __MIDL___MIDL_itf_strmif_0138_0002
{
	LPWSTR strName;
	BOOL bRendered;
	BOOL bOutput;
	BOOL bZero;
	BOOL bMany;
	const CLSID *clsConnectsToFilter;
	const WCHAR *strConnectsToPin;
	UINT nMediaTypes;
	const REGPINTYPES *lpMediaType;
} REGFILTERPINS;
#endif

typedef REGPINTYPES
AMOVIESETUP_MEDIATYPE, * PAMOVIESETUP_MEDIATYPE, * FAR LPAMOVIESETUP_MEDIATYPE;

typedef REGFILTERPINS
AMOVIESETUP_PIN, * PAMOVIESETUP_PIN, * FAR LPAMOVIESETUP_PIN;

typedef struct _AMOVIESETUP_FILTER
{
  const CLSID * clsID;
  const WCHAR * strName;
  DWORD      dwMerit;
  UINT       nPins;
  const AMOVIESETUP_PIN * lpPin;
}
AMOVIESETUP_FILTER, * PAMOVIESETUP_FILTER, * FAR LPAMOVIESETUP_FILTER;

extern VMF_Version g_Version;

/* The DLLENTRY module initialises the module handle on loading */

extern HINSTANCE g_hInst;


/* Version of IUnknown that is renamed to allow a class to support both
   non delegating and delegating IUnknowns in the same COM object */

#ifndef INONDELEGATINGUNKNOWN_DEFINED
struct INonDelegatingUnknown
{
	STDMETHOD(NonDelegatingQueryInterface) (THIS_ REFIID, LPVOID *) = 0;
	STDMETHOD_(ULONG, NonDelegatingAddRef)(THIS) = 0;
	STDMETHOD_(ULONG, NonDelegatingRelease)(THIS) = 0;
};
#define INONDELEGATINGUNKNOWN_DEFINED
#endif

typedef INonDelegatingUnknown *PNDUNKNOWN;


/* This is the base object class that supports active object counting. As
   part of the debug facilities we trace every time a C++ object is created
   or destroyed. The name of the object has to be passed up through the class
   derivation list during construction as you cannot call virtual functions
   in the constructor. The downside of all this is that every single object
   constructor has to take an object name parameter that describes it */

class CBaseObject
{

private:

    // Disable the copy constructor and assignment by default so you will get
    //   compiler errors instead of unexpected behavior if you pass objects
    //   by value or assign objects.
    CBaseObject(const CBaseObject& objectSrc);          // no implementation
    void operator=(const CBaseObject& objectSrc);       // no implementation

private:
    static LONG m_cObjects;     /* Total number of objects active */

protected:


public:

    /* These increment and decrement the number of active objects */

    CBaseObject(__in_opt LPCTSTR pName);
#ifdef UNICODE
    CBaseObject(__in_opt LPCSTR pName);
#endif
    ~CBaseObject();

    /* Call this to find if there are any CUnknown derived objects active */

    static LONG ObjectsActive() {
        return m_cObjects;
    };
};


/* An object that supports one or more COM interfaces will be based on
   this class. It supports counting of total objects for DLLCanUnloadNow
   support, and an implementation of the core non delegating IUnknown */

class CUnknown : public INonDelegatingUnknown,
                 public CBaseObject
{
private:
    const LPUNKNOWN m_pUnknown; /* Owner of this object */

protected:                      /* So we can override NonDelegatingRelease() */
    volatile VMF_Atomic m_cRef;       /* Number of reference counts */

public:

    CUnknown(__in_opt LPCTSTR pName, __in_opt LPUNKNOWN pUnk);
	virtual ~CUnknown() {VMF_DeleteAtomic(m_cRef);};

    // This is redundant, just use the other constructor
    //   as we never touch the HRESULT in this anyway
    CUnknown(__in_opt LPCTSTR Name, __in_opt LPUNKNOWN pUnk, __inout_opt HRESULT *phr);
#ifdef UNICODE
    CUnknown(__in_opt LPCSTR pName, __in_opt LPUNKNOWN pUnk);
    CUnknown(__in_opt LPCSTR pName, __in_opt LPUNKNOWN pUnk,__inout_opt HRESULT *phr);
#endif

    /* Return the owner of this object */

    LPUNKNOWN GetOwner() const {
        return m_pUnknown;
    };

    /* Called from the class factory to create a new instance, it is
       pure virtual so it must be overriden in your derived class */

    /* static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *) */

    /* Non delegating unknown implementation */

    STDMETHODIMP NonDelegatingQueryInterface(REFIID, __deref_out void **);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
};

/* Return an interface pointer to a requesting client
   performing a thread safe AddRef as necessary */

STDAPI GetInterface(LPUNKNOWN pUnk, __out void **ppv);

/* A function that can create a new COM object */

typedef CUnknown *(CALLBACK *LPFNNewCOMObject)(__in_opt LPUNKNOWN pUnkOuter, __inout_opt HRESULT *phr);
/*  A function (can be NULL) which is called from the DLL entrypoint
    routine for each factory template:

    bLoading - TRUE on DLL load, FALSE on DLL unload
    rclsid   - the m_ClsID of the entry
*/
typedef void (CALLBACK *LPFNInitRoutine)(BOOL bLoading, const CLSID *rclsid);

/* Create one of these per object class in an array so that
   the default class factory code can create new instances */

class CFactoryTemplate {

public:

    const WCHAR *              m_Name;
    const CLSID *              m_ClsID;
    LPFNNewCOMObject           m_lpfnNew;
    LPFNInitRoutine            m_lpfnInit;
    const AMOVIESETUP_FILTER * m_pAMovieSetup_Filter;
 
    BOOL IsClassID(REFCLSID rclsid) const {
        return (IsEqualCLSID(*m_ClsID,rclsid));
    };

    CUnknown *CreateInstance(__inout_opt LPUNKNOWN pUnk, __inout_opt HRESULT *phr) const {
		if(!phr) return NULL;
        return m_lpfnNew(pUnk, phr);
    };
};


/* You must override the (pure virtual) NonDelegatingQueryInterface to return
   interface pointers (using GetInterface) to the interfaces your derived
   class supports (the default implementation only supports IUnknown) */

#define DECLARE_IUNKNOWN                                        \
    STDMETHODIMP QueryInterface(REFIID riid, __deref_out void **ppv) {      \
	return CUnknown::GetOwner()->QueryInterface(riid,ppv);      \
    };                                                          \
    STDMETHODIMP_(ULONG) AddRef() {                             \
	return CUnknown::GetOwner()->AddRef();                      \
    };                                                          \
    STDMETHODIMP_(ULONG) Release() {                            \
	return CUnknown::GetOwner()->Release();                     \
    };


STDAPI  DllGetClassObject(__in REFCLSID rclsid, __in REFIID riid, LPVOID FAR* ppv);
#ifndef COMPILE_METRO
STDAPI  DllCanUnloadNow();
#endif
STDAPI  DllGetVersion(VMF_Version *pVersion);
STDAPI DllSetComManager(VOID *pCOMManager);
STDAPI  AMovieGetRegisterInfo(CLSID **ppclsidCategory, VMF_VOID **ppRegisterInfo, UINT *pRegisterNum);

#endif /* __COMBASE__ */



