
#if !defined (cray) && !(defined (_WIN32) && !defined (__GNUC__))
#if defined (VMS) || ((defined (hpux) || defined (aix)) && !defined(NAGware))

#define GOPKS	gopks
#define GCLKS	gclks
#define GOPWK	gopwk
#define GCLWK	gclwk
#define GACWK	gacwk
#define GDAWK	gdawk
#define GCLRWK	gclrwk
#define GRSGWK  grsgwk
#define GUWK	guwk
#define GSDS	gsds
#define GESC	gesc
#define GMSG	gmsg
#define GPL	gpl
#define GPM	gpm
#define GTX	gtx
#define GTXS	gtxs
#define GFA	gfa
#define GCA	gca
#define GSPLI	gspli
#define GSLN	gsln
#define GSLWSC	gslwsc
#define GSPLCI	gsplci
#define GSPMI	gspmi
#define GSMK	gsmk
#define GSMKSC	gsmksc
#define GSPMCI	gspmci
#define GSTXI	gstxi
#define GSTXFP	gstxfp
#define GSCHXP	gschxp
#define GSCHSP	gschsp
#define GSTXCI	gstxci
#define GSCHH	gschh
#define GSCHUP	gschup
#define GSTXP	gstxp
#define GSTXAL	gstxal
#define GSFAI	gsfai
#define GSFAIS	gsfais
#define GSFASI	gsfasi
#define GSFACI	gsfaci
#define GSASF	gsasf
#define GSCR	gscr
#define GSWN	gswn
#define GSVP	gsvp
#define GSELNT	gselnt
#define GSCLIP	gsclip
#define GSWKWN	gswkwn
#define GSWKVP	gswkvp
#define GCRSG   gcrsg
#define GCLSG   gclsg
#define GDSG    gdsg
#define GASGWK  gasgwk
#define GCSGWK  gcsgwk
#define GSSGT   gssgt
#define GINLC	ginlc
#define GRQLC	grqlc
#define GRQSK	grqsk
#define GRQCH	grqch
#define GRQST	grqst
#define GRDITM  grditm
#define GGTITM  ggtitm
#define GIITM   giitm
#define GEVTM   gevtm
#define GQOPS	gqops
#define GQLVKS	gqlvks
#define GQEWK	gqewk
#define GQMNTN	gqmntn
#define GQOPWK	gqopwk
#define GQACWK	gqacwk
#define GQSGWK	gqsgwk
#define GQLN	gqln
#define GQLWSC	gqlwsc
#define GQPLCI	gqplci
#define GQMK	gqmk
#define GQMKSC	gqmksc
#define GQPMCI	gqpmci
#define GQTXFP	gqtxfp
#define GQCHXP	gqchxp
#define GQCHSP	gqchsp
#define GQTXCI	gqtxci
#define GQCHH	gqchh
#define GQCHUP	gqchup
#define GQTXP	gqtxp
#define GQTXAL	gqtxal
#define GQFAIS	gqfais
#define GQFASI	gqfasi
#define GQFACI	gqfaci
#define GQOPSG	gqopsg
#define GQCNTN	gqcntn
#define GQNT	gqnt
#define GQCLIP	gqclip
#define GQWKC	gqwkc
#define GQWKCA	gqwkca
#define GQTXX	gqtxx
#define GQTXXS	gqtxxs
#define GQDSP	gqdsp
#define GECLKS	geclks

#else

