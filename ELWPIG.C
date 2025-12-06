/***************************************************************************
 * ELWPIG.C - Pig v1.02                                                    *
 *                                                                         *
 * Originally by Yannick Tessier, Tessier Technologies.                    *
 * Copyright (c) 2005 Rick Hadsall. All Rights Reserved.                   *
 *                                                                         *
 ***************************************************************************/
#include "gcomm.h"
#include "majorbbs.h"
#include "elwpig.h"
 
#define VERSION  "1.02"                 /*  Version number                 */

GBOOL piggam(VOID);                     /*  Input Routine                  */

INT  caschk(VOID);                      /*  Case 0 Function call           */

VOID chksta(VOID),                      /*  Check Start Routine            */
     rstvbls(VOID),                     /*  Reset variables                */
     byepig(VOID),                      /*  Hangup Routine if Called       */
     dwnpig(VOID),                      /*  Board down routine             */
     pigdis(INT type),                  /*  Display Messages to Users      */
     loapla(VOID),                      /*  Load Player                    */
     welcom(VOID),                      /*  Welcome Message                */
     iniper(VOID),                      /*  Initialize Person              */
     showho(VOID),                      /*  Shows who is present in Tele   */
     plagam(VOID);                      /*  RTKICK during play             */

static
VOID pigwhisp(VOID),                    //  Whisper in pig
     pigmem(VOID),                      /*  Memory Allocation (INIT)       */
     inivar(VOID),                      /*  Initialize Variables (INIT)    */
     extusr(VOID),                      /*  Exit User Routine              */
     joigam(VOID),                      /*  Enter a user into the game     */
     quigam(VOID),                      /*  Remove a user from the game    */
     calctp(VOID),                      /*  Calculate the Total Players    */
     prnper(VOID),                      /*  Notify Players whos turn it is */
     figfst(VOID),                      /*  Figure out First Person        */
     prnpig(VOID),                      /*  Print to Users EXCEPT CURPLA   */
     plarol(VOID),                      /*  Users roll dice                */
     ressco(VOID),                      /*  Reset all Scores to 0          */
     nxtnum(VOID),                      /*  Get next user in line          */
     stanow(VOID),                      /*  User wants to stay his turn    */
     scanme(VOID),                      /*  Scan Function for users        */
     draw_dice(INT dice_1,INT dice_2),  /*  Print Dice Function            */
     updsta(VOID),                      /*  Update Players Stats           */
     topten(VOID),                      /*  Users top ten listing          */
     restuf(VOID);                      /*  Reset Playing Variables        */

static 
INT  chkfin(VOID),                      /*  Check to see if user wins      */
     piggetusr(VOID),                   /*  Try to figure a USERID         */
     ttirnd(INT what);                  /*  Random Function (1-what)       */

static VOID initrnd(VOID);              /*  Initialize/seed random gen     */
static INT getrnd(INT lo, INT hi);      /*  Get a random from lo to hi     */
#ifdef __BUILDV10MODULE
static INT random(INT hi);              /*  Get a random from 0 to hi-1    */
#endif

static HMCVFILE    pigmsg;              /*  Message File                        */ 
DFAFILE *pigdat;                        /*  PIG Database                        */

INT PIGSTT;                        /*  module state (number) for PIG       */

struct module elwpig={             /*  module interface block              */
     "",                           /*  Main menu description               */
     NULL,                         /*  user logon supplemental routine     */
     piggam,                       /*  input routine if selected           */
     dfsthn,                       /*  status-input routine if selected    */
     NULL,                         /*  INJOTH routine if selected          */
     NULL,                         /*  Logoff routine if selected          */
     byepig,                       /*  hangup (lost carrier) routine       */
     NULL,                         /*  midnight cleanup routine            */
     NULL,                         /*  delete-account routine              */
     dwnpig                        /*  finish-up (sys shutdown) routine    */
};

struct pigusr {                    /*  User Structure for PIG              */
     CHAR userid[30];              /*  User ID of current user             */ // 000-029
#ifndef __BUILDV10MODULE
     CHAR pack1[2];
#endif
     //pack2
     INT  wining;                  /*  Amount of wins user has             */ // 032-035
     INT  wanpla;                  /*  User wants to play?                 */ // 036-039
     INT  playin;                  /*  User is Playing or not?             */ // 040-043
     INT  trnsco;                  /*  Score on your particular turn       */ // 044-047
     INT  totsco;                  /*  Total Score for entire round        */ // 048-051
     INT  dicdis;                  /*  Display the LARGE Dice or not       */ // 052-055
     CHAR spare[100-56];           /*  Spare space for upgrades            */ // 056-099
} *pigusr,*pigptr,pigpla;

