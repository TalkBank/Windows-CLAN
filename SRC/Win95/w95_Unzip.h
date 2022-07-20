#pragma once


// CUnzip dialog

class CUnzip: public CDialog
{

public:
	CUnzip(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUnzip();
	virtual BOOL DestroyWindow();
	virtual void OnCancel();

// Dialog Data
	enum { IDD = IDD_UNZIP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

extern bool UnZipFolder(LPCTSTR zipFile, LPCTSTR destination);
extern void GetMORGrammar(char *grammar, size_t fileSize);
extern void GetKidevalDB(const char *database, size_t fileSize);
