#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "header.h"
#include "sysdep.h"
#include "graphics.h"

char fktext [12][64];

static char history[MAXHIST][MAXLINE];
static int act_history=0,	/* number of actual entries in history */
           hist=0;			/* current line */
extern int outputing;

static void cpy (char *dest, char *source)
{	memmove(dest,source,strlen(source)+1);
}

static void push_in_history (char *s)
{	int i;

	/* do not push empty lines */
	char *p=s;
	while (*p==' ' || *p=='\t') p++;
	if (!strlen(p)) return;
	
	/* do not push a line in history, if it is already the last
	   one pushed in */
	if (strcmp(s,history[act_history-1])==0) return;
	
	/* make room if necessary*/
	if (act_history>=MAXHIST) {
		for (i=0; i<MAXHIST-1; i++) strcpy(history[i],history[i+1]);
		act_history=MAXHIST-1;
		hist--; if (hist<0) hist=0;
	}
	
	/* push the line in history */
	strcpy(history[act_history],s);
	if (hist==act_history) hist++;
	act_history++;
}

static void prompt (void)
{	if (!outputing) gprint("\n>");
	if (!udf) output(">");
	else output("udf>");
}

static void left (int n)
{	int i;
	for (i=0; i<n; i++) move_cl();
}
/*
static void right (int n)
{	int i;
	for (i=0; i<n; i++) move_cr();
}
*/
static void fkinsert (int i, int *pos, char *s)
{	char *p;
	p=fktext[i];
	if (strlen(s)+strlen(p)>=MAXLINE-2) return;
	cpy(s+*pos+strlen(p),s+*pos); memmove(s+*pos,p,strlen(p));
	gprint(p); *pos+=strlen(p);
}

static char helpstart[MAXNAME];
static char helpextend[16][MAXNAME];
static int helpn=0,helpnext=0,helphist=-1;
int extend(char* start, char extend[16][MAXNAME]);

static void edithelp (char *s, int *pos, int *shorter)
/* extend the command at cursor position */
{	char *start,*end,*p;
	int i,l;
	/* search history */
	l=strlen(s);
	helphist=-1;
	for (i=act_history-1; i>=0; i--)
		if (strncmp(history[i],s,l)==0)
		{	helphist=i;
			break;
		}
	/* not the beginning of a command in history,
	   maybe a command, builtin or user defined function name,
	   so find the extent of the name */
	helpn=0;
	start=end=p=s+(*pos);
	while (start>s && (isalpha(*(start-1)) || isdigit(*(start-1))))
		start--;
	while (isdigit(*start)) start++;
	while (isalpha(*end) || isdigit(*end)) end++;
	if (start>s+(*pos) || start>=end || (end-start)>MAXNAME-1) return;
	/* we have an extent */
	while (p<end) { move_cr(); (*pos)++; p++; }
	memmove(helpstart,start,end-start);
	helpstart[end-start]=0;
	helpn=extend(helpstart,helpextend);
	helpnext=0;
}