#define GOPKS	gopks_
#define GCLKS	gclks_
#define GOPWK	gopwk_
#define GCLWK	gclwk_
#define GACWK	gacwk_
#define GDAWK	gdawk_
#define GCLRWK	gclrwk_
#define GRSGWK  grsgwk_
#define GUWK	guwk_
#define GSDS	gsds_
#define GESC	gesc_
#define GMSG	gmsg_
#define GPL	gpl_
#define GPM	gpm_
#define GTX	gtx_
#define GTXS	gtxs_
#define GFA	gfa_
#define GCA	gca_
#define GSPLI	gspli_
#define GSLN	gsln_
#define GSLWSC	gslwsc_
#define GSPLCI	gsplci_
#define GSPMI	gspmi_
#define GSMK	gsmk_
#define GSMKSC	gsmksc_
#define GSPMCI	gspmci_
#define GSTXI	gstxi_
#define GSTXFP	gstxfp_
#define GSCHXP	gschxp_
#define GSCHSP	gschsp_
#define GSTXCI	gstxci_
#define GSCHH	gschh_
#define GSCHUP	gschup_
#define GSTXP	gstxp_
#define GSTXAL	gstxal_
#define GSFAI	gsfai_
#define GSFAIS	gsfais_
#define GSFASI	gsfasi_
#define GSFACI	gsfaci_
#define GSASF	gsasf_
#define GSCR	gscr_
#define GSWN	gswn_
#define GSVP	gsvp_
#define GSELNT	gselnt_
#define GSCLIP	gsclip_
#define GSWKWN	gswkwn_
#define GSWKVP	gswkvp_
#define GCRSG   gcrsg_
#define GCLSG   gclsg_
#define GDSG    gdsg_
#define GASGWK  gasgwk_
#define GCSGWK  gcsgwk_
#define GSSGT   gssgt_
#define GINLC	ginlc_
#define GRQLC	grqlc_
#define GRQSK	grqsk_
#define GRQCH	grqch_
#define GRQST	grqst_
#define GRDITM  grditm_
#define GGTITM  ggtitm_
#define GIITM   giitm_
#define GEVTM   gevtm_
#define GQOPS	gqops_
#define GQLVKS	gqlvks_
#define GQEWK	gqewk_
#define GQMNTN	gqmntn_
#define GQOPWK	gqopwk_
#define GQACWK	gqacwk_
#define GQSGWK	gqsgwk_
#define GQLN	gqln_
#define GQLWSC	gqlwsc_
#define GQPLCI	gqplci_
#define GQMK	gqmk_
#define GQMKSC	gqmksc_
#define GQPMCI	gqpmci_
#define GQTXFP	gqtxfp_
#define GQCHXP	gqchxp_
#define GQCHSP	gqchsp_
#define GQTXCI	gqtxci_
#define GQCHH	gqchh_
#define GQCHUP	gqchup_
#define GQTXP	gqtxp_
#define GQTXAL	gqtxal_
#define GQFAIS	gqfais_
#define GQFASI	gqfasi_
#define GQFACI	gqfaci_
#define GQOPSG	gqopsg_
#define GQCNTN	gqcntn_
#define GQNT	gqnt_
#define GQCLIP	gqclip_
#define GQWKC	gqwkc_
#define GQWKCA	gqwkca_
#define GQTXX	gqtxx_
#define GQTXXS	gqtxxs_
#define GQDSP	gqdsp_
#define GECLKS	geclks_

#endif
#endif /* cray, _WIN32 */

#if defined (_WIN32) && !defined (__GNUC__)
#define STDCALL __stdcall
#else
#define STDCALL
#endif

extern void STDCALL GOPKS(int *, int *);
extern void STDCALL GCLKS(void);
extern void STDCALL GOPWK(int *, int *, int *);
extern void STDCALL GCLWK(int *);
extern void STDCALL GACWK(int *);
extern void STDCALL GDAWK(int *);
extern void STDCALL GCLRWK(int *, int *);
extern void STDCALL GRSGWK(int *);
extern void STDCALL GUWK(int *, int *);
extern void STDCALL GSDS(int *, int *, int *);
extern void STDCALL GESC(int *, int *, int *, int *, int *, int *);
extern void STDCALL GMSG(int *, char *, unsigned short);
extern void STDCALL GPL(int *, float *, float *);
extern void STDCALL GPM(int *, float *, float *);
extern void STDCALL GTX(float *, float *, char *, unsigned short);
extern void STDCALL GTXS(float *, float *, int *, char *, unsigned short);
extern void STDCALL GFA(int *, float *, float *);
extern void STDCALL GCA(
  float *, float *, float *, float *,
  int *, int *, int *, int *, int *, int *, int *);