static struct sysvab {             /*  System Variables for PIG            */
     INT  dices1;                  /*  Value of Die #1                     */
     INT  dices2;                  /*  Value of Die #2                     */
     INT  gameip;                  /*  Game in progress flag               */   
     INT  platrn;                  /*  Usrnum of Player who has the dice   */
     INT  totpla;                  /*  Total Number of Players             */
     INT  laspla;                  /*  Last players turn                   */
     INT  scores;                  /*  Score needed to win (CNF CONFIG)    */
     INT  onepla;                  /*  If a player is playing vs computer  */
     INT  myturn;                  /*  Computer Turn Flag                  */
     INT  trnsco;                  /*  Mr. Pigs Turn Score                 */
     INT  totsco;                  /*  Mr. Pigs Total score                */
} pigvar;


void EXPORT
init__elwpig(VOID)              /*  Initialization Routine (Board up)   */
{
  pigmsg=opnmsg("ELWPIG.MCV");
  setmbk(pigmsg);

  stzcpy(elwpig.descrp,gmdnam("ELWPIG.MDF"),MNMSIZ);
  PIGSTT=register_module(&elwpig);

  pigdat=dfaOpen("ELWPIG.DAT",sizeof(struct pigusr),NULL);

  pigmem();                /*  Initialize User Structure           */
  inivar();                /*  Initialize Variables                */
  initrnd();               /*  Seed random number generator        */

  rtkick(45, chksta);
  
  shocst(spr("ELW Pig v%s",VERSION),"(C) Copyright 2025 Elwynor Technologies - www.elwynor.com");
}

INT
caschk(VOID)
{
               loapla();
               iniper();
               prfmsg(GOTHER,usaptr->userid);
               pigdis(1);
               welcom();
               return(1);
}

VOID
loapla(VOID)
{
     dfaSetBlk(pigdat);

     if(!dfaQueryEQ(usaptr->userid,0)) {
       setmem(pigusr[usrnum].userid,30,0);
       strcpy(pigusr[usrnum].userid,usaptr->userid);
       pigusr[usrnum].wanpla=0;
       pigusr[usrnum].playin=0;
       pigusr[usrnum].trnsco=0;
       pigusr[usrnum].totsco=0;
       pigusr[usrnum].wining=0;
       dfaInsert(&pigusr[usrnum]);
     }
     dfaAbsRec(&pigusr[usrnum],0);
}

VOID                     /*  Welcome Messages                    */
welcom(VOID)
{
     prfmsg(WELCOM);
     showho();
     prfmsg(usrptr->substt=TELECO);
}

VOID                     /*  Initialize User Variables           */
iniper(VOID)
{
     btumil(usrnum,150);
     btupmt(usrnum,':');
     pigptr->wanpla=0;
     pigptr->playin=0;
     pigptr->totsco=0;
     pigptr->trnsco=0;
}

VOID
dwnpig(VOID)
{
	clsmsg(pigmsg);
	dfaClose(pigdat);
}


