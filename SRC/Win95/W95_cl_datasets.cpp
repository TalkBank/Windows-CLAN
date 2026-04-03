#include "ced.h"
#include "cu.h"
#include "w95_cl_datasets.h"

struct label_cols {
	char* label;
	int  num;
	struct label_cols* next_label;
};
static struct label_cols* datasets;

/////////////////////////////////////////////////////////////////////////////
// CClanDataset dialog
extern struct DefWin DatasetWinSize;

IMPLEMENT_DYNAMIC(CClanDataset, CDialog)

CClanDataset::CClanDataset(CWnd* pParent /*=NULL*/)
	: CDialog(CClanDataset::IDD, pParent)
{

}

void CClanDataset::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClanDataset)
	DDX_Control(pDX, IDC_LIST_OF_DATASETS, m_ClanDatasetListControl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClanDataset, CDialog)
	//{{AFX_MSG_MAP(CClanProgs)
	ON_LBN_DBLCLK(IDC_LIST_OF_DATASETS, OnDblclkClanDatasets)
	ON_LBN_SELCHANGE(IDC_LIST_OF_DATASETS, OnSelchangeClanDatasets)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


static struct label_cols* freeLabels(struct label_cols* p) {
	struct label_cols* t;

	while (p != NULL) {
		t = p;
		p = p->next_label;
		if (t->label)
			free(t->label);
		free(t);
	}
	return(NULL);
}

static struct label_cols* addDSToList(label_cols* root, char* label) {
	struct label_cols* ts;

	if (root == NULL) {
		root = NEW(struct label_cols);
		ts = root;
	}
	else {
		for (ts = root; ts->next_label != NULL; ts = ts->next_label) {
			if (uS.mStricmp(ts->label, label) == 0)
				return(root);
		}
		if (uS.mStricmp(ts->label, label) == 0)
			return(root);
		ts->next_label = NEW(struct label_cols);
		ts = ts->next_label;
	}
	if (ts == NULL) {
		root = freeLabels(root);
	}
	ts->next_label = NULL;
	ts->label = (char*)malloc(strlen(label) + 1);
	if (ts->label == NULL) {
		root = freeLabels(root);
	}
	//	uS.lowercasestr(label, &dFnt, C_MBF);
	strcpy(ts->label, label);
	return(root);
}

