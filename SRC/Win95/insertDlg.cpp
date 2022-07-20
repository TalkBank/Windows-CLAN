// insertDlg.cpp : implementation file
//

#include "ced.h"
#include "c_clan.h"
#include "insertDlg.h"

#ifdef __MWERKS__
#include <unistd.h>
#else
#include "msutil.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CInsertDlg *insertDlg = NULL;
/////////////////////////////////////////////////////////////////////////////
// CInsertDlg dialog

CInsertDlg::CInsertDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsertDlg)
	m_group = _T("");
	m_age = _T("");
	m_corpus = _T("");
	m_language = _T("");
	m_role = _T("");
	m_SES = _T("");
	m_sex = _T("");
	m_all = FALSE;
	m_split = FALSE;
	m_howmany = _T("");
	m_setting = _T("");
	m_check_language = FALSE;
	m_check_age = FALSE;
	m_check_corpus = FALSE;
	m_check_group = FALSE;
	m_check_SES = FALSE;
	m_check_sex = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	header=_T("");
	LPCTSTR lpszTemplateName = MAKEINTRESOURCE(IDD);
	ASSERT(HIWORD(lpszTemplateName) == 0 ||
		AfxIsValidString(lpszTemplateName));
	m_lpszTemplateName = lpszTemplateName;  // used for help
	if (HIWORD(m_lpszTemplateName) == 0 && m_nIDHelp == 0)
		m_nIDHelp = LOWORD((DWORD)m_lpszTemplateName);
	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	ASSERT(hTemplate != NULL);
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
/*
	CDialogTemplate dlgTemp(lpDialogTemplate);
	if (!strcmp(dFnt.fontName, "Courier") ||
			(!strcmp(dFnt.fontName, "Courier New") && dFnt.CharSet <= 1))
		dlgTemp.SetFont("MS Sans Serif", (WORD)8);
	else
		dlgTemp.SetFont(dFnt.fontName, (WORD)dFnt.fontSize);
	HGLOBAL hTemplate2 = dlgTemp.Detach();
	if (hTemplate2 != NULL)
		lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate2);
*/
	CreateIndirect(lpDialogTemplate,pParent,NULL,NULL/*hInst*/);
/*
	if (hTemplate2 != NULL) {
		GlobalUnlock(hTemplate2);
		GlobalFree(hTemplate2);
	}
*/
	UnlockResource(hTemplate);
	FreeResource(hTemplate);
}

void CInsertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertDlg)
	DDX_Control(pDX, IDC_Sid, m_sid);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_EDIT_group, m_group);
	DDX_Text(pDX, IDC_EDIT_age, m_age);
	DDX_Text(pDX, IDC_EDIT_corpus, m_corpus);
	DDX_Text(pDX, IDC_EDIT_language, m_language);
	DDX_Text(pDX, IDC_EDIT_role, m_role);
	DDX_Text(pDX, IDC_EDIT_SES, m_SES);
	DDX_Text(pDX, IDC_EDIT_sex, m_sex);
	DDX_Check(pDX, IDC_all, m_all);
	DDX_Check(pDX, IDC_Split, m_split);
	DDX_Text(pDX, IDC_participant, m_howmany);
	DDX_Text(pDX, IDC_EDIT_setting, m_setting);
	DDX_Check(pDX, IDC_check_language, m_check_language);
	DDX_Check(pDX, IDC_check_age, m_check_age);
	DDX_Check(pDX, IDC_check_corpus, m_check_corpus);
	DDX_Check(pDX, IDC_check_group, m_check_group);
	DDX_Check(pDX, IDC_check_SES, m_check_SES);
	DDX_Check(pDX, IDC_check_sex, m_check_sex);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CInsertDlg, CDialog)
	//{{AFX_MSG_MAP(CInsertDlg)
	ON_WM_PAINT()
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_LBN_SETFOCUS(IDC_LIST1, OnSetfocusList1)
	ON_BN_CLICKED(IDC_Apply, OnApply)
	ON_CBN_SELCHANGE(IDC_Sid, OnSelchangeSid)
	ON_BN_CLICKED(IDC_DIR, OnDir)
	ON_BN_CLICKED(IDC_all, Onall)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertDlg message handlers

