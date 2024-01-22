/**********************************************************************
	"Copyright 1990-2020 Brian MacWhinney. Use is subject to Gnu Public License
	as stated in the attached "gpl.txt" file."
*/

#ifndef __CONSTR_HPP__
#define __CONSTR_HPP__

class CRule {
public:
	mortag*	tail;
	int		ntail;
	mortag*	head;
	int		nhead;
	CRule*	next;
	int		section;

	CRule();
	void init(mortag* h, int nh, mortag* t, int nt);
	void print(FILE* out);
	void delete_rule();
	void delete_rule_internal();
};

class NTree {
public:
	mortag*	val;
	mortag* mac;
	NTree*	down;
	NTree*	next;
	NTree*	prev;

	void next_print(FILE* out, int level);
	void down_print(FILE* out, int level);
	void nexttop_print(FILE* out);
	void print(FILE* out);
	void l_print(FILE* out);

	int compare_with_rule(CRule* r);
	NTree();
	void init(mortag* m, int nm);
	CRule* find(int section);
};

#endif