extern void STDCALL GSPLI(int *);
extern void STDCALL GSLN(int *);
extern void STDCALL GSLWSC(float *);
extern void STDCALL GSPLCI(int *);
extern void STDCALL GSPMI(int *);
extern void STDCALL GSMK(int *);
extern void STDCALL GSMKSC(float *);
extern void STDCALL GSPMCI(int *);
extern void STDCALL GSTXI(int *);
extern void STDCALL GSTXFP(int *, int *);
extern void STDCALL GSCHXP(float *);
extern void STDCALL GSCHSP(float *);
extern void STDCALL GSTXCI(int *);
extern void STDCALL GSCHH(float *);
extern void STDCALL GSCHUP(float *, float *);
extern void STDCALL GSTXP(int *);
extern void STDCALL GSTXAL(int *, int *);
extern void STDCALL GSFAI(int *);
extern void STDCALL GSFAIS(int *);
extern void STDCALL GSFASI(int *);
extern void STDCALL GSFACI(int *);
extern void STDCALL GSASF(int *);
extern void STDCALL GSCR(int *, int *, float *, float *, float *);
extern void STDCALL GSWN(int *, float *, float *, float *, float *);
extern void STDCALL GSVP(int *, float *, float *, float *, float *);
extern void STDCALL GSELNT(int *);
extern void STDCALL GSCLIP(int *);
extern void STDCALL GSWKWN(int *, float *, float *, float *, float *);
extern void STDCALL GSWKVP(int *, float *, float *, float *, float *);
extern void STDCALL GCRSG(int *);
extern void STDCALL GCLSG(void);
extern void STDCALL GDSG(int *);
extern void STDCALL GASGWK(int *, int *);
extern void STDCALL GCSGWK(int *, int *);
extern void STDCALL GSSGT(int *, float [3][2]);
extern void STDCALL GINLC(
  int *, int *, int *, float *, float *, int *,
  float *, float *, float *, float *, int *, char *, unsigned short);
extern void STDCALL GRQLC(int *, int *, int *, int *, float *, float *);
extern void STDCALL GRQSK(
  int *, int *, int *, int *, int *, int *, float *, float *);
extern void STDCALL GRQCH(int *, int *, int *, int *);
extern void STDCALL GRQST(int *, int *, int *, int *, char *, unsigned short);
extern void STDCALL GRDITM(int *, int *, int *, char *, unsigned short);
extern void STDCALL GGTITM(int *, int *, int *);
extern void STDCALL GIITM(int *, int *, int *, char *, unsigned short);
extern void STDCALL GEVTM(
  float *, float *, float *, float *, float *,
  float *, float *, int *, float [3][2]);
extern void STDCALL GQOPS(int *);
extern void STDCALL GQLVKS(int *, int *);
extern void STDCALL GQEWK(int *n, int *, int *, int *);
extern void STDCALL GQMNTN(int *, int *);
extern void STDCALL GQOPWK(int *, int *, int *, int *);
extern void STDCALL GQACWK(int *, int *, int *, int *);
extern void STDCALL GQSGWK(int *, int *, int *, int *, int *);
extern void STDCALL GQLN(int *, int *);
extern void STDCALL GQLWSC(int *, float *);
extern void STDCALL GQPLCI(int *, int *);
extern void STDCALL GQMK(int *, int *);
extern void STDCALL GQMKSC(int *, float *);
extern void STDCALL GQPMCI(int *, int *);
extern void STDCALL GQTXFP(int *, int *, int *);
extern void STDCALL GQCHXP(int *, float *);
extern void STDCALL GQCHSP(int *, float *);
extern void STDCALL GQTXCI(int *, int *);
extern void STDCALL GQCHH(int *, float *);
extern void STDCALL GQCHUP(int *, float *, float *);
extern void STDCALL GQTXP(int *, int *);
extern void STDCALL GQTXAL(int *, int *, int *);
extern void STDCALL GQFAIS(int *, int *);
extern void STDCALL GQFASI(int *, int *);
extern void STDCALL GQFACI(int *, int *);
extern void STDCALL GQOPSG(int *, int *);
extern void STDCALL GQCNTN(int *, int *);
extern void STDCALL GQNT(int *, int *, float *, float *);
extern void STDCALL GQCLIP(int *, int *, float *);
extern void STDCALL GQWKC(int *, int *, int *, int *);
extern void STDCALL GQWKCA(int *, int *, int *);
#if defined (_WIN32) && !defined (__GNUC__)
extern void STDCALL GQTXX(
  int *, float *, float *, char *, unsigned short,
  int *, float *, float *, float *, float *);
extern void STDCALL GQTXXS(
  int *, float *, float *, int *, char *, unsigned short,
  int *, float *, float *, float *, float *);
#else
extern void STDCALL GQTXX(
  int *, float *, float *, char *,
  int *, float *, float *, float *, float *, unsigned short);
extern void STDCALL GQTXXS(
  int *, float *, float *, int *, char *,
  int *, float *, float *, float *, float *, unsigned short);
#endif
extern void STDCALL GQDSP(
  int *, int *, int *, float *, float *ry, int *lx, int *ly);
extern void STDCALL GECLKS(void);