GBOOL
piggam(VOID)              /*  Input Routine                       */
{

	setmbk(pigmsg);
	pigptr=&(pigusr[usrnum]);

	switch(usrptr->substt) {
		case 0:
			if(!caschk()) return(0);
			break;

		case TELECO:
		  if(margc==0) {
		    showho();
		    prfmsg(TELECO);
		    break;
		  }
		  if (margc>=2 && margv[0][0]=='/') {
			  pigwhisp();
			  break;
		  }
		  if(margc==1) {
		    if(sameas("X",margv[0])) {
			 usrptr->substt=9999;
			 extusr();
			 return(0);
		    }

		    if(sameas("SCORE",margv[0])) {
			 prfmsg(SCORED,pigptr->wining);
			 break;
		    }
		    
		    if(sameas("JOIN",margv[0])) {
			 joigam();
			 break;
		    }
		    
		    if(sameas("HELP",margv[0]) || sameas("?",margv[0])) {
			 prfmsg(HELPSC,pigvar.scores,pigvar.scores);
			 break;
		    }
 
		    if(sameas("QUIT",margv[0])) {
			 quigam();
			 break;
		    }
 
		    if(sameas("ROLL",margv[0])) {
			 plarol();
			 return(1);
		    }
 
		    if(sameas("STAY",margv[0])) {
			 stanow();
			 return(1);
		    }
		    
		    if(sameas("HIGH",margv[0])) {
			 topten();
			 break;
		    }
 
		    if(sameas("SCAN",margv[0])) {
			 scanme();
			 break;
		    }
		  }
		  if(margc==2) {
		    if(sameas("DICE",margv[0]) && sameas("ON",margv[1])) {
			 prfmsg(DICEON);
			 pigptr->dicdis=1;
			 break;
		    }
		    if(sameas("DICE",margv[0]) && sameas("OFF",margv[1])) {
			 prfmsg(DICEOF);
			 pigptr->dicdis=0;
			 break;
		    }
		  }
		  rstrin();
		  pigdis(2);
		  break;
 
	}
	outprf(usrnum);
	return(1);
}
 
VOID
byepig(VOID)
{
	pigptr=&(pigusr[usrnum]);
	setmbk(pigmsg);
	 

	if(usrptr->state==PIGSTT) {
	  usrptr->substt=9999;
	  prfmsg(BYEUSR,usaptr->userid);
	  pigdis(1);
	  quigam();
	  clrprf();
	  updsta();
	}
}
 
 
static VOID                     /*  Initialize User Structure           */
pigmem(VOID)
{
/*	
    if ((pigusr = (struct pigusr*)getml((LONG)nterms * sizeof(struct pigusr))) == NULL) {
	  catastro("INIEML: NOT ENOUGH MEMORY");
	}
*/
	pigusr = (struct pigusr*)alcblok((USHORT)nterms, sizeof(struct pigusr));
	if (!pigusr) {
		catastro("ELWPIG: NOT ENOUGH MEMORY");
	}
	else {
		for (usrnum = 0; usrnum < nterms; usrnum++)
			setmem(&pigusr[usrnum], sizeof(struct pigusr), 0);
	}

}
 
static VOID                     /*  Initialize System Variables         */
inivar(VOID)
{
	pigvar.dices1=1;
	pigvar.dices2=1;
	pigvar.gameip=0;
	pigvar.platrn=-1;
	pigvar.scores=numopt(FINSCO,10,1000);
	pigvar.onepla=0;
}
 
VOID                     /* Display Messages to Users          */
pigdis(INT type)
{
	INT j = 0;
 
	switch (type) {
	  case 2:
	    j=0;
	    prfmsg(DISPLA,usaptr->userid,margv[0]);
	    break;
	}
 
	prf("[35m");
	for (othusn=0;othusn<nterms;othusn++) {
	  othusp=usroff(othusn);
	  switch (type) {
							 /* Messages going to all but usrnum  */
	    case 1:
		 if (othusp->state==PIGSTT && othusp->substt==TELECO && othusn!=usrnum) {
		   outprf(othusn);
		 }
		 break;
							 /* Regular From: Message too all      */
	    case 2:
		 if (othusp->state==PIGSTT && othusp->substt==TELECO && othusn!=usrnum) {
		   j=1;
		   outprf(othusn);
		 }
		 break;
							 /* Messages Sent to Everyone          */
	    case 3:
		 if (othusp->state==PIGSTT && othusp->substt==TELECO) {
		   outprf(othusn);
		 }
		 break;
	  }
	}
	clrprf();
 
	switch (type) {
	  case 2:
	    if (j==0){
		 prfmsg(ONYHRE);
	    }else{
		 prfmsg(MESENT);
	    }
	    break;
	}
}
 
VOID                             /* Shows who is present in Horse Tele */
showho(VOID)
{
 
	INT j=0;
	INT i;
	setmbk(pigmsg);
 
	for(i=0;i<nterms;i++) {
		if (usroff(i)->state==PIGSTT && usroff(i)->substt==TELECO && i!=usrnum) {
			othuap=uacoff(i);
			j++;
			if (j>1){
				prfmsg(WHOHR2,othuap->userid);
			}else{
				prfmsg(WHOHR1,othuap->userid);
			}
		}
	}
 
	if (j==0) {
		prfmsg(ONYHRE);
	} else {
		prfmsg(PERIOD);
	}
 
}
 
