#if !defined(AFX_W95_FEEDBACK_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
#define AFX_W95_FEEDBACK_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// W95_feedback.h : header file
//

extern void initFeedback(wchar_t *Label, wchar_t *message);
extern void setCurrentFeedbackMessage(int perc);
extern void deleteFeedback(void);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_W95_FEEDBACK_H__B5D78A9F_1F9D_40FA_B07C_5D6CE85815CF__INCLUDED_)
