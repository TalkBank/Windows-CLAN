#if !defined(AFX_W95_PROGRESS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
#define AFX_W95_PROGRESS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W95_progress.h : header file
//

extern void initProgressBar(unCH *Label);
extern void setCurrentProgressBarValue(int perc);
extern void deleteProgressBar(void);
extern BOOL isProgressBarCanceled(void);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W95_PROGRESS_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