static VOID                     /*  Exit User Routine                   */
extusr(VOID)
{
	prfmsg(EXTUSR,usaptr->userid);
	pigdis(1);
	btuech(usrnum,1);
	prfmsg(PIGEXT);
	btupmt(usrnum,'\0');
	outprf(usrnum);
	quigam();
	updsta();
	clrprf();
}
 
static VOID                        /*  Enter a user into the game          */
joigam(VOID)
{
	if(pigptr->wanpla || pigptr->playin) {
	  prfmsg(ALRPLA);
	  return;
	}
 
	pigptr->wanpla=1;
	prfmsg(SIGNUP,usaptr->userid);
	pigdis(1);
	prfmsg(ALLSIG);
 
}
 
static VOID
quigam(VOID)
{
	INT isplay;

	if(!pigptr->wanpla && !pigptr->playin) {
	  prfmsg(NOTENT);
	  return;
	}
	isplay=pigptr->playin;
	pigptr->playin=0;
	pigptr->wanpla=0;

	if (isplay==0) return;
	if(pigvar.platrn==usrnum) {
	    if(pigvar.onepla==1) {
		 pigvar.myturn=1;
		 pigvar.platrn=-3;
		 chkfin();
		 prfmsg(TAKOUT);
		 return;
	    }
	    pigvar.laspla=usrnum;
	    nxtnum();
	    if(pigvar.platrn==pigvar.laspla) {
		 chkfin();
		 prfmsg(TAKOUT);
		 return;
	    }
	} else {
	  if(pigvar.onepla==1) {
	    pigvar.myturn=1;
	    pigvar.platrn=-3;
	    chkfin();
	    prfmsg(TAKOUT);
	    return;
	  }
	}
	prfmsg(TAKOUT);
}
 
VOID
chksta(VOID)
{
	setmbk(pigmsg);

	if (!pigvar.gameip) {
		pigvar.gameip=1;
		for (usrnum=0;usrnum<nterms;usrnum++) {
			if(usroff(usrnum)->state==PIGSTT && pigusr[usrnum].wanpla==1) {
				pigusr[usrnum].wanpla=0;
				pigusr[usrnum].playin=1;
			}
		}
		calctp();
		if (pigvar.totpla==0) {
			pigvar.gameip=0;
			rtkick(45,chksta);
			return;
		}
		if (pigvar.totpla==1) {
			pigvar.onepla=1;
			pigvar.myturn=0;
		}
		else {
			pigvar.onepla=0;
			pigvar.myturn=0;
		}
		prfmsg(NEWGAM);
		pigdis(3);
		ressco();
		figfst();
		prnper();
		rtkick(7, plagam);
	}
	else rtkick(45,chksta);
}
 
static VOID
calctp(VOID)
{
	INT i;

	pigvar.totpla=0;
	for (i=0;i<nterms;i++) {
		if (usroff(i)->state==PIGSTT && pigusr[i].playin==1) {
			pigvar.totpla+=1;
		}
	}
}
 
static VOID
figfst(VOID)
{
	for(usrnum=0;usrnum<nterms;usrnum++) {
	  if(usroff(usrnum)->state==PIGSTT && pigusr[usrnum].playin==1) {
	    pigvar.platrn=usrnum;
	    pigvar.laspla=-1;
	  }
	}
}
 
static VOID
prnper(VOID)
{
	if(pigvar.myturn==1) {
	  prfmsg(HISTRN,"Mr. Pig");
	  prnpig();
	  return;
	}
	
	othuap=uacoff(pigvar.platrn);
	prfmsg(HISTRN,othuap->userid);
	prnpig();
	
	prfmsg(YOUTRN);
	outprf(pigvar.platrn);
	clrprf();
}
 