BOOL CInsertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_dir=_T("");
	m_dir.FreeExtra();
//	GetCurrentDirectory(_MAX_PATH,m_dir.GetBufferSetLength(_MAX_PATH));
	strcpy(m_dir.GetBufferSetLength(_MAX_PATH), wd_st_full);
	m_dir.ReleaseBuffer();
	m_dir=m_dir.Mid(0,m_dir.ReverseFind('\\'));
	m_list.Dir(0,m_dir+_T("\\*.cha"));
	UpdateData(false);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CInsertDlg::OnCancel() {
	DestroyWindow();
	delete insertDlg;
	insertDlg = NULL;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CInsertDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
	}
	else
	{
		CDialog::OnPaint();
	}
}

void CInsertDlg::GetHeader(LPCTSTR name)
{
	CFile file;
	file.Open(name,CFile::modeRead |CFile::shareDenyWrite);
	char pbuf[100];
	int nbytes=file.Read(pbuf,100);
	header=_T("");
	header.FreeExtra();
	while (nbytes){
		CString chunk =CString (pbuf,nbytes);
		int star=chunk.Find(_T("\n*"));
		if (star>=0){
			header+=chunk.Mid(0,star);
			break;
		}else header+=chunk;
		nbytes=file.Read(pbuf,100);
	}
	oldheaderlen=header.GetLength();
	file.Close();
	m_n=0;int pos=-1;
	CString persons=_T("@Participants");
	SetField(persons);
	do{//set the numer of participants
		pos=persons.Find(',');
		m_n++;
		persons=persons.Mid(pos+1);
		participants.Add(participant());
	} while (pos>-1);
#ifdef _UNICODE
	_itow(m_n,m_howmany.GetBuffer(2),10);
#else
	itoa(m_n,m_howmany.GetBuffer(2),10);
#endif
	m_howmany.ReleaseBuffer();
	m_howmany+=_T(" participants");
}

void CInsertDlg::GetID(CString &ID,int which)
{
	int pos=0,i=0;
	while(i<which){
		i++;
		pos=ID.Find(_T("@ID:"));
		if (pos==-1) {
			ID=_T("");return;
		}else {
			pos+=5;
			ID=ID.Mid(pos);
		}
	}
	i=ID.FindOneOf(_T("@*"));
	ID=ID.Mid(0,i);
	ID.TrimRight(_T(" \n\r\t"));
	if (ID==_T("")) return;
	//if (ID.Right(1)!=_T("."))  ID+=_T(".");
	if (ID.Right(1)!=_T("|"))  ID+=_T("|");
	ID.TrimLeft((char)0x02);
	ID.TrimLeft(_T(" \n\r\t"));
}

void CInsertDlg::Display(int i) 
{
	// TODO: Add your control notification handler code here
	//read fields in ID
	int j=0;
	CString ID=_T(""),temp;
	m_all=false;
	m_split=false;
	if (i!=LB_ERR){
		CString filename;
		m_list.GetText(i,filename);
		GetHeader(m_dir+_T("\\")+filename);
		m_sid.ResetContent();
		for (int which=1;which<=m_n;which++){
			ID=header;
			GetID(ID,which);

			if (ID.GetLength() == 0) {
				setting=m_setting;
				j = 0;
				for (j=0;j<m_n;j++){
					participants[j].language=m_language;
					participants[j].corpus=m_corpus;
				}
				j=m_sid.GetCurSel();
				if (j==CB_ERR) j=0;
				participants[j].age=m_age;
				participants[j].sex=m_sex;
				participants[j].group=m_group;
				participants[j].SES=m_SES;
				participants[j].role=m_role;
				BuildIDs(filename);
				Display(i);
				return;
			}
			
			int pos;
			CString temp=ID;
			j = 0;
			//while ((pos=temp.Find('.'))>-1){
			while ((pos=temp.Find('|'))>-1){
				pos++;j++;temp=temp.Mid(pos);
			}
			if (j!=9) {
				participants.RemoveAll();
				return;
			}
			GetFieldsFrom(ID,which);//fixed number of ID fields
			m_sid.InsertString(which-1,participants[which-1].sid);
		}
		m_sid.SetCurSel(0);
		OnSelchangeSid() ;
	}
	UpdateData(false);
	return;	
}