scantyp edit (char *s)
{	int pos, shorter;
	scantyp scan;
	char ch,chs[2]="a",*p=0;
	s[0]=0; pos=0;
	edit_on();
	prompt();
	helpn=0; helphist=-1; //hist=act_history;
	while (1){
		ch=wait_key(&scan);
		if (scan!=help && helpnext<helpn) helpn=0;
		if (scan==enter || scan==eot) break;
		shorter=0;
		switch (scan) {
			case key_none :	/* standard characters */
				if (isprint(ch) && strlen(s)<MAXLINE-2) {
					cpy(s+pos+1,s+pos);
					s[pos]=ch; pos++;
					chs[0]=ch;
					cursor_off(); gprint(chs);
				}
				goto cont;
			case cursor_left :
				if (pos) { pos--; move_cl(); }; continue;
		    case cursor_right : /* cursor right */
		    	if (s[pos]) { pos++; move_cr(); }; continue;
			case word_right : /* a word to the right */
			{	while (s[pos] && s[pos]!=' ')
				{	pos++; move_cr(); }
				while (s[pos]==' ')
				{	pos++; move_cr(); }
				continue;
			}
			case word_left : /* a word to the right */
			{	if (pos) { pos--; move_cl(); }
				while (pos && s[pos]==' ')
				{	pos--; move_cl(); }
				while (pos && s[pos]!=' ')
				{	pos--; move_cl(); }
				if (pos) { pos++; move_cr(); }
				continue;
			}
		    case backspace :
		    	if (pos) {
		    		pos--;
		    		cpy(s+pos,s+pos+1); 
		    		move_cl(); shorter=1;
		    	}
		    	break;
		    case delete :
		    	if (s[pos]) {
		    		cpy(s+pos,s+pos+1); shorter=1;
		    	}
		    	break;
		    case cursor_up :
		    	if (hist) {
		    		hist--; strcpy(s,history[hist]); shorter=1;
		    	}
		    	cursor_off(); left(pos); pos=0; goto cont;
		    case cursor_down :
		    	hist++;
		    	if (hist<act_history) {
		    		strcpy(s,history[hist]); shorter=1;
		    	} else {
		    		hist=act_history; s[0]=0; shorter=1;
		    	}
		    	cursor_off(); left(pos); pos=0; goto cont;
		    case clear_home :
		    case escape :
		    	cursor_off();
		    	left(pos); pos=0; s[0]=0;
		    	shorter=1; hist=act_history;
		    	goto cont;
		    case line_end :
		    	while (s[pos]) { pos++; move_cr(); }
		    	continue;
		    case line_start :
		    	left(pos); pos=0;
		    	continue;
		    case switch_screen :
		    	edit_off(); show_graphics(); edit_on(); break;
		    case help :
		    	if (helpnext>=helpn && helphist<0) {
		    		edithelp(s,&pos,&shorter);
		    		p=0;
		    	}
		    	if (helphist>=0) {	/* completion found in history */
		    		cursor_off();
		    		strcpy(s,history[helphist]);
		    		hist=helphist;
		    		p=s+pos;
		    		gprint(p);
		    		pos+=strlen(p);
		    		shorter=1;
		    		helphist=-1; goto cont;
		    	} else if (helpnext<helpn) {	/* completion is a builtin/command/udf */
		    		cursor_off();
		    		if (p) {					/* erase preceding extension (not the first time) */
		    			left(strlen(p));
		    			pos-=strlen(p);
		    			cpy(s+pos,s+pos+strlen(p));
		    		}
		    		p=helpextend[helpnext++];
		    		if (strlen(s)+strlen(p)>=MAXLINE-2) {
		    			p=0; helpn=0; break;
		    		}
					cpy(s+pos+strlen(p),s+pos);	/* get the following extension in the list */
					memmove(s+pos,p,strlen(p));
					gprint(p);
					pos+=strlen(p);
					shorter=1;
					goto cont;
		    	}
		    	break;
		    case fk1 : fkinsert(0,&pos,s); break;
		    case fk2 : fkinsert(1,&pos,s); break;
		    case fk3 : fkinsert(2,&pos,s); break;
		    case fk4 : fkinsert(3,&pos,s); break;
		    case fk5 : fkinsert(4,&pos,s); break;
		    case fk6 : fkinsert(5,&pos,s); break;
		    case fk7 : fkinsert(6,&pos,s); break;
		    case fk8 : fkinsert(7,&pos,s); break;
		    case fk9 : fkinsert(8,&pos,s); break;
		    case fk10 : fkinsert(9,&pos,s); break;
		    case fk11 : fkinsert(10,&pos,s); break;
		    case fk12 : fkinsert(11,&pos,s); break;
		    default:
		    	continue;
		}
		cursor_off();

		cont: gprint(s+pos);
		if (shorter) clear_eol();
		left(strlen(s+pos));
		cursor_on();
	}
	push_in_history(s);
	if (outfile) {
		fprintf(outfile,"%s",s);
	}
	output("\n");
	edit_off();
	return scan;
}