VOID
plagam(VOID)
{
	setmbk(pigmsg);
	
	calctp();

	if (pigvar.platrn==-1 || pigvar.totpla==0) {
		rstvbls();
		rtkick(45, chksta);
		return;
	}

	if (pigvar.myturn==1) {
		if (ttirnd(5)==1 && pigvar.trnsco!=0) {
			pigvar.totsco+=pigvar.trnsco;
			prfmsg(YOUSTA,"Mr. Pig",pigvar.trnsco,pigvar.totsco);
			pigdis(3);
			nxtnum();
			pigvar.trnsco=0;
			rtkick(3, plagam);
			return;
		}
		pigvar.dices1=ttirnd(6);
		pigvar.dices2=ttirnd(6);

		if (pigvar.dices1+pigvar.dices2==7) {
			pigvar.trnsco=0;
			prfmsg(HELOST,"Mr. Pig","His");
			pigdis(3);
			nxtnum();
			rtkick(3,plagam);
			return;
		}
		else pigvar.trnsco+=pigvar.dices1+pigvar.dices2;

		prfmsg(HEROLL,"Mr. Pig",pigvar.dices1,pigvar.dices2,"His",pigvar.trnsco);
		pigdis(3);
		chkfin();
		rtkick(3,plagam);
		return;
	}

	if (pigvar.platrn==pigvar.laspla) {
		prfmsg(NUDGEY);
		outprf(pigvar.platrn);
		clrprf();
	}
	pigvar.laspla=pigvar.platrn;
	rtkick(7, plagam);
}

static VOID
prnpig(VOID)
{
	INT i;
 
	for(i=0;i<nterms;i++) {
	  if(usroff(i)->state==PIGSTT && i!=pigvar.platrn) {
	    outprf(i);
	  }
	}
	clrprf();
}
 
static VOID
plarol(VOID)
{
	if(pigvar.platrn!=usrnum) {
	  prfmsg(NOTTRN);
	  outprf(usrnum);
	  return;
	}
 
	if(chkfin()==1) return;

	pigvar.dices1=ttirnd(6);
	pigvar.dices2=ttirnd(6);
 
	draw_dice(pigvar.dices1,pigvar.dices2);

	if(pigvar.dices1+pigvar.dices2==7) {
	  pigptr->trnsco=0;
	  prfmsg(HELOST,usaptr->userid,(usaptr->sex == 'M' ? "His" : "Her"));
	  pigdis(1);
	  pigvar.laspla=usrnum;
	  prfmsg(YOLOST);
	  outprf(usrnum);
	  nxtnum();
	  return;
	} else {
	  pigptr->trnsco+=pigvar.dices1+pigvar.dices2;
	}
 
	prfmsg(HEROLL,usaptr->userid,pigvar.dices1,pigvar.dices2,(usaptr->sex == 'M' ? "His" : "Her"),pigptr->trnsco);
	pigdis(1);
	prfmsg(YOUROL,pigvar.dices1,pigvar.dices2);
	prfmsg(TOTSCO,pigptr->trnsco);
	pigvar.laspla=-1;
	outprf(usrnum);
	chkfin();
	return;
}

static VOID initrnd(VOID)                 /* initialize random number seed        */
{
#ifdef __BUILDV10MODULE  
	srand((unsigned)time(NULL)); // BBSV10
#else
	randomize(); // WG32, MBBS6/WG1/WG2
#endif
}

static INT getrnd(INT lo, INT hi)         /* Returns random # between lo and hi   */
{
	if ((hi < 0) || (lo < 0)) {
		return(0);
	}

	if (hi < lo) {
		INT tmp = lo;
		lo = hi;
		hi = tmp;
	}

	if (hi == lo) {
		return lo;
	}

#ifdef __BUILDV10MODULE
	return(lo + rand() % (hi - lo + 1)); // BBSV10
#else
	return (random(hi - lo + 1) + lo); //WG32, MBBS6/WG1/WG2
#endif
}

#ifdef __BUILDV10MODULE
static INT random(INT hi)
{
	if (hi <= 0) {
		return 0;
	}
	return getrnd(0, hi - 1); // replicate the behavior of Borland's random(int hi); 
}
#endif

static INT
ttirnd(INT what)
{
  return(random(what)+1);
}
 
static VOID
ressco(VOID)
{
	for(usrnum=0;usrnum<nterms;usrnum++) {
		pigusr[usrnum].trnsco=0;
		pigusr[usrnum].totsco=0;
	}
	pigvar.trnsco=0; 
	pigvar.totsco=0;
}

