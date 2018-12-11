#ifndef __GSM_H__
#define __GSM_H__
extern "C"{
    typedef short			word;		/* 16 bit signed int	*/
    typedef long			longword;	/* 32 bit signed int	*/

    typedef unsigned short		uword;		/* unsigned word	*/
    typedef unsigned long		ulongword;	/* unsigned longword	*/
    struct gsm_state {

    	word		dp0[ 280 ];
    	word		e[ 50 ];	/* code.c 			*/

    	word		z1;		/* preprocessing.c, Offset_com. */
    	longword	L_z2;		/*                  Offset_com. */
    	int		mp;		/*                  Preemphasis	*/

    	word		u[8];		/* short_term_aly_filter.c	*/
    	word		LARpp[2][8]; 	/*                              */
    	word		j;		/*                              */

    	word            ltp_cut;        /* long_term.c, LTP crosscorr.  */
    	word		nrp; /* 40 */	/* long_term.c, synthesis	*/
    	word		v[9];		/* short_term.c, synthesis	*/
    	word		msr;		/* decoder.c,	Postprocessing	*/

    	char		verbose;	/* only used if !NDEBUG		*/
    	char		fast;		/* only used if FAST		*/

    	char		wav_fmt;	/* only used if WAV49 defined	*/
    	unsigned char	frame_index;	/*            odd/even chaining	*/
    	unsigned char	frame_chain;	/*   half-byte to carry forward	*/
    };

    typedef struct gsm_state * 	gsm;

    gsm  gsm_create 	(void);
    void gsm_destroy (gsm);
    void gsm_encode  (gsm, short *, unsigned char *);
    int  gsm_decode  (gsm, unsigned char   *, short *);
}

class GsmEnCoder{
public:
    GsmEnCoder(){
        coder = gsm_create();
    }
    ~GsmEnCoder(){
        gsm_destroy(coder);
    }
    void EnCode(short* input, unsigned char* output){
        gsm_encode(coder , input , output);
    }
private:
    gsm coder;
};

class GsmDeCoder{
public:
    GsmDeCoder(){
        coder = gsm_create();
    }
    ~GsmDeCoder(){
        gsm_destroy(coder);
    }
    void DeCode(unsigned char* input , short* output){
        gsm_decode(coder , input , output);
    }
private:
    gsm coder;
};


#endif