void CInsertDlg::GetFieldsFrom(CString ID, int which)
{
	which--;
	if (ID.Find('|')==-1)
		ID.Replace('.','|');
	if (ID!=_T("")){
		//int pos=ID.Find('.');
		int pos=ID.Find('|');
		if (pos>-1) {
			participants[which].language=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].language=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			participants[which].corpus=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].corpus=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		participants[which].sid=ID.Mid(0,pos);
		if (pos>-1) {
			participants[which].sid=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].sid=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			ID.Mid(0,pos).Replace('-','.');
			participants[which].age=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].age=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			participants[which].sex=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].sex=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			participants[which].group=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].group=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			participants[which].SES=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].SES=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			participants[which].role=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			participants[which].role=_T("");
		}
		//pos=ID.Find('.');
		pos=ID.Find('|');
		if (pos>-1) {
			setting=ID.Mid(0,pos);
			ID=ID.Mid(pos+1);
		}else {
			setting=_T("");
		}
	}else 
		participants[which].Reset();
}

void CInsertDlg::OnDblclkList1() 
{
	// TODO: Add your control notification handler code here
	int i=m_list.GetCurSel();
	CString fname,exe;
	if (i!=LB_ERR)	{
		Display(i);	
		m_list.GetText(i,fname);
		FindExecutable(fname,_T(""),exe.GetBuffer(MAX_PATH));
		fname=m_dir+_T("\\")+fname;
		ShellExecute (this->m_hWnd,NULL,exe,fname,m_dir,SW_SHOW);
	}
}

void CInsertDlg::OnSelchangeList1() 
{
	// TODO: Add your control notification handler code here
	int i=m_list.GetCurSel();
	if (i!=LB_ERR)	Display(i);	
}



void CInsertDlg::OnSetfocusList1() 
{
	// TODO: Add your control notification handler code here
	int i=m_list.GetCurSel();
	if (i!=LB_ERR)	Display(i);	
}

void CInsertDlg::SetField(CString& field)
{
	CString temp=_T(""); 
//	field.Replace(_T("/"),_T(" of "));
	int pos=-1;
	if ((pos=header.Find(field))>-1){
		temp=header.Mid(pos);
		temp=temp.Mid(temp.Find(':')+1);
		pos=temp.Find('@');
		if (pos==-1) pos=temp.Find('*');
		if (pos==-1) pos=temp.Find('.');
		if (pos!=-1) temp=temp.Mid(0,pos);
	}else temp=_T("");
	field=temp;
//	field.Replace(_T("."),_T("-"));
	field.Replace(_T("\n"),_T(" "));
	field.Replace(_T("\r"),_T(" "));
	field.Replace(_T("\t"),_T(" "));
	field.Replace((char)0x02,' ');
	field.TrimRight(_T(" "));
	field.TrimLeft(_T(" "));
}

void CInsertDlg::OnApply() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_dir.Right(1)=='\\')	m_dir=m_dir.Left(m_dir.GetLength()-1);
	CString fname,ID;
	if (m_all){
		int max=m_list.GetCount();
		int index=m_sid.GetCurSel();
		CString who;
		if (index!=LB_ERR)
		m_sid.GetLBText(index,who);
		else who=_T("");
		for (int i=0;i<max;i++){
			m_list.SetCurSel(i);
			m_list.GetText(i,fname);
			GetHeader(m_dir+_T("\\")+fname);
			setting=m_setting;
			for (int i=0;i<m_n;i++){
				ID=header;
				GetID(ID,i+1);
				GetFieldsFrom(ID,i+1);
				if (m_check_language) participants[i].language=m_language;
				if (m_check_corpus) participants[i].corpus=m_corpus;
				if (participants[i].sid==who && who!=_T("")){
					if (m_check_age) participants[index].age=m_age;
					if (m_check_sex) participants[index].sex=m_sex;
					if (m_check_group) participants[index].group=m_group;
					if (m_check_SES) participants[index].SES=m_SES;
					participants[index].role=_T("");
				}else{
					participants[i].age=_T("");
					participants[i].sex=_T("");
					participants[i].group=_T("");
					participants[i].SES=_T("");
					participants[i].role=_T("");
				}
			}
			BuildIDs(fname);
			if (m_split)ToBeSplit(fname);
			participants.RemoveAll();
		}
	}else{
		int i=m_list.GetCurSel();
		if (i==LB_ERR) return;
		m_list.GetText(i,fname);
		GetHeader(m_dir+_T("\\")+fname);
		setting=m_setting;
		for (i=0;i<m_n;i++){
			participants[i].language=m_language;
			participants[i].corpus=m_corpus;
		}
		i=m_sid.GetCurSel();
		if (i==CB_ERR) i=0;
		participants[i].age=m_age;
		participants[i].sex=m_sex;
		participants[i].group=m_group;
		participants[i].SES=m_SES;
		participants[i].role=m_role;
		BuildIDs(fname);
		Display(m_list.GetCurSel());	
		if (m_split)ToBeSplit(fname);
	}
}