static VOID
nxtnum(VOID)
{
	INT i;
	if(pigvar.onepla==1) pigvar.laspla=pigvar.platrn;
	if(pigvar.platrn<0) pigvar.platrn=-1;

	for(i=pigvar.platrn+1;i<nterms;i++) {
	  if(usroff(i)->state==PIGSTT && pigusr[i].playin==1) {
	    pigvar.platrn=i;
	    pigvar.myturn=0;
	    prnper();
	    return;
	  }
	}
 
	for(i=0;i<nterms;i++) {
	  if(usroff(i)->state==PIGSTT && pigusr[i].playin==1) {
	    pigvar.platrn=i;
	    if(pigvar.platrn==pigvar.laspla && pigvar.onepla==1 && pigvar.myturn==0) {
		 pigvar.platrn=-2;
		 pigvar.myturn=1;
	    } else {
		 pigvar.myturn=0;
	    }
	    prnper();
	    return;
	  }
	}

}

static VOID
stanow(VOID)
{
	if(pigvar.platrn!=usrnum) {
	  prfmsg(NOTTRN);
	  outprf(usrnum);
	  return;
	}

	if(chkfin()==1) return;

	pigptr->totsco+=pigptr->trnsco;
	prfmsg(YOUSTA,usaptr->userid,pigptr->trnsco,pigptr->totsco);
	pigdis(1);
	prfmsg(YOUCOL,pigptr->trnsco);
	prfmsg(TOTALS,pigptr->totsco);
	outprf(usrnum);
	nxtnum();
	pigptr->trnsco=0;
}

 
static INT
chkfin(VOID)
{
	calctp();
 
	if (pigvar.onepla==1 && pigvar.myturn==1) {
		if (pigvar.totsco+pigvar.trnsco>=pigvar.scores) {
			rstvbls();
			prfmsg(YOUWIN,"Mr. Pig",pigvar.scores);
			pigdis(3);
			return(1);
		}
		if (pigvar.platrn==-3) {
			rstvbls();
			prfmsg(DEFAUL,"Mr. Pig");
			pigdis(3);
			return(1);
		}
	}

	if (pigptr->totsco+pigptr->trnsco>=pigvar.scores) {
		pigptr->wining+=1;
		updsta();
		rstvbls();
		prfmsg(YOUWIN,usaptr->userid,pigvar.scores);
		pigdis(1);
		prfmsg(YOUWON,pigvar.scores);
		outprf(usrnum);
		clrprf();
		return(1);
	}
	if (pigvar.onepla==0 && pigvar.totpla==1) {
		pigptr->wining+=1;
		updsta();
		rstvbls();
		prfmsg(DEFAUL,usaptr->userid);
		pigdis(1);
		prfmsg(DEFAUT);
		outprf(usrnum);
		clrprf();
		return(1);
	}
	return(0);
}

static VOID
scanme(VOID)
{
	CHAR wait,play,turn;
 
	prfmsg(SCANME);
	for(othusn=0;othusn<nterms;othusn++) {
	  othusp=usroff(othusn);
	  othuap=uacoff(othusn);
	  wait=' ';
	  play=' ';
	  turn=' ';
	  if(othusp->state==PIGSTT) {
	    if(pigusr[othusn].wanpla==1) wait='*';
	    if(pigusr[othusn].playin==1) play='*';
	    if(pigvar.platrn==othusn) turn='*';
	    othuap=uacoff(othusn);
	    prfmsg(SCANEM,othuap->userid,wait,play,turn,pigusr[othusn].totsco+pigusr[othusn].trnsco);
	  }
	}

	if(pigvar.onepla==1) { 
	  turn=' ';
	  if(pigvar.myturn==1) turn='*';
	  prfmsg(SCANEM,"Mr. Pig",' ','*',turn,pigvar.totsco+pigvar.trnsco);
	}
}
 
