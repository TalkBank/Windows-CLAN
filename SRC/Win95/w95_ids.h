// w95_ids.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#include "ids.h"
#include "afxwin.h"
#include "afxcmn.h"

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSetIDs dialog

class CSetIDs : public CDialog
{
	DECLARE_DYNAMIC(CSetIDs)

public:
	CSetIDs(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetIDs();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeIdsSpeakerid();
	afx_msg void OnBnClickedIdsUnk();
	afx_msg void OnBnClickedIdsMale();
	afx_msg void OnBnClickedIdsFemale();
	afx_msg void OnTiersIdheaders();
	afx_msg void OnEnChangeIdsCode();

// Dialog Data
	enum { IDD = IDD_ID_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void createPopupSESeMenu();
	void createPopupSESsMenu();
	void createPopupRoleMenu();
	void createPopupSpeakerMenu();
	void updateSpeakerMenu();
	void fillInIDFields();
	BOOL saveIDFields();

	DECLARE_MESSAGE_MAP()
public:
	IDSTYPE *rootIDs;
	ROLESTYPE *rootRoles;
	SESTYPE *rootSESe, *rootSESs;
	CComboBox m_SpeakerIDC;
//	CString m_SpeakerIDs;
	CComboBox m_RoleC;
	CString m_RoleS;
	afx_msg void OnIdsCopy();
	afx_msg void OnIdsCreate();
	afx_msg void OnIdsDelete();
	CEdit m_LanguageC;
	CString m_LanguageS;
	CEdit m_CorpusC;
	CString m_CorpusS;
	CEdit m_TierC;
	CString m_TierS;
	CEdit m_Y1C;
	CString m_Y1S;
	CEdit m_M1C;
	CString m_M1S;
	CEdit m_D1C;
	CString m_D1S;
	CEdit m_SexC;
	BOOL m_Unknown;
	BOOL m_Male;
	BOOL m_Female;
	CEdit m_GroupC;
	CString m_GroupS;
	CComboBox m_SES_eC;
	CString m_SES_eS;
	CComboBox m_SES_sC;
	CString m_SES_sS;
	CEdit m_EducationC;
	CString m_EducationS;
	CEdit m_UFIDC;
	CString m_UFIDS;
	CEdit m_SpNameC;
	CString m_SpNameS;

protected:
	int spItem;
};