void CClanDataset::listDatasets(char* f) {
	char* b, * e;
	char ds_name[128], ds_nameo[512], name[128], ds_age[64], age[64];
	char ds_activity[128], ds_activityo[128], activity[128];
	int cnt, agef, aget, ds_agef, ds_aget;
	FILE* fp;

	datasets = NULL;
	strcpy(FileName1, lib_dir);
	addFilename2Path(FileName1, "kideval");
	addFilename2Path(FileName1, "0refvals.csv");
	fp = fopen(FileName1, "r");
	if (fp == NULL) {
		strcpy(FileName1, wd_dir);
		addFilename2Path(FileName1, "0refvals.csv");
		fp = fopen(FileName1, "r");
	}
	if (fp == NULL) {
		return;
	}

	name[0] = EOS;
	age[0] = EOS;
	activity[0] = EOS;
	agef = 0;
	aget = 0;
	b = f;
	while (b != NULL) {
		e = strchr(b, '~');
		if (e != NULL) {
			*e = EOS;
			if (b == f) {
				if (isdigit(*b) && (isdigit(*(b + 1)) || *(b + 1) == ';' || *(b + 1) == '.' || *(b + 1) == '-' || *(b + 1) == EOS)) {
					strncpy(age, b, 62);
					age[62] = EOS;
				}
				else {
					strncpy(name, b, 126);
					name[126] = EOS;
					uS.lowercasestr(name, &dFnt, MBF);
				}
			}
			else if (isdigit(*b)) {
				strncpy(age, b, 62);
				age[62] = EOS;
			}
			else {
				strncpy(activity, b, 126);
				activity[126] = EOS;
				uS.lowercasestr(activity, &dFnt, MBF);
			}
			*e = '~';
			b = e + 1;
		}
		else {
			if (b == f) {
				if (isdigit(*b) && (isdigit(*(b + 1)) || *(b + 1) == ';' || *(b + 1) == '.' || *(b + 1) == '-' || *(b + 1) == EOS)) {
					strncpy(age, b, 62);
					age[62] = EOS;
				}
				else {
					strncpy(name, b, 126);
					name[126] = EOS;
					uS.lowercasestr(name, &dFnt, MBF);
				}
			}
			else if (isdigit(*b)) {
				strncpy(age, b, 62);
				age[62] = EOS;
			}
			else {
				strncpy(activity, b, 126);
				activity[126] = EOS;
				uS.lowercasestr(activity, &dFnt, MBF);
			}
			b = NULL;
		}
	}
	if (age[0] != EOS) {
		if (isAge(age, &agef, &aget) == FALSE) {
			return;
		}
	}
	datasets = NULL;
	if (!fgets_cr(templineC4, BUFSIZ, fp))
		return;
	while (fgets_cr(templineC4, BUFSIZ, fp)) {
		if (uS.isUTF8(templineC4) || uS.isInvisibleHeader(templineC4))
			continue;
		ds_name[0] = EOS;
		ds_nameo[0] = EOS;
		uS.remblanks(templineC4);
		b = templineC4;
		cnt = 0;
		while (b != NULL) {
			e = strchr(b, ',');
			if (e != NULL) {
				*e = EOS;
				cnt++;
				if (cnt == 3) {
					strncpy(ds_nameo, b, 510);
					ds_nameo[510] = EOS;
					strncpy(ds_name, b, 126);
					ds_name[126] = EOS;
					uS.lowercasestr(ds_name, &dFnt, MBF);
				}
				else if (cnt == 5) {
					strncpy(ds_age, b, 62);
					ds_age[62] = EOS;
					isAge(ds_age, &ds_agef, &ds_aget);
				}
				else if (cnt == 14) {
					strncpy(ds_activityo, b, 126);
					ds_activityo[126] = EOS;
					strncpy(ds_activity, b, 126);
					ds_activity[126] = EOS;
					uS.lowercasestr(ds_activity, &dFnt, MBF);
					*e = ',';
					if (name[0] == EOS || uS.fpatmat(ds_name, name) == 1) {
						if ((agef == 0 && aget == 0) ||
							(aget == 0 && agef >= ds_agef && agef <= ds_aget) ||
							(agef == ds_agef && aget == ds_aget)) {
							if (activity[0] == EOS || uS.fpatmat(ds_activity, activity) == 1) {
								strcat(ds_nameo, "~");
								strcat(ds_nameo, ds_age);
								strcat(ds_nameo, "    ");
								strcat(ds_nameo, ds_activityo);
								datasets = addDSToList(datasets, ds_nameo);
							}
						}
					}
					break;
				}
				*e = ',';
				b = e + 1;
			}
			else {
				cnt++;
				if (cnt == 3) {
					strncpy(ds_nameo, b, 510);
					ds_nameo[510] = EOS;
					strncpy(ds_name, b, 126);
					ds_name[126] = EOS;
					uS.lowercasestr(ds_name, &dFnt, MBF);
				}
				else if (cnt == 5) {
					strncpy(ds_age, b, 62);
					ds_age[62] = EOS;
					isAge(ds_age, &ds_agef, &ds_aget);
				}
				else if (cnt == 14) {
					strncpy(ds_activity, b, 126);
					ds_activity[126] = EOS;
					uS.lowercasestr(ds_activity, &dFnt, MBF);
					if (name[0] == EOS || uS.fpatmat(ds_name, name) == 1) {
						if ((agef == 0 && aget == 0) ||
							(aget == 0 && agef >= ds_agef && agef <= ds_aget) ||
							(agef == ds_agef && aget == ds_aget)) {
							if (activity[0] == EOS || uS.fpatmat(ds_activity, activity) == 1) {
								strcat(ds_nameo, "~");
								strcat(ds_nameo, ds_age);
								strcat(ds_nameo, "    ");
								strcat(ds_nameo, ds_activityo);
								datasets = addDSToList(datasets, ds_nameo);
							}
						}
					}
					break;
				}
				b = NULL;
			}
		}
	}
	fclose(fp);
}