void CInsertDlg::BuildIDs(CString& fname){
	CString ID=_T(""),temp=_T(""),tempID=_T(""),insert=_T("");
	CString persons=_T("@Participants");
	SetField(persons);
	if (persons==_T("")) {
		persons=_T("@PARTICIPANTS");
		SetField(persons);
	}
	int pos=-1;
	m_n=0;
	do{//set the numer of participants
		pos=persons.Find(',');
		m_n++;
		persons=persons.Mid(pos+1);
		participants.Add(participant());
	} while (pos>-1);
	persons=_T("@Participants");
	SetField(persons);
	if (persons==_T("")) {
		persons=_T("@PARTICIPANTS");
		SetField(persons);
	}
	CArray <bool,bool> targets;
	int index=0;
	for (int i=0;i<m_n;i++){
		tempID=_T("@ID:\t");
//		tempID+=participants[index].language +_T(".")+participants[index].corpus+_T("."); //+ participants[i].sid+_T(".");
		tempID+=participants[index].language +_T("|")+participants[index].corpus+_T("|");//+ participants[i].sid+_T(".");
		int pos=0;
		CString sid,role;
		pos=persons.Find(',');
		if (pos==-1){
			sid=persons;
		}else{
			sid=persons.Mid(0,pos);
			persons=persons.Mid(pos+1);
		}
		sid.TrimLeft(' ');
		sid.TrimRight(' ');
		pos=sid.ReverseFind(' ');
		role=sid.Mid(pos);
		role.TrimLeft(' ');
		role.TrimRight(' ');
		sid=sid.Mid(0,sid.Find(' '));
		role.TrimLeft(' ');
		if (role.Find(_T("Target"))==-1) {tempID=_T(""); targets.Add(false);continue;}
		else targets.Add(true);
		participants[index].role=role;
		participants[index].sid=sid;
//		tempID+=participants[index].sid+_T(".");
		tempID+=participants[index].sid+_T("|");
		temp=_T("@Age of ")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@AGE OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+="@Age of "+participants[index].sid+_T(":\t")+(participants[index].age!=_T("")?participants[index].age:temp)+_T("\r\n");
//		tempID+=(participants[index].age!=_T("")?participants[index].age:temp)+_T(".");
		tempID+=(participants[index].age!=_T("")?participants[index].age:temp)+_T("|");
		temp=_T("@Sex of")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@SEX OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@Sex of ")+participants[index].sid+_T(":\t")+(participants[index].sex!=_T("")?participants[index].sex:temp)+_T("\r\n");
//		tempID+=(participants[index].sex!=_T("")?participants[index].sex:temp)+_T(".");
		tempID+=(participants[index].sex!=_T("")?participants[index].sex:temp)+_T("|");
		temp=_T("@Group of")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@GROUP OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@Group of ")+participants[index].sid+_T(":\t")+(participants[index].group!=_T("")?participants[index].group:temp)+_T("\r\n");
//		tempID+=(participants[index].group!=_T("")?participants[index].group:temp)+_T(".");
		tempID+=(participants[index].group!=_T("")?participants[index].group:temp)+_T("|");
		temp=_T("@SES of ")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@SES OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@SES of ")+participants[index].sid+_T(":\t")+(participants[index].SES!=_T("")?participants[index].SES:temp)+_T("\r\n");
//		tempID+=(participants[index].SES!=_T("")?participants[index].SES:temp)+_T(".");
		tempID+=(participants[index].SES!=_T("")?participants[index].SES:temp)+_T("|");
		temp=_T("@Role of ")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@ROLE OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
//		tempID+=(participants[index].role!=_T("")?participants[index].role:temp)+_T(".");
		tempID+=(participants[index].role!=_T("")?participants[index].role:temp)+_T("|");
		temp=_T("@Setting");SetField(temp);
		if (temp==_T("")) {
			temp=_T("@SETTING")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		if (temp==_T("") && setting!=_T("")) insert+=_T("@Setting:\t")+setting+_T("\r\n");
//		tempID+=(setting!=_T("")?setting:temp)+_T(".");
		tempID+=(setting!=_T("")?setting:temp)+_T("|");
		if (tempID.Right(2).FindOneOf(_T("\r\n"))==-1) tempID+=_T("\r\n");
		ID+=tempID;
		index++;
	}
	//nontargets
	persons=_T("@Participants");
	SetField(persons);
	for (i=0;i<m_n;i++){
		tempID=_T("@ID:\t");
//		tempID+=participants[index].language +_T(".")+participants[index].corpus+_T(".");//+ participants[index].sid+_T(".");
		tempID+=participants[index].language +_T("|")+participants[index].corpus+_T("|");//+ participants[index].sid+_T(".");
		int pos=0;
		CString sid,role;
		pos=persons.Find(',');
		if (pos==-1){
			sid=persons;
		}else{
			sid=persons.Mid(0,pos);
			persons=persons.Mid(pos+1);
		}
		if (!targets[i]){
		sid.TrimLeft(' ');
		sid.TrimRight(' ');
		pos=sid.ReverseFind(' ');
		role=sid.Mid(pos);
		role.TrimLeft(' ');
		role.TrimRight(' ');
		participants[index].role=role;
		sid=sid.Mid(0,sid.Find(' '));
		participants[index].sid=sid;
		role.TrimLeft(' ');
//		tempID+=participants[index].sid+_T(".");
		tempID+=participants[index].sid+_T("|");
		temp=_T("@Age of ")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@AGE OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@Age of ")+participants[index].sid+_T(":\t")+(participants[index].age!=_T("")?participants[index].age:temp)+_T("\r\n");
//		tempID+=(participants[index].age!=_T("")?participants[index].age:temp)+_T(".");
		tempID+=(participants[index].age!=_T("")?participants[index].age:temp)+_T("|");
		temp=_T("@Sex of")+participants[index].sid;SetField(temp);	
		if (temp==_T("")) {
			temp=_T("@SEX OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@Sex of ")+participants[index].sid+_T(":\t")+(participants[index].sex!=_T("")?participants[index].sex:temp)+_T("\r\n");
//		tempID+=(participants[index].sex!=_T("")?participants[index].sex:temp)+_T(".");
		tempID+=(participants[index].sex!=_T("")?participants[index].sex:temp)+_T("|");
		temp=_T("@Group of")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@GROUP OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@Group of ")+participants[index].sid+_T(":\t")+(participants[index].group!=_T("")?participants[index].group:temp)+_T("\r\n");
//		tempID+=(participants[index].group!=_T("")?participants[index].group:temp)+_T(".");
		tempID+=(participants[index].group!=_T("")?participants[index].group:temp)+_T("|");
		temp=_T("@SES of ")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@SES OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		insert+=_T("@SES of ")+participants[index].sid+_T(":\t")+(participants[index].SES!=_T("")?participants[index].SES:temp)+_T("\r\n");
//		tempID+=(participants[index].SES!=_T("")?participants[index].SES:temp)+_T(".");
		tempID+=(participants[index].SES!=_T("")?participants[index].SES:temp)+_T("|");
		temp=_T("@Role of ")+participants[index].sid;SetField(temp);
		if (temp==_T("")) {
			temp=_T("@ROLE OF ")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
//		tempID+=(participants[index].role!=_T("")?participants[index].role:temp)+_T(".");
		tempID+=(participants[index].role!=_T("")?participants[index].role:temp)+_T("|");
		temp=_T("@Setting");SetField(temp);
		if (temp==_T("")) {
			temp=_T("@SETTING")+participants[index].sid;
			SetField(temp);
		}
		temp.Replace(_T("\r"),_T(" "));
		if (temp==_T("") && setting!=_T("")) insert+=_T("@Setting:\t")+setting+_T("\r\n");
//		tempID+=(setting!=_T("")?setting:temp)+_T(".");
		tempID+=(setting!=_T("")?setting:temp)+_T("|");
		if (tempID.Right(2).FindOneOf(_T("\r\n"))==-1) tempID+=_T("\r\n");
		ID+=tempID;
		index++;
		}
	}
//	temp=_T("@Situation");SetField(temp);
//	temp.Replace(_T("\r"),_T(" "));
//	if (temp!=_T("")) insert+=_T("@Situation:\t")+temp+_T("\r\n");

	//write the new header with included ID
	ID+=insert;
	targets.RemoveAll();
	m_sid.ResetContent();
	temp=header;
	GetID(temp,1);
	if (temp==_T("")){
		pos=header.Find(_T("@Participants"))+1;
		if (pos==0) pos=header.Find(_T("@PARTICIPANTS"))+1;
		pos+=(header.Mid(pos)).Find('@');
		temp=header.Mid(0,pos);
		temp+=ID;
		header=header.Mid(pos);
		do{
			pos=header.Find(_T("@ID"));
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Age of"));
			pos==-1?pos=header.Find(_T("@AGE OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Sex of"));
			pos==-1?pos=header.Find(_T("@SEX OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Group of"));
			pos==-1?pos=header.Find(_T("@GROUP OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Role of"));
			pos==-1?pos=header.Find(_T("@ROLE OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@SES of"));
			header=header.Mid(pos+1);
		}while(pos>-1);
		pos=header.Find(_T("\n"));
		if (pos==-1) {
			pos=header.Find(_T("\r"));
			if (pos==-1)
				pos=0;
			else
				pos++;
		} else
			pos++;
		temp+=header.Mid(pos);
	}else{
		pos=header.Find(_T("@ID"));
		temp=header.Mid(0,pos);
		temp+=ID;
		do{
			pos=header.Find(_T("@ID"));
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Age of"));
			pos==-1?pos=header.Find(_T("@AGE OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Sex of"));
			pos==-1?pos=header.Find(_T("@SEX OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Group of"));
			pos==-1?pos=header.Find(_T("@GROUP OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@Role of"));
			pos==-1?pos=header.Find(_T("@ROLE OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		do{
			pos=header.Find(_T("@SES of"));
			pos==-1?pos=header.Find(_T("@SES OF")):pos;
			header=header.Mid(pos+1);
		}while(pos>-1);
		pos=header.Find(_T("\n"));
		if (pos==-1) {
			pos=header.Find(_T("\r"));
			if (pos==-1)
				pos=0;
			else
				pos++;
		} else
			pos++;
		temp+=header.Mid(pos);
	}
	temp.TrimRight();
	header=temp;temp=_T("");
	header.TrimRight();
	WriteNewHeader(header,fname);
}

void CInsertDlg::WriteNewHeader(CString&header,CString &fname)
{
	CFile file,fileout;
	int nbytes=0;
	file.Open(m_dir+_T("\\")+fname,CFile::modeRead |CFile::shareDenyWrite);
	file.Seek(oldheaderlen,CFile::begin);
	fileout.Open(m_dir+_T("\\")+fname.Mid(0,fname.ReverseFind('.')),CFile::modeCreate  | CFile::modeWrite |CFile::shareDenyWrite|CFile::shareDenyRead);
	char buf[32768];
	fileout.Write(header,header.GetLength());
	do{
	nbytes=file.Read(buf,32768);
	fileout.Write(buf,nbytes);
	}while (nbytes==32768);
	fileout.Flush();
	file.Close();
	file.Remove(m_dir+_T("\\")+fname);
	fileout.Close();
	fileout.Rename(m_dir+_T("\\")+fname.Mid(0,fname.ReverseFind('.')),m_dir+_T("\\")+fname);
}

void CInsertDlg::ToBeSplit(CString & fname){
	GetHeader(m_dir+_T("\\")+fname);
	CFile file (fname, CFile::modeRead   | CFile::shareDenyWrite );
	int nbytes,maxbytes=file.GetLength();
	CString temp;
	file.Seek(header.GetLength(),CFile::begin);
	maxbytes-=header.GetLength();
	nbytes=file.Read(temp.GetBufferSetLength(maxbytes),maxbytes);
	temp.ReleaseBuffer(nbytes);
	CString which=_T("");
	if (temp.Find(_T("@Setting"))>-1 || temp.Find(_T("@setting"))>-1) which="setting";
	if (temp.Find(_T("@Age")) >-1 || temp.Find(_T("@age")) >-1) which="Age";
	if (temp.Find(_T("@Group"))>-1 || temp.Find(_T("@group"))>-1) which="Group";
	temp=_T("");
	if (which!=_T("")) AfxMessageBox(fname+ _T(" : ")+ which+_T(" field found more then once after header"));
	file.Close();
}

void CInsertDlg::OnSelchangeSid() 
{
	// TODO: Add your control notification handler code here
	int index=m_sid.GetCurSel();
	if (index<m_n && index!=CB_ERR){
		m_language=participants[index].language;
		m_corpus=participants[index].corpus;
		m_age=participants[index].age;
		m_sex=participants[index].sex;
		m_group=participants[index].group;
		m_SES=participants[index].SES;
		m_role=participants[index].role;
		m_setting=setting;
	}else{
		m_language=_T("");
		m_corpus=_T("");
		m_age=_T("");
		m_sex=_T("");
		m_group=_T("");
		m_SES=_T("");
		m_role=_T("");
		m_setting=_T("");
	}
	UpdateData(false);
}

void CInsertDlg::OnDir() 
{
	// TODO: Add your control notification handler code here
	participants.RemoveAll();
	m_dir=_T("");
	m_dir.FreeExtra();
	m_language=_T("");
	m_check_language=false;
	m_corpus=_T("");
	m_check_corpus=false;
	m_age=_T("");
	m_check_age=false;
	m_sex=_T("");
	m_check_sex=false;
	m_group=_T("");
	m_check_group=false;
	m_SES=_T("");
	m_check_SES=false;
	m_role=_T("");
	m_setting=_T("");
	m_sid.ResetContent();
	UpdateData(false);

	CFileDialog x(true);
	x.DoModal();
	m_dir=x.GetPathName();
/*
	BROWSEINFO dir;int image=0;
	dir.ulFlags=BIF_RETURNONLYFSDIRS;
	dir.pszDisplayName=m_dir.GetBuffer(_MAX_PATH);
	dir.lpfn =NULL;
	dir.pidlRoot =NULL;
	dir.hwndOwner=this->m_hWnd;
	dir.lpszTitle="Please choose the current working folder";
	dir.iImage=image;
	SHBrowseForFolder(&dir);
	m_dir.ReleaseBuffer();*/
	m_dir=m_dir.Mid(0, m_dir.ReverseFind('\\'));
	UpdateData();
	int max=m_list.GetCount();
	if (max!=LB_ERR){
		for (int i=max;i>=0;i--)
			m_list.DeleteString(i);
	}
	SetCurrentDirectory(m_dir);
	CString wildcard=m_dir+_T("\\*.cha");
#ifdef _DEBUG
	m_list.Dir(0x0000,wildcard);
#else
	DlgDirList(_T("*.cha"),m_list.GetDlgCtrlID(),0,DDL_READWRITE);
#endif
	
}

void CInsertDlg::Onall() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_all){
		if (m_language!=_T("")) m_check_language=true;
		if (m_corpus!=_T("")) m_check_corpus=true;
		if (m_age!=_T("")) m_check_age=true;
		if (m_sex!=_T("")) m_check_sex=true;
		if (m_SES!=_T("")) m_check_SES=true;
		if (m_group!=_T("")) m_check_group=true;
	}else{
		m_check_language=false;
		m_check_corpus=false;
		m_check_age=false;
		m_check_sex=false;
		m_check_SES=false;
		m_check_group=false;
	}
	m_role=_T("");
	UpdateData(false);
}