static VOID
draw_dice(INT dice_1,INT dice_2)
{
 
	INT i, x, y[2] = { 0,0 };
	CHAR row[2][9] = { {0},{0} };
 
    if(!pigptr->dicdis) return;
    
    setmem(row[0],9,0);
    setmem(row[1],9,0);
    y[0]=dice_1;y[1]=dice_2;
 
    for(i=0;i<2;i++) {
	 for(x=0;x<9;x++) {
	   row[i][x]=' ';
	 }
    }
 
    for (x=0;x<2;x++) {
 
	 if (y[x] == 4 || y[x] == 5 || y[x] == 6)
	   row[x][0]='þ';
	 if (y[x] == 2 || y[x] == 3 || y[x] == 4 || y[x] == 5 || y[x] == 6)
	   row[x][2]='þ';
	 if (y[x] == 6)
	   row[x][3]='þ';
	 if (y[x] == 1 || y[x] == 3 || y[x] == 5)
	   row[x][4]='þ';
	 if (y[x] == 6)
	   row[x][5]='þ';
	 if (y[x] == 2 || y[x] == 3 || y[x] == 4 ||	y[x] == 5 || y[x] == 6)
	   row[x][6]='þ';
	 if (y[x] == 4 || y[x] == 5 || y[x] == 6)
	   row[x][8]='þ';
    }
 
    prfmsg(DICE,row[0][0],row[0][1],row[0][2],row[1][0],
			 row[1][1],row[1][2],row[0][3],row[0][4],
			 row[0][5],row[1][3],row[1][4],row[1][5],
			 row[0][6],row[0][7],row[0][8],row[1][6],
			 row[1][7],row[1][8]);
    
    outprf(usrnum);
}

static VOID
updsta(VOID)
{
	dfaSetBlk(pigdat);

	if(dfaQueryEQ(usaptr->userid,0)) {
	  dfaAbsRec(NULL,0);
	  dfaUpdate(&pigusr[usrnum]);
	}

}


static VOID
topten(VOID)
{
	   INT i;
	   dfaSetBlk(pigdat);

	   prfmsg(TOPTEN);
	   
	   if(dfaQueryHI(1)) {
		dfaAbsRec(&pigpla,1);
		prfmsg(TOPTN1,pigpla.userid,pigpla.wining);
		i=1;
		while(dfaQueryPR()) {
		  dfaAbsRec(&pigpla,1);
		  prfmsg(TOPTN1,pigpla.userid,pigpla.wining);
		  i++;
		  if(i==10) {
		    break;
		  }
		}
	   }
}

static VOID
restuf(VOID)
{
	INT i;

	for(i=0;i<nterms;i++) {
		if (pigusr[i].playin==1) {
		  pigusr[i].playin=0;
		  pigusr[i].wanpla=1;
		  pigusr[i].totsco=0;
		  pigusr[i].trnsco=0;
		}
	}
}

static VOID
pigwhisp(VOID)                //  Whisper in pig
{
	INT count;               //  Just a counter

	count=piggetusr();
	if (count==9999) {
		prfmsg(NOUSER);
		return;
	}
	if (count==9998) {
		prfmsg(MORLET);
		return;
	}
	rstrin();
	prfmsg(WHISPFR, usaptr->userid, input);
	outprf(count);
	clrprf();
	prfmsg(WHISENT);

}

static INT
piggetusr(VOID)               //  Try to figure a USERID
{
	CHAR usr[30] = { 0 };    //  User handle trying to find
	INT count;               //  Just a counter
	INT count2;              //  Just a counter
	INT many;                //  How many handles have this user
	INT line;                //  What line user is on
	INT start;               //  Starting place of the words
	INT tmpstatus;

	setmem(usr, 30, 0);
	tmpstatus=9999;

	for (count=0; count<margc; count++) {
		if (strlen(usr)+strlen(margv[count])>29) return(tmpstatus);
		if (count!=0) strcat(usr, " ");
		if (count==0) strcat(usr, margv[count]+1);
		else          strcat(usr, margv[count]);
		start=count+1;
		many=0;
		line=9999;
		for(count2=0; count2<nterms; count2++){
			if (usroff(count2)->state!=PIGSTT) continue;
			if (sameto(usr, uacoff(count2)->userid)) {
				many+=1;
				line=count2;
				if (sameas(usr, uacoff(count2)->userid)) {
					if (start==margc) return(9999);
					rstrin();
					strcpy(input, margv[start]);
					parsin();
					return(count2);
				}
			}
		}
		if (many==1)     tmpstatus=line;
		else if (many>1) tmpstatus=9998;
		if (tmpstatus<nterms && many==1) {
			if (start==margc) return(9999);
			rstrin();
			strcpy(input, margv[start]);
			parsin();
			return(tmpstatus);
		}
	}
	return(tmpstatus);
}

VOID
rstvbls(VOID)
{
	pigvar.gameip=0;
	pigvar.platrn=-1;
	pigvar.laspla=-1;
	pigvar.myturn=0;
	pigvar.onepla=0;
	restuf();
}