// CClanDataset message handlers
BOOL CClanDataset::OnInitDialog() {
	int i;
	int cnt;
	char datasetName[256];
	struct label_cols* ts;
	CRect lpRect;
	CDialog::OnInitDialog();

	if (DatasetWinSize.top || DatasetWinSize.width || DatasetWinSize.height || DatasetWinSize.left) {
		this->GetWindowRect(&lpRect);
		lpRect.top = DatasetWinSize.top;
		lpRect.bottom = DatasetWinSize.height;
		lpRect.left = DatasetWinSize.left;
		lpRect.right = DatasetWinSize.width;
		AdjustWindowSize(&lpRect);
		this->MoveWindow(&lpRect, FALSE);
	}

	m_ClanDatasetListControl.ResetContent();

	datasets = NULL;
	listDatasets(datasetArg);
	cnt = 0;
	if (datasets != NULL) {
		for (ts = datasets; ts != NULL; ts = ts->next_label) {
			cnt++;
			sprintf(datasetName, "#%2d: ", cnt);
			strcat(datasetName, ts->label);
			u_strcpy(templineW, datasetName, UTTLINELEN);
			m_ClanDatasetListControl.AddString(templineW);
		}
	}
	if (datasets != NULL)
		datasets = freeLabels(datasets);

	m_ClanDatasetListControl.SetCurSel(0);
	return TRUE;
}

void CClanDataset::OnDblclkClanDatasets()
{
	int i;
	CRect lpRect;

	i = m_ClanDatasetListControl.GetCurSel();
	if (i != LB_ERR) {
		m_ClanDatasetListControl.GetText(i, ced_line);
//		set_lastCommand(i);
	}
}

void CClanDataset::OnSelchangeClanDatasets()
{
	int i;

	i = m_ClanDatasetListControl.GetCurSel();
	if (i != LB_ERR)
		m_ClanDatasetListControl.GetText(i, ced_line);
}

void CClanDataset::OnOK() {
	CRect lpRect;

	this->GetWindowRect(&lpRect);
	DatasetWinSize.top = lpRect.top;
	DatasetWinSize.left = lpRect.left;
	DatasetWinSize.width = lpRect.right;
	DatasetWinSize.height = lpRect.bottom;
	WriteCedPreference();
	EndDialog(IDOK);
}

void CClanDataset::ResizeDatasetWindow(int cx, int cy) {
	RECT itemRect, winRect;
	UINT buttonsHight, buttonsWidth;
	CWnd* pw_OK, * pw_ListBox;

	pw_OK = this->GetDlgItem(IDOK);
	pw_ListBox = this->GetDlgItem(IDC_LIST_OF_DATASETS);
	if (this == NULL || pw_OK == NULL || pw_ListBox == NULL)
		return;
	this->GetClientRect(&winRect);

	pw_OK->GetWindowRect(&itemRect);
	buttonsHight = itemRect.bottom - itemRect.top;
	buttonsWidth = itemRect.right - itemRect.left;
	itemRect.top = winRect.bottom - buttonsHight - 10;
	itemRect.bottom = itemRect.top + buttonsHight;
	itemRect.left = ((winRect.right - winRect.left) / 2) - (buttonsWidth / 2) + 10;
	itemRect.right = itemRect.left + buttonsWidth;
	pw_OK->MoveWindow(&itemRect, true);

	pw_ListBox->GetWindowRect(&itemRect);
	itemRect.top = winRect.top + 10;
	itemRect.bottom = winRect.bottom - buttonsHight - 17;
	itemRect.left = winRect.left + 10;
	itemRect.right = winRect.right - 10;
	pw_ListBox->MoveWindow(&itemRect, true);
}

void CClanDataset::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ResizeDatasetWindow(cx, cy);
}
